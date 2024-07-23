#ifndef __SLEEP_H__
#define __SLEEP_H__

int scheduler_early_init(struct device_init_entry *entry);
int sleep(int ms);
int schedule(void);

#endif
