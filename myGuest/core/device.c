#include "device.h"
#include "uart.h"
#include "asm/pgtable.h"
#include "asm/type.h"
#include "string.h"
#include "print.h"

void create_devices(void)
{
	extern struct driver_init_entry DRIVER_INIT_TABLE,
	    DRIVER_INIT_TABLE_END;
	struct device_init_entry *device_entry =
	    (struct device_init_entry *)FIXMAP_HW_START;
	struct driver_init_entry *driver_entry =
	    (struct driver_init_entry *)&DRIVER_INIT_TABLE;
	struct driver_init_entry *driver_entry_end =
	    (struct driver_init_entry *)&DRIVER_INIT_TABLE_END;
	struct driver_init_entry *tmp;
	int driver_nr = driver_entry_end - driver_entry;

	//myGuest_uart_init(device_entry);

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE END"))) {
		driver_nr = driver_entry_end - driver_entry;
		for (tmp = driver_entry; driver_nr > 0; driver_nr--, tmp++) {
			if (!strncmp
			    (tmp->compatible, device_entry->compatible, 128)) {
				tmp->init(device_entry->start,
					  device_entry->len,
					  device_entry->data);
			}
		}
		device_entry++;
	}
}
