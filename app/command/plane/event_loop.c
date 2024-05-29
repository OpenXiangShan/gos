#include <print.h>
#include <device.h>
#include "event_loop.h"

void event_loop_start(int (*process)(char command))
{
	int fd, i;
	int ret;
	char buf[64];

	fd = open("UART0");
	if (fd < 0) {
		print("open %s failed...\n", "UART0");
		return;
	}

	while (1) {
		ret = read(fd, buf, 0, 64, BLOCK);
		for (i = 0; i < ret; i++) {
			if (process(buf[i]))
				goto end;
		}
	}
end:
	return;
}
