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
	print("%s %d\n", __FUNCTION__, __LINE__);
	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		print("%s %d %s\n", __FUNCTION__, __LINE__,
		      device_entry->compatible);
		if (!strncmp(device_entry->compatible, "scheduler", 128)) {
			scheduler_init(device_entry->start, device_entry->len,
				       NULL);
		}
		device_entry++;
	}
}
