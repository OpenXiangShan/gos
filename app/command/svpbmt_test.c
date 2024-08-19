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
#include "mm.h"
#include "vmap.h"
#include "clock.h"

static int cmd_svpbmt_test_handler(int argc, char *argv[], void *priv)
{
	char *addr;
	unsigned long *pte;
	unsigned long start;
	int i, __attribute__((unused)) tmp;

	addr = (char *)vmem_alloc(4096, GFP_NOCACHE);
	pte = mmu_get_pte((unsigned long)addr);

	print("svpbmt nocache test...\n");
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 10000 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 10000; i++)
		tmp = addr[0];
	print("cost: %d ticks\n", get_system_tick() - start);

	vmem_free(addr, 4096);

	addr = (char *)vmem_alloc(4096, NULL);
	pte = mmu_get_pte((unsigned long)addr);

	print("\n");
	print("cacheable test...\n");
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 10000 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 10000; i++)
		tmp = addr[0];
	print("cost: %d ticks\n", get_system_tick() - start);

	vmem_free(addr, 4096);

	return 0;
}

static const struct command cmd_svpbmt_test = {
	.cmd = "svpbmt_test",
	.handler = cmd_svpbmt_test_handler,
	.priv = NULL,
};

int cmd_svpbmt_test_init()
{
	register_command(&cmd_svpbmt_test);

	return 0;
}

APP_COMMAND_REGISTER(svpbmt_test, cmd_svpbmt_test_init);
