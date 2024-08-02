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
#include "asm/pgtable.h"
#include "vmap.h"

static int cmd_get_pte_handler(int argc, char *argv[], void *priv)
{
	void *addr;
	unsigned long *pte;

	addr = vmem_alloc(PAGE_SIZE, NULL);
	if (!addr) {
		print("%s -- Out of memory!!\n", __FUNCTION__);
		return -1;
	}

	pte = mmu_get_pte((unsigned long)addr);
	print("mmu_get_pte -- pte:0x%lx\n", *pte);

	pte = mmu_get_pte_level((unsigned long)addr, 1);
	print("mmu_get_pte_level(1) -- pte:0x%lx\n", *pte);
	pte = mmu_get_pte_level((unsigned long)addr, 2);
	print("mmu_get_pte_level(2) -- pte:0x%lx\n", *pte);
	pte = mmu_get_pte_level((unsigned long)addr, 3);
	print("mmu_get_pte_level(3) -- pte:0x%lx\n", *pte);

	return 0;
}

static const struct command cmd_get_pte = {
	.cmd = "get_pte_test",
	.handler = cmd_get_pte_handler,
	.priv = NULL,
};

int cmd_get_pte_init(void)
{
	register_command(&cmd_get_pte);

	return 0;
}

APP_COMMAND_REGISTER(get_pte, cmd_get_pte_init);
