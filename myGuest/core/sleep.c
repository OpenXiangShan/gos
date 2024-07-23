#include "device.h"
#include "asm/type.h"
#include "string.h"
#include "../drivers/scheduler.h"
#include "print.h"

int sleep(int ms)
{
	scheduler_sleep_to_timeout(ms);

	return 0;
}

int schedule(void)
{
	scheduler_schedule();

	return 0;
}

int scheduler_early_init(struct device_init_entry *entry)
{
	struct device_init_entry *device_entry = entry;
	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		if (!strncmp(device_entry->compatible, "scheduler", 128)) {
			scheduler_init(device_entry->start, device_entry->len,
				       NULL);
		}
		device_entry++;
	}

	return 0;
}
