#include <pebble.h>
#include "event.h"

event_Event* event_create(const char* title, time_t seconds) {
	event_Event* event = (event_Event*)malloc(sizeof(event_Event));
	event->title = (char*)malloc(strlen(title) * sizeof(char));
	strcpy(event->title, title);
	event->seconds = seconds;
	return event;
}

int32_t event_getTimeLeft(event_Event* event, struct tm* tick_time) {
	return event->seconds - mktime(tick_time);
}

void event_destroy(event_Event* event) {
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
		event_destroy(events->events[i]);
	}
	free(events->events);
	free(events);
}
enum tot {
	TITLE,
	TIME
};

static uint getUint(struct main_data* data, uint event, enum tot param3) {
	switch (param3) {
		case TITLE:
			switch(event) {
				case 0:
					return (data->first_event_title);
				case 1:
					return (data->second_event_title);
				case 2:
					return (data->third_event_title);
				default:
					return (NULL_NUMBER);
			}
		case TIME:
			switch (event) {
				case 0:
					return (data->first_event_time);
				case 1:
					return (data->second_event_time);
				case 2:
					return (data->third_event_time);
				default:
					return (NULL_NUMBER);
			}
		default:
			return (NULL_NUMBER);
	}
}

char** events_getCurrent(Events** events, struct main_data* data, struct tm* tick_time, char** out, Unit units) {
	if (data->date_layer != NULL_NUMBER) {
		strftime(out[data->date_layer], MAX_TEXT_LENGTH * sizeof(char), "%a, %b %d", tick_time);
	}
	for (uint i = 0; i < data->events_enabled; i++) {
		event_Event* current = NULL;
		int32_t seconds;
		uint8_t ii;
		for (ii = events[i]->index; ii < events[i]->size; ii++) {
			seconds = event_getTimeLeft(events[i]->events[ii], tick_time);
			if (seconds >= 0) {
				current = events[i]->events[ii];
				break;
			}
		}
		events[i]->index = ii;
		if (current == NULL) {
			uint temp = getUint(data, i, TITLE);
			if (temp != NULL_NUMBER) {
				out[temp][0] = '\0';
			}
			temp = getUint(data, i, TIME);
			if (temp != NULL_NUMBER) {
				if (clock_is_24h_style()) {
					strftime(out[temp], MAX_TEXT_LENGTH * sizeof(char), "%H:%M", tick_time);
				} else {
					strftime(out[temp], MAX_TEXT_LENGTH * sizeof(char), "%I:%M %P", tick_time);
				}
			}
		} else {
			uint hours;
			uint minutes;
			hours = seconds / SECONDS_PER_HOUR;
			seconds -= hours * SECONDS_PER_HOUR;
			minutes = seconds / SECONDS_PER_MINUTE;
			seconds -= minutes * SECONDS_PER_MINUTE;
			uint temp = getUint(data, i, TITLE);
			if (temp != NULL_NUMBER) {
				strncpy(out[temp], current->title, MAX_TEXT_LENGTH * sizeof(char));
			}
			temp = getUint(data, i, TIME);
			if (temp != NULL_NUMBER) {
				switch (units) {
					case MINUTE:
						snprintf(out[temp], MAX_TEXT_LENGTH * sizeof(char), "%u:%u", hours, minutes);
						break;
					case SECOND:
						snprintf(out[temp], MAX_TEXT_LENGTH * sizeof(char), "%u:%i", minutes, (int)seconds);
						break;
					default:
						break;
				}
			}
		}
	}
	return out;
}
