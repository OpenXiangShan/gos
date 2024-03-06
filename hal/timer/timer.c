#include "timer.h"
#include <device.h>
#include <irq.h>
#include <asm/type.h>

struct timer_event_info timer;

extern unsigned long volatile jiffies;

unsigned long get_system_time()
{
	return jiffies;
}

unsigned long get_system_time_ms()
{
	return get_system_time();
}

int set_timer(unsigned long ms, void (*timer_handler)(void *data), void *data)
{
	if (timer.registered == 1)
		return -1;

	timer.registered = 1;
	timer.handler = timer_handler;
	timer.data = data;
	timer.ms = ms;
	timer.done = 0;

	return 0;
}

int del_timer()
{
	timer.registered = -1;
	timer.handler = NULL;
	timer.data = NULL;
	timer.ms = 0;

	return 0;
}

int mod_timer(unsigned long ms)
{
	if (timer.registered == -1)
		return -1;

	timer.ms = ms / 10;
	timer.done = 0;

	return 0;
}

unsigned long get_timer_event_ms()
{
	if (timer.registered == -1)
		return 0;

	return timer.ms;
}

void do_timer_handler()
{
	if (timer.registered == -1)
		return;

	if (timer.done)
		return;

	if (!timer.handler)
		return;

	timer.done = 1;
	timer.handler(timer.data);
}

unsigned long get_system_cycles(void)
{
	return get_cycles();
}

int init_timer(struct device_init_entry *hw)
{
	timer.registered = -1;

	return timer_setup(hw);
}
