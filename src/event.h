#include <pebble.h>

typedef enum {
	MINUTE,
	SECOND,
	NONE
} unit;

typedef struct {
	char* title;
	uint16_t year;
	uint32_t seconds;
	char* s_time;
} event_Event;

event_Event* event_createEvent(const char* title, struct tm *tick_time);

char* event_getTimeLeft(event_Event* event, struct tm* tick_time, unit units);

void event_destroyEvent(event_Event* event);
