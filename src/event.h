#include <pebble.h>

#pragma once

#define TITLE_SIZE 15
#define TIME_SIZE 9

typedef enum {
	MINUTE,
	SECOND
} Unit;

typedef struct {
	char* title;
	time_t seconds;
} event_Event;

typedef struct {
	uint8_t size;
	uint8_t index;
	event_Event** events;
} Events;

event_Event* event_createEvent(const char* title, time_t seconds);

int32_t event_getTimeLeft(event_Event* event, struct tm* tick_time);

void event_destroyEvent(event_Event* event);

Events* events_create(uint8_t size);

void events_destroy(Events* events);

char** events_getCurrent(Events* events, struct tm* tick_time, char** out, Unit units);

