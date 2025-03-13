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
#include <asm/mmio.h>
#include "gos.h"
#include "../command.h"
#include "mm.h"
#include "vmap.h"
#include "clock.h"
#include "asm/sbi.h"

#define MMIO_BASE_FOR_TEST 0x310b0000UL  /* Choose ns16550a controller reg as test target*/
#define MMIO_SIZE_FOR_TEST 32

//smblockctl
void set_csr_ot_bit7(void)
{
#ifdef CONFIG_SELECT_KMH_FPGA
	register unsigned long __v;

	__asm__ __volatile__ ("csrr %0, 0x5C3"	\
			      : "=r" (__v) :			\
			      : "memory");				\
	__v |= (1 << 7);

	__asm__ __volatile__ ("csrw 0x5C3 , %0"	\
			      : : "rK" (__v)			\
			      : "memory");

	print("smblockctl: 0x%lx\n", __v);
#endif
	return;
}

static int cmd_svpbmt_test_handler(int argc, char *argv[], void *priv)
{
	char *addr;
	unsigned int *mmio_va;
	unsigned long *pte;
	unsigned long start;
	int i, __attribute__((unused)) tmp;
	unsigned long nocache_time;
	unsigned long cache_time;

	unsigned long base = MMIO_BASE_FOR_TEST;
	int len = MMIO_SIZE_FOR_TEST;
	print("####################\n");
	print("1. svpbmt MMIO test...\n");
	mmio_va = ioremap((void *)base, len, GFP_IO);
	print("Set pte IO property done\n");
	pte = mmu_get_pte((unsigned long)mmio_va);
	print("mmio_va:0x%lx pte:0x%lx\n", mmio_va, *pte);
	print("read MMIO [0x%lx] %d times...\n", mmio_va, len/sizeof(*mmio_va));
	start = get_system_tick();
	for (i = 0; i < len; i += sizeof(*mmio_va)){
	    tmp = readl(mmio_va++);
	}
	print("cost: %d ticks\n", cache_time = get_system_tick() - start);
	iounmap((void *)mmio_va, len);

	print("\n####################\n");
	print("2. svpbmt DDR test...\n");

	print("2.1 io test...\n");
	addr = (char *)vmem_alloc(4096, GFP_IO);
	print("Set pte IO property done\n");
	pte = mmu_get_pte((unsigned long)addr);
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 4096 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 4096; i++)
		tmp = addr[i];
	print("cost: %d ticks\n", cache_time = get_system_tick() - start);
	vmem_free(addr, 4096);

	print("menvcfg: 0x%lx\n", sbi_get_csr_menvcfg());

	print("\n2.2 no cacheable test...\n");
	addr = (char *)vmem_alloc(4096, GFP_NOCACHE);
	print("Set pte NOCACHE property done\n");
	pte = mmu_get_pte((unsigned long)addr);
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 4096 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 4096; i++)
		tmp = addr[i];
	print("cost: %d ticks\n", nocache_time = get_system_tick() - start);
	vmem_free(addr, 4096);

	print("\n2.3 no cacheable(set_csr_ot_bit7) test...\n");
	addr = (char *)vmem_alloc(4096, GFP_NOCACHE);
	pte = mmu_get_pte((unsigned long)addr);
	// OT   smblockctl
	set_csr_ot_bit7();
	print("set_csr_ot_bit7 done\n");

	print("menvcfg: 0x%lx\n", sbi_get_csr_menvcfg());

	print("Set pte NOCACHE property done\n");
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 4096 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 4096; i++)
		tmp = addr[i];

	print("cost: %d ticks\n", nocache_time = get_system_tick() - start);
	vmem_free(addr, 4096);

	print("\n2.4 cacheable test...\n");
	addr = (char *)vmem_alloc(4096, NULL);
	print("Set pte PMA property done\n");
	pte = mmu_get_pte((unsigned long)addr);
	print("addr:0x%lx pte:0x%lx\n", addr, *pte);
	print("read 0x%lx 4096 times...\n", addr);
	start = get_system_tick();
	for (i = 0; i < 4096; i++)
		tmp = addr[i];
	print("cost: %d ticks\n", cache_time = get_system_tick() - start);
	vmem_free(addr, 4096);

	if(cache_time < nocache_time)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}

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
