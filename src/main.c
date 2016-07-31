#include <pebble.h>
#include "event.h"

static Window* s_main_window;
static TextLayer* title_layer;
static TextLayer* time_layer;
static event_Event* event;
static unit current_unit;

void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
}

static void main_window_load(Window *window) {
	current_unit = MINUTE;
	
	Layer* window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	window_set_background_color(window, GColorBlack);
	
	title_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_text_color(title_layer, GColorWhite);
	text_layer_set_text(title_layer, "");
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	
	time_layer = text_layer_create (GRect(0, 30, bounds.size.w, 30));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_text(time_layer, "00:00");
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void main_window_unload(Window *window) {
	text_layer_destroy(title_layer);
	text_layer_destroy(time_layer);
	event_destroyEvent(event);
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

