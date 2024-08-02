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
#include "print.h"
#include "asm/csr.h"
#include "timer.h"
#include "asm/sbi.h"
#include "asm/type.h"
#include "asm/mmio.h"
#include "vmap.h"
#include "gos-auto/autoconf.h"

#define CLINT_TIMER_CMP 0x4000
#define CLINT_TIMER_VAL 0xbff8

#define MIP_SSIP  (1UL << IRQ_S_SOFT)
#define MIP_STIP  (1UL << IRQ_S_TIMER)
#define MIP_SEIP  (1UL << IRQ_S_EXT)
#define MIP_MTIP  (1UL << IRQ_M_TIMER)
#define MIP_MEIP  (1UL << IRQ_M_EXT)
#define MIP_MSIP  (1UL << IRQ_M_SOFT)

unsigned long clint_freq;
static unsigned long base_address;
unsigned long jiffs;

struct clint_priv_data {
	unsigned long clint_freq;
};

static unsigned long get_jiffs(void)
{
	return jiffs;
}

static unsigned long get_cycles(void)
{
	return readq(base_address + CLINT_TIMER_VAL);
}

static int timer_set_next_event(unsigned long next)
{
#if CONFIG_ENABLE_VS_SSTC
	write_csr(CSR_STIMECMP, next);
#else
	sbi_set_timer(next);
#endif
	return 0;
}

int clint_do_timer_handler(void *data)
{
	unsigned long next;

	csr_clear(sip, MIP_STIP);
	next = get_cycles() + (clint_freq / 1000) * 100;
	timer_set_next_event(next);

	jiffs++;

	return 0;
}

int clint_timer_init(unsigned long base, unsigned int len, void *data)
{
	struct clint_priv_data *priv = (struct clint_priv_data *)data;
	unsigned long next;

	myGuest_print("%s base:0x%lx len:0x%x clint_freq:%d\n", __FUNCTION__,
		      base, len, priv->clint_freq);

	clint_freq = priv->clint_freq;
	base_address = (unsigned long)ioremap((void *)base, len, NULL);

	csr_set(sie, SIE_STIE);

	register_timer_irq_handler(clint_do_timer_handler, NULL);
	register_system_tick_handler(get_jiffs);

	next = get_cycles() + (clint_freq / 1000);
	timer_set_next_event(next);

	jiffs = 0;

	return 0;
}

DRIVER_REGISTER(riscv_timer, clint_timer_init, "clint");
