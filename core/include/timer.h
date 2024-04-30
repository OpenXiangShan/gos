#ifndef _TIMER_H
#define _TIMER_H

#include <device.h>
#include "list.h"

struct timer_event_info {
	struct list_head list;
	void (*handler)(void *data);
	void (*del_cb)(void *data);
	void *data;
	int done;
	unsigned long expiry_time;
	unsigned long period;
	int restart;
};

int init_timer(struct device_init_entry *hw);
unsigned long get_system_time(void);
unsigned long get_system_time_ms(void);
int set_timer(unsigned long ms, void (*timer_handler)(void *data), void *data);

#endif