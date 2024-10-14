#ifndef __EVENT_H__
#define __EVENT_H__

void wait_for_event_timeout(void *data, int (*expr)(void *data),
			    unsigned long ms);

#endif
