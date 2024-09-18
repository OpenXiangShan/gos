/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

	while (strncmp(device_entry->compatible, "THE END", sizeof("THE_END"))) {
		driver_nr_tmp = driver_nr;
		for (driver_entry = driver_tmp; driver_nr_tmp;
		     driver_entry++, driver_nr_tmp--) {
			if (!strncmp
			    (driver_entry->compatible, device_entry->compatible,
			     128)) {

				driver_entry->init(device_entry->start, device_entry->len, &earlycon);
				earlycon.early_print_enable = 1;
			}
		}
		device_entry++;
	}

	return 0;
}
