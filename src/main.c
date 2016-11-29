#include <pebble.h>
#include "main.h"
#include "event.h"

#define SMALL_TEXT_SIZE 20
#define MEDIUM_TEXT_SIZE 24
#define LARGE_TEXT_SIZE 38
#define MAX_PADDING_SIZE 10

#define PERSIST_MAIN_DATA 0
#define PERSIST_FIRST_EVENT_COUNT 1
#define PERSIST_SECOND_EVENT_COUNT 2
#define PERSIST_THIRD_EVENT_COUNT 3
#define PERSIST_FIRST_EVENT_DATA 100
#define PERSIST_SECOND_EVENT_DATA 140
#define PERSIST_THIRD_EVENT_DATA 160

#define INBOX_SIZE 1024
#define OUTBOX_SIZE 10

#define AppKeyLayersEnabled 0
#define AppKeyDateLayer 1
#define AppKeyFirstEventTitle 2
#define AppKeyFirstEventTime 3
#define AppKeySecondEventTitle 4
#define AppKeySecondEventTime 5
#define AppKeyThirdEventTitle 6
#define AppKeyThirdEventTime 7
#define AppKeyEventsEnabled 8
#define AppKeyStartFontSize 9
#define AppKeyFirstEventCount 16
#define AppKeySecondEventCount 17
#define AppKeyThirdEventCount 18
#define AppKeyStartFirstEvents 100
#define AppKeyStartSecondEvents 140
#define AppKeyStartThirdEvents 160


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
	if (events_getCurrent(events, &m_data, tick_time, text, current_unit)) {
		switch (current_unit) {
			case MINUTE:
				current_unit = SECOND;
				tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
				break;
			case SECOND:
				current_unit = MINUTE;
				tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
				break;
		}
	}
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		text_layer_set_text(text_layers[i], text[i]);
	}
}

static bool check_uint_changed(DictionaryIterator *iter, uint *update, int appKey) {
	Tuple *tuple = dict_find(iter, appKey);
	if (tuple) {
		*update = tuple->value->uint8;
		return true;
	}
	return false;
}

static void check_changed_font_size(DictionaryIterator *iter, int offset, enum FontSize *size){
	Tuple *tuple = dict_find(iter, AppKeyStartFontSize + offset);
	if (tuple) {
		switch (tuple->value->uint8) {
			case 0:
				*size = SMALL;
				break;
			case 1:
				*size = MEDIUM;
				break;
			case 2:
				*size = LARGE;
				break;
			default:
				*size = SMALL;
		}
	}
}

static void check_all_changed_font_size(DictionaryIterator *iter, int temp) {
	// check for changed FontSizes
	switch (temp) {
		case 7:
			check_changed_font_size(iter, 6, &m_data.seventh_layer_font_size);
		case 6:
			check_changed_font_size(iter, 5, &m_data.sixth_layer_font_size);
		case 5:
			check_changed_font_size(iter, 4, &m_data.fifth_layer_font_size);
		case 4:
			check_changed_font_size(iter, 3, &m_data.fourth_layer_font_size);
		case 3:
			check_changed_font_size(iter, 2, &m_data.third_layer_font_size);
		case 2:
			check_changed_font_size(iter, 1, &m_data.second_layer_font_size);
		case 1:
			check_changed_font_size(iter, 0, &m_data.first_layer_font_size);
	}
}

static void load_events(DictionaryIterator *iter, int int1, uint count, int appKey) {
	events_destroy(events[int1]);
	events[int1] = events_create(count);
	int temp = 0;
	for (uint i = 0; i < count; i++) {
		Tuple *tuple = dict_find(iter, appKey + temp);
		if (tuple) {
			char *string = tuple->value->cstring;
			strncpy(events[int1]->events[i]->title, string, MAX_TEXT_LENGTH * sizeof(char));
		}
		temp++;
		tuple = dict_find(iter, appKey + temp);
		if (tuple) {
			events[int1]->events[i]->seconds = tuple->value->uint32;
		}
		temp++;
	}
}

static void write_persist(uint key, uint int1, uint countKey) {
	uint temp = 0;
	uint count = 0;
	for (uint i = events[int1]->index; i < events[int1]->size; i++) {
		persist_write_string(key + temp, events[int1]->events[i]->title);
		temp++;
		persist_write_int(key + temp, events[int1]->events[i]->seconds);
		temp++;
		count++;
	}
	persist_write_int(countKey, count);
}

static void read_persist(uint key, uint int1) {
	uint temp = 0;
	for (uint i = 0; i < events[int1]->size; i++) {
        persist_read_string(key + temp, events[int1]->events[i]->title, MAX_TEXT_LENGTH * sizeof(char));
		temp++;
		events[int1]->events[i]->seconds = persist_read_int(key + temp);
		temp++;
	}
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
	// check if layer count changed
	Tuple *tuple = dict_find(iter, AppKeyLayersEnabled);
	if (tuple) {
		uint temp = tuple->value->uint8;
		if (temp > m_data.layers_enabled) {
			check_all_changed_font_size(iter, temp);
			destroy_text_layers();
			text = realloc(text, temp * sizeof(char*));
			for (uint i = m_data.layers_enabled; i < temp; i++) {
				text[i] = malloc(MAX_TEXT_LENGTH * sizeof(char));
				text[i][0] = '\0';
			}
			text_layers = realloc(text_layers, temp * sizeof(TextLayer*));
			m_data.layers_enabled = temp;
			create_text_layers();
		}
		else if (temp < m_data.layers_enabled) {
			check_all_changed_font_size(iter, temp);
			destroy_text_layers();
			for (uint i = temp; i < m_data.layers_enabled; i++) {
				free(text[i]);
			}
			text = realloc(text, temp * sizeof(char*));
			text_layers = realloc(text_layers, temp * sizeof(TextLayer*));
			m_data.layers_enabled = temp;
			create_text_layers();
		}
		else {
			check_all_changed_font_size(iter, temp);
			destroy_text_layers();
			create_text_layers();
		}
	}
	check_uint_changed(iter, &m_data.date_layer, AppKeyDateLayer);
	uint events_before = m_data.events_enabled;
	check_uint_changed(iter, &m_data.events_enabled, AppKeyEventsEnabled);
	if (events_before < m_data.events_enabled) {
		realloc(events, m_data.events_enabled * sizeof(Events*));
		for (uint i = events_before; i < m_data.events_enabled; i++) {
			events[i] = events_create(0);
		}
	} else if (events_before > m_data.events_enabled) {
		for (uint i = m_data.events_enabled; i < events_before; i++) {
			events_destroy(events[i]);
		}
		realloc(events, m_data.events_enabled * sizeof(Events*));
	}
	uint count;
	switch (m_data.events_enabled) {
		case 3:
			check_uint_changed(iter, &m_data.third_event_title, AppKeyThirdEventTitle);
			check_uint_changed(iter, &m_data.third_event_time, AppKeyThirdEventTime);
			if (check_uint_changed(iter, &count, AppKeyThirdEventCount)) {
				load_events(iter, 2, count, AppKeyStartThirdEvents);
			}
		case 2:
			check_uint_changed(iter, &m_data.second_event_title, AppKeySecondEventTitle);
			check_uint_changed(iter, &m_data.second_event_time, AppKeySecondEventTime);
			if (check_uint_changed(iter, &count, AppKeySecondEventCount)) {
				load_events(iter, 1, count, AppKeyStartSecondEvents);
			}
		case 1:
			check_uint_changed(iter, &m_data.first_event_title, AppKeyFirstEventTitle);
			check_uint_changed(iter, &m_data.first_event_time, AppKeyFirstEventTime);
			if (check_uint_changed(iter, &count, AppKeyFirstEventCount)) {
				load_events(iter, 0, count, AppKeyStartFirstEvents);
			}
	}
	//Save persistently...
	persist_write_data(PERSIST_MAIN_DATA, &m_data, sizeof(struct main_data));
	switch(m_data.events_enabled) {
		case 3:
			write_persist(PERSIST_THIRD_EVENT_DATA, 2, PERSIST_THIRD_EVENT_COUNT);
		case 2:
			write_persist(PERSIST_SECOND_EVENT_DATA, 1, PERSIST_SECOND_EVENT_COUNT);
		case 1:
			write_persist(PERSIST_FIRST_EVENT_DATA, 0, PERSIST_FIRST_EVENT_COUNT);
	}
	//update display
	time_t currentTime = time(NULL);
	struct tm* ticker = localtime(&currentTime);
	tick_handler(ticker, MINUTE_UNIT);
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

static void destroy_text_layers() {
	for (uint i = 0; i < m_data.layers_enabled; i++) {
		text_layer_destroy(text_layers[i]);
	}
}

static void create_text_layers() {
	int layers_height = 0;
	int padding;
	Layer* window_layer = window_get_root_layer(s_main_window);
	int window_height = layer_get_bounds(window_layer).size.h;
	int window_width = layer_get_bounds(window_layer).size.w;
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
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void main_window_load(Window *window) {
	window_set_background_color(window, GColorBlack);
	current_unit = MINUTE;
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
		text[i] = malloc(MAX_TEXT_LENGTH * sizeof(char));
		text[i][0] = '\0';
	}
	create_text_layers();
	events = malloc(m_data.events_enabled * sizeof(Events*));
	switch(m_data.events_enabled) {
		case 3:
			if (persist_exists(PERSIST_THIRD_EVENT_COUNT)) {
				events[2] = events_create(persist_read_int(PERSIST_THIRD_EVENT_COUNT));
				read_persist(PERSIST_THIRD_EVENT_DATA, 2);
			} else {
				events[2] = events_create(0);
			}
		case 2:
			if (persist_exists(PERSIST_SECOND_EVENT_COUNT)) {
				events[1] = events_create(persist_read_int(PERSIST_SECOND_EVENT_COUNT));
				read_persist(PERSIST_SECOND_EVENT_DATA, 1);
			} else {
				events[1] = events_create(0);
			}
		case 1:
			if (persist_exists(PERSIST_FIRST_EVENT_COUNT)) {
				events[0] = events_create(persist_read_int(PERSIST_FIRST_EVENT_COUNT));
				read_persist(PERSIST_FIRST_EVENT_DATA, 0);
			} else {
				events[0] = events_create(0);
			}
	}
	// Subscribe to alerts about the minute changing
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	time_t now = time(NULL);
	struct tm* ticker = localtime(&now);
	tick_handler(ticker, MINUTE_UNIT);
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_open(INBOX_SIZE, OUTBOX_SIZE);
}

static void main_window_unload(Window *window) {
	tick_timer_service_unsubscribe();
	destroy_text_layers();
	for (uint i = 0; i < m_data.layers_enabled; i++) {
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

