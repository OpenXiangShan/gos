#ifndef _EVENT_H
#define _EVENT_H

void wait_for_event(void *data, int (*expr)(void *));
void wait_for_ms(unsigned long ms);
char wait_for_input_timeout(int fd, unsigned long ms);

#endif
