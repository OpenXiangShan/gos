#include <device.h>
#include <string.h>
#include "vmap.h"

extern int mmu_is_on;

static struct early_print_device earlycon = { 0, };

void uart_putc(char c)
{
	char str[2];

	if (earlycon.early_print_enable != 1)
		return;

	if (!earlycon.write)
		return;

	if (c == '\n') {
		str[0] = '\r';
		str[1] = c;
		str[2] = 0;
	} else {
		str[0] = c;
		str[1] = 0;
	}

	earlycon.write(str);
}

void uart_puts(char *str)
{
	if (earlycon.early_print_enable == 1)
		if (earlycon.write)
			earlycon.write(str);
}

int uart_init(void)
{
	return 0;
}

int early_print_setup(struct device_init_entry *hw)
{
	extern unsigned long EARLYCON_INIT_TABLE, EARLYCON_INIT_TABLE_END;

	int driver_nr =
	    (struct earlycon_init_entry *)&EARLYCON_INIT_TABLE_END -
	    (struct earlycon_init_entry *)&EARLYCON_INIT_TABLE;
	struct earlycon_init_entry *driver_tmp =
	    (struct earlycon_init_entry *)&EARLYCON_INIT_TABLE;
	int driver_nr_tmp = 0;
	struct earlycon_init_entry *driver_entry;
	struct device_init_entry *device_entry = hw;
	unsigned long base;

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE_END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {
				if (mmu_is_on)
					base = (unsigned long)ioremap((void *)
								      device_entry->start, device_entry->len, 0);
				else
					base = device_entry->start;

				driver_entry->init(base, &earlycon);
				earlycon.early_print_enable = 1;
			}
		}
		device_entry++;
	}

	return 0;
}
