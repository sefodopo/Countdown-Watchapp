#include <pebble.h>
#include "event.h"

event_Event* event_createEvent(const char* title, struct tm* tick_time) {
	event_Event* event = (event_Event*)malloc(sizeof(event_Event));
	struct tm yeartime;
	yeartime.tm_year = tick_time->tm_year;
	event->title = (char*)malloc(strlen(title) * sizeof(char));
	strcpy(event->title, title);
	event->year = tick_time->tm_year;
	event->seconds = (uint32_t)(mktime(tick_time) - mktime(&yeartime));
	event->s_time = (char*)malloc(9 * sizeof(char));
	return event;
}

char* event_getTimeLeft(event_Event* event, struct tm* tick_time, unit units) {
	uint32_t seconds;
	uint16_t hour;
	uint8_t minute;
	if (tick_time->tm_year == event->year) {
		seconds = event->seconds - mktime(tick_time);
	} else {
		seconds = 0;
	}
	hour = seconds / SECONDS_PER_HOUR;
	seconds -= hour * SECONDS_PER_HOUR;
	minute = seconds / SECONDS_PER_MINUTE;
	seconds -= minute * SECONDS_PER_MINUTE;
	switch (units) {
		case MINUTE:
			snprintf(event->s_time, 9 * sizeof(char), "%u:%u", (unsigned int)hour, (unsigned int)minute);
			break;
		case SECOND:
			snprintf(event->s_time, 9 * sizeof(char), "%u:%u", (unsigned int)minute, (unsigned int)seconds);
			break;
		default:
			break;
	}
	return event->s_time;
}

void event_destroyEvent(event_Event* event) {
	free(event->title);
	free(event->s_time);
	free(event);
}
