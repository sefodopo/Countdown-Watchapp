#include <pebble.h>
#include "event.h"

#define SMALL_TEXT_SIZE 20
#define MEDIUM_TEXT_SIZE 24
#define LARGE_TEXT_SIZE 38
#define MAX_PADDING_SIZE 10

#define PERSIST_MAIN_DATA 0
#define PERSIST_EVENT_DATA 1

#define INBOX_SIZE 64
#define OUTBOX_SIZE 256

static Window* s_main_window;
static TextLayer** text_layers;
static Unit current_unit;
static GFont f_small;
static GFont f_medium;
static GFont f_large;
static Events** events;
static char** text;

static struct main_data m_data;

void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
	events_getCurrent(events, &m_data, tick_time, text, current_unit);
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		text_layer_set_text(text_layers[i], text[i]);
	}
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
	Tuple *dataTuple = dict_find(iter, MESSAGE_KEY_Data);
	if (dataTuple) {
		memcpy(&m_data, dataTuple->value->data, sizeof(struct main_data));
		persist_write_data(PERSIST_MAIN_DATA, &m_data, sizeof(struct main_data));
	}
	dataTuple = dict_find(iter, MESSAGE_KEY_Events);
	if (dataTuple) {
		memcpy(&events, dataTuple->value->data, 3 * sizeof(Events*));
		persist_write_data(PERSIST_EVENT_DATA, &events, sizeof(Events**));
	}
	time_t currentTime;
	struct tm* ticker;
	time(&currentTime);
	ticker = localtime(&currentTime);
	tick_handler(ticker, MINUTE);
}

GFont get_font_from_size(enum FontSize size) {
	switch (size) {
		case SMALL:
			return f_small;
		case MEDIUM:
			return f_medium;
		case LARGE:
			return f_large;
		default:
			return f_small;
	}
}

static enum FontSize get_font_size(int i) {
	switch(i) {
		case 6:
			return m_data.seventh_layer_font_size;
		case 5:
			return m_data.sixth_layer_font_size;
		case 4:
			return m_data.fifth_layer_font_size;
		case 3:
			return m_data.fourth_layer_font_size;
		case 2:
			return m_data.third_layer_font_size;
		case 1:
			return m_data.second_layer_font_size;
		case 0:
			return m_data.first_layer_font_size;
		default:
			return SMALL;
	}
}

static int get_height(int i) {
	switch (get_font_size(i)) {
		case SMALL:
			return SMALL_TEXT_SIZE;
		case MEDIUM:
			return MEDIUM_TEXT_SIZE;
		case LARGE:
			return LARGE_TEXT_SIZE;
		default:
			return SMALL_TEXT_SIZE;
	}
}

static void main_window_load(Window *window) {
	window_set_background_color(window, GColorBlack);
	current_unit = MINUTE;
	Layer* window_layer = window_get_root_layer(window);
	int window_height = layer_get_bounds(window_layer).size.h;
	int window_width = layer_get_bounds(window_layer).size.w;
	int layers_height = 0;
	int padding;
	if (persist_exists(PERSIST_MAIN_DATA)) {
		persist_read_data(PERSIST_MAIN_DATA, &m_data, sizeof(struct main_data));
	} else {
		m_data.layers_enabled = 2;
		m_data.date_layer = 0;
		m_data.events_enabled = 1;
		m_data.first_event_title = NULL_NUMBER;
		m_data.first_event_time = 1;
		m_data.first_layer_font_size = MEDIUM;
		m_data.second_layer_font_size = LARGE;
	}
	// alocate memory for all pointers
	text = malloc (m_data.layers_enabled * sizeof(char*));
	text_layers = malloc (m_data.layers_enabled * sizeof(TextLayer*));
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		layers_height += get_height(i);
	}
	padding = (window_height - layers_height) / (m_data.layers_enabled + 1);
	int ii = padding;
	if (padding > MAX_PADDING_SIZE) {
		layers_height += MAX_PADDING_SIZE * (m_data.layers_enabled - 1);
		ii = (window_height - layers_height) / 2;
		padding = MAX_PADDING_SIZE;
	}
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		text[i] = malloc(MAX_TEXT_LENGTH * sizeof(char));
		text[i][0] = '\0';
		text_layers[i] = text_layer_create(GRect(0, ii, window_width, get_height(i)));
		text_layer_set_background_color(text_layers[i], GColorClear);
		text_layer_set_text_color(text_layers[i], GColorWhite);
		text_layer_set_text(text_layers[i], text[i]);
		text_layer_set_text_alignment(text_layers[i], GTextAlignmentCenter);
		text_layer_set_font(text_layers[i], get_font_from_size(get_font_size(i)));
		layer_add_child(window_layer, text_layer_get_layer(text_layers[i]));
		ii += get_height(i);
		ii += padding;
	}
	events = malloc(3 * sizeof(Events*));
	if (persist_exists(PERSIST_EVENT_DATA)) {
		persist_read_data(PERSIST_EVENT_DATA, &events, sizeof(Events**));
	} else {
		for (uint i = 0; i < m_data.events_enabled; i++) {
			events[i] = events_create(0);
		}
	}
	// Subscribe to alerts about the minute changing
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	app_message_open(INBOX_SIZE, OUTBOX_SIZE);
	app_message_register_inbox_received(inbox_received_callback);
}

static void main_window_unload(Window *window) {
	tick_timer_service_unsubscribe();
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		text_layer_destroy(text_layers[i]);
		free(text[i]);
	}
	free(text_layers);
	free(text);
	for (uint i = 0; i < m_data.events_enabled; i++) {
		events_destroy(events[i]);
	}
	free(events);
}

static void init() {
	f_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SANS_15));
	f_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SANS_20));
	f_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SANS_30));
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	window_stack_push(s_main_window, true);
}

static void deinit() {
	window_destroy(s_main_window);
	fonts_unload_custom_font(f_small);
	fonts_unload_custom_font(f_medium);
	fonts_unload_custom_font(f_large);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

