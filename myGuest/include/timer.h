#ifndef __TIMER_H__
#define __TIMER_H__

typedef int (*do_timer_t)(void *data);
typedef unsigned long (*get_system_tick_t)(void);

int irq_do_timer_handler(void);
void register_timer_irq_handler(do_timer_t do_timer, void *data);
unsigned long get_system_tick(void);
unsigned long get_system_time(void);
void register_system_tick_handler(get_system_tick_t handler);
int set_timer(do_timer_t handler, unsigned long ms, void *data);

#endif
