#include <device.h>
#include <irq.h>
#include <asm/type.h>
#include "timer.h"
#include "string.h"

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
	if(timer.registered == 1)
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

static int timer_setup(struct device_init_entry *hw)
{
	extern unsigned long TIMER_INIT_TABLE, TIMER_INIT_TABLE_END;
	int driver_nr =
	    (struct timer_init_entry *)&TIMER_INIT_TABLE_END -
	    (struct timer_init_entry *)&TIMER_INIT_TABLE;
	int driver_nr_tmp = 0;
	struct timer_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	struct timer_init_entry *driver_tmp =
	    (struct timer_init_entry *)&TIMER_INIT_TABLE;
	struct irq_domain *d;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				d = find_irq_domain(device_entry->irq_parent);

				driver_entry->init(device_entry->start, d,
						   device_entry->data);
			}
		}
		device_entry++;
	}

	return 0;
}

int init_timer(struct device_init_entry *hw)
{
	timer.registered = -1;

	return timer_setup(hw);
}
