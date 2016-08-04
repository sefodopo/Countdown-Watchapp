#include <pebble.h>

#pragma once

#define TITLE_SIZE 15
#define TIME_SIZE 9
#define NULL_NUMBER 99999
#define MAX_TEXT_LENGTH 20

typedef enum {
	MINUTE,
	SECOND
} Unit;

enum FontSize {
	SMALL,
	MEDIUM,
	LARGE
};

struct main_data {
	uint layers_enabled;
	uint date_layer;
	uint first_event_title;
	uint first_event_time;
	uint second_event_title;
	uint second_event_time;
	uint third_event_title;
	uint third_event_time;
	uint events_enabled;
	enum FontSize first_layer_font_size;
	enum FontSize second_layer_font_size;
	enum FontSize third_layer_font_size;
	enum FontSize fourth_layer_font_size;
	enum FontSize fifth_layer_font_size;
	enum FontSize sixth_layer_font_size;
	enum FontSize seventh_layer_font_size;
};

typedef struct {
	char* title;
	time_t seconds;
} event_Event;

typedef struct {
	uint8_t size;
	uint8_t index;
	event_Event** events;
} Events;

event_Event* event_create(const char* title, time_t seconds);

int32_t event_getTimeLeft(event_Event* event, struct tm* tick_time);

void event_destroy(event_Event* event);

Events* events_create(uint8_t size);

void events_destroy(Events* events);

char** events_getCurrent(Events** events, struct main_data* data, struct tm* tick_time, char** out, Unit units);

