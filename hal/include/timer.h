#ifndef _TIMER_H
#define _TIMER_H

#include <device.h>

struct timer_event_info {
	int registered;
	unsigned long ms;
	void (*handler)(void *data);
	void *data;
	int done;
};

int init_timer(struct device_init_entry *hw);
void do_timer_handler();
unsigned long get_system_time(void);
int del_timer();
int set_timer(unsigned long ms, void (*timer_handler)(void *data), void *data);
unsigned long get_timer_event_ms(void);
int mod_timer(unsigned long ms);

#endif
