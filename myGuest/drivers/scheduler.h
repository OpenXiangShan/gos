#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

void scheduler_schedule(void);
void scheduler_sleep_to_timeout(int ms);
int scheduler_init(unsigned long base, unsigned int len, void *data);

#endif
