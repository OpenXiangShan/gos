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
