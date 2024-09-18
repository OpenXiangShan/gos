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

#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "pci_device_driver.h"

static int cmd_lspci_handler(int argc, char *argv[], void *priv)
{
	walk_pci_devices();

	return 0;
}

static const struct command cmd_lspci = {
	.cmd = "lspci",
	.handler = cmd_lspci_handler,
	.priv = NULL,
};

int cmd_lspci_init()
{
	register_command(&cmd_lspci);

	return 0;
}

APP_COMMAND_REGISTER(lspci, cmd_lspci_init);
