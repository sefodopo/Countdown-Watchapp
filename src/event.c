#include <pebble.h>
#include "event.h"

event_Event* event_createEvent(const char* title, uint16_t year, uint32_t seconds) {
	event_Event* event = (event_Event*)malloc(sizeof(event_Event));
	event->title = (char*)malloc(strlen(title) * sizeof(char));
	strcpy(event->title, title);
	event->year = year;
	event->seconds = seconds;
	return event;
}

int32_t event_getTimeLeft(event_Event* event, struct tm* tick_time) {
	int32_t seconds;
	if (tick_time->tm_year == event->year) {
		seconds = event->seconds - mktime(tick_time);
	} else {
		seconds = 0;
	}
	return seconds;
}

void event_destroyEvent(event_Event* event) {
	free(event->title);
	free(event);
}

Events* events_create(uint8_t size) {
	Events* events = (Events*)malloc(sizeof(Events));
	events->size = size;
	events->index = 0;
	events->events = (event_Event**)malloc(size * sizeof(event_Event*));
	return events;
}

void events_destroy(Events* events) {
	for (int i = 0; i < events->size; i++) {
		event_destroyEvent(events->events[i]);
	}
	free(events->events);
	free(events);
}

char** events_getCurrent(Events* events, struct tm* tick_time, char** out, TimeUnits units) {
	event_Event* current = NULL;
	int32_t seconds;
	uint8_t i;
	for (i = events->index; i < events->size; i++) {
		seconds = event_getTimeLeft(events->events[i], tick_time);
		if (seconds >= 0) {
			current = events->events[i];
			break;
		}
	}
	events->index = i;
	if (current == NULL) {
		snprintf(out[0], TITLE_SIZE * sizeof(char), " ");
		strftime(out[1], TIME_SIZE * sizeof(char), "%H:%M", tick_time);
		return out;
	}
	uint16_t hours;
	uint8_t minutes;
	hours = seconds / SECONDS_PER_HOUR;
	seconds -= hours * SECONDS_PER_HOUR;
	minutes = seconds / SECONDS_PER_MINUTE;
	seconds -= minutes * SECONDS_PER_MINUTE;
	out[0] = current->title;
	switch (units) {
		case MINUTE_UNIT:
			snprintf(out[1], TIME_SIZE * sizeof(char), "%u:%u", (unsigned int)hours, (unsigned int)minutes);
			break;
		case SECOND_UNIT:
			snprintf(out[1], TIME_SIZE * sizeof(char), "%u:%u", (unsigned int)minutes, (unsigned int)seconds);
			break;
		default:
			break;
	}
	return out;
}
