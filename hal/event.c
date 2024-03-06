#include <timer.h>
#include <device.h>

void wait_for_event(void *data, int (*expr)(void *data))
{
	while(1)
		if(expr(data))
			break;
}

static void do_timer(void *data)
{
	int *start = (int *)data;

	*start = 1;
}

void wait_for_ms(unsigned long ms)
{
	int start = 0;

	set_timer(get_system_time() + ms, do_timer, &start);

	while (1)
		if (start == 1)
			break;

	del_timer();
}

char wait_for_input_timeout(int fd, unsigned long ms)
{
	char c;
	int start = 0;
	int ret = 0;

	set_timer(get_system_time() + ms, do_timer, &start);

	while (1) {
		ret = read(fd, &c, 0, 1, NONBLOCK);
		if (ret > 0)
			break;

		if (start == 1)
			break;
	}

	del_timer();

	return c;
}

void wait_for_event_timeout(void *data, int (*expr)(void *data),
			    unsigned long ms)
{
	int start = 0;

	set_timer(get_system_time() + ms, do_timer, &start);

	while (1) {
		if (expr(data))
			break;

		if (start == 1)
			break;
	}

	del_timer();
}
