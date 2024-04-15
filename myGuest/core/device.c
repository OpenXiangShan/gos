#include "device.h"
#include "uart.h"
#include "asm/pgtable.h"

void create_devices(void)
{
	struct device_init_entry *entry =
	    (struct device_init_entry *)FIXMAP_HW_START;

	myGuest_uart_init(entry);
}
