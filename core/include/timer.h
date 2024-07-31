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
	int freeze;
};

int init_timer(struct device_init_entry *hw);
unsigned long get_system_time(void);
unsigned long get_system_time_ms(void);
struct timer_event_info *set_timer(unsigned long ms, void (*timer_handler)(void *data), void *data);
struct timer_event_info *set_timer_restart(unsigned long ms, void (*timer_handler)(void *data), void *data);
struct timer_event_info *set_timer_cpu(unsigned long ms,
				void (*timer_handler)(void *data), void *data, int cpu);
struct timer_event_info *set_timer_restart_cpu(unsigned long ms,
				void (*timer_handler)(void *data), void *data, int cpu);
int del_timer_cpu(struct timer_event_info *timer, int cpu);
int del_timer(struct timer_event_info *timer);
void set_timer_freeze(struct timer_event_info *timer, int freeze);

#endif
