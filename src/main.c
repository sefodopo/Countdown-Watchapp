#include <pebble.h>
#include "event.h"

static Window* s_main_window;
static TextLayer* title_layer;
static TextLayer* time_layer;
static TimeUnits current_unit;
static Events* first_event;
static char first_event_string[2][9];

void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
	events_getCurrent(first_event, tick_time, (char**)first_event_string, current_unit);
}

static void main_window_load(Window *window) {
	current_unit = MINUTE_UNIT;
	first_event = events_create(1);
	time_t tempt = time(NULL);
	struct tm temp = *localtime(&tempt);
	temp.tm_min = 20;
	first_event->events[0] = event_createEvent("Test", 2016, mktime(&temp));
	first_event_string[0][0] = '\0';
	first_event_string[1][0] = '\0';
	
	Layer* window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	window_set_background_color(window, GColorBlack);
	
	title_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_text_color(title_layer, GColorWhite);
	text_layer_set_text(title_layer, first_event_string[0]);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	
	time_layer = text_layer_create (GRect(0, 30, bounds.size.w, 30));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_text(time_layer, first_event_string[1]);
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
	events_destroy(first_event);
	text_layer_destroy(title_layer);
	text_layer_destroy(time_layer);
}

static void init() {
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	window_stack_push(s_main_window, true);
}

static void deinit() {
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

