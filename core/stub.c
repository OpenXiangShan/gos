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

#include "print.h"
#include "vmap.h"
#include "mm.h"
#include "list.h"
#include "string.h"
#include "asm/pgtable.h"
#include "asm/tlbflush.h"
#include "asm/type.h"
#include "asm/ptregs.h"
#include "kallsyms.h"
#include "stub.h"

static LIST_HEAD(stubs);
static LIST_HEAD(stub_slots);

extern int mmu_is_on;

static void *get_free_stub_addr(int size)
{
	struct stub_slot *slot;
	unsigned long bitmap;
	int i = 0, nr = 0, size_nr = size / STUB_SLOT_PER_SIZE;
	unsigned long ret;

	list_for_each_entry(slot, &stub_slots, list) {
		ret = (unsigned long)slot->slot_addr;
		while (i < STUB_SLOT_TOTAL) {
			bitmap = slot->free_bitmap[i / 64];
			if (((bitmap >> (i % 64)) & 0x01) == 0) {
				if (++nr == size_nr)
					goto find;
			} else {
				nr = 0;
				ret += (nr + 1) * STUB_SLOT_PER_SIZE;
			}
			i++;
		}
	}

	return NULL;
find:
	bitmap |= (1UL << (i % 64));
	slot->free_bitmap[i / 64] = bitmap;

	return (void *)ret;
}

static int alloc_stub_buffer(void)
{
	struct stub_slot *s;
	void *insn_va, *insn_pa;
	pgprot_t pgprot;

	s = (struct stub_slot *)mm_alloc(sizeof(struct stub_slot));
	if (!s) {
		print("stub -- alloc_stub_buffer fail\n");
		return -1;
	}
	memset((char *)s, 0, sizeof(struct stub_slot));

	insn_va = vmap_alloc(PAGE_SIZE);
	if (!insn_va) {
		print("stub -- vmap_alloc fail\n");
		return 0;
	}
	insn_pa = mm_alloc(PAGE_SIZE);
	if (!insn_pa) {
		print("stub -- mm_alloc fail\n");
		return 0;
	}
	if (mmu_is_on)
		insn_pa = (void *)virt_to_phy(insn_pa);
	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY);
	if (mmu_page_mapping
	    ((unsigned long)insn_pa, (unsigned long)insn_va, PAGE_SIZE,
	     pgprot)) {
		print("stub -- mmu_page_mapping fail\n");
		goto free;
	}

	s->slot_addr = insn_va;
	list_add_tail(&s->list, &stub_slots);

	return 0;

free:

	vmap_free(insn_va, PAGE_SIZE);
	if (mmu_is_on)
		insn_pa = (void *)phy_to_virt(insn_pa);
	mm_free(insn_pa, PAGE_SIZE);

	return -1;
}

static void *alloc_insn_buffer(int size)
{
	void *addr;

get_free_slot:
	addr = get_free_stub_addr(size);
	if (addr)
		return addr;

	if (0 == alloc_stub_buffer())
		goto get_free_slot;

	return NULL;
}

static int setup_stub_insn(struct stub *s)
{
	unsigned short *insn = (unsigned short *)s->addr;
	unsigned int *next_insn;
	int insn_len = 2, size;

	s->opcode = (unsigned int)(*insn++);
	if (GET_INSN_LENGTH(s->opcode) == 4) {
		insn_len = 4;
		s->opcode |= (unsigned int)(*insn) << 16;
	}

	size = 8;
	s->previous_insn = alloc_insn_buffer(size);
	*((unsigned int *)s->previous_insn) = *((unsigned int *)s->addr);
	*((unsigned int *)s->addr) = __EBREAK_INSN;
	s->insn_len = insn_len;

	next_insn = (unsigned int *)(s->previous_insn + 4);
	*next_insn = __EBREAK_INSN;

	local_flush_icache_all();
	mb();

	return 0;
}

int gos_stub_do_process(struct pt_regs *regs)
{
	struct stub *s;
	void *addr = (void *)regs->sepc;
	int ret = -1;

	list_for_each_entry(s, &stubs, list) {
		if (s->addr == addr) {
			if (!strcmp(s->name, "handle_exception"))
				s->handler((struct pt_regs *)regs->a0);
			else if (!strcmp(s->name, "do_ebreak"))
				s->handler((struct pt_regs *)regs->a0);
			else if (s->handler)
				s->handler(regs);
			regs->sepc = (unsigned long)s->previous_insn;
			ret = 0;
		} else if (addr == s->previous_insn + 4) {
			regs->sepc = (unsigned long)(s->addr + 4);
			ret = 0;
		}
	}

	return ret;
}

int __attribute__((section(".gos_stub_func")))
register_stub(const char *name, void (*stub_handler)(struct pt_regs * regs))
{
	struct stub *s;
	void *addr;

	addr = (void *)kallsyms_lookup_name(name);
	if (!addr)
		return -1;

	s = (struct stub *)mm_alloc(sizeof(struct stub));
	if (!s)
		return -1;
	memset((char *)s, 0, sizeof(struct stub));

	strcpy(s->name, (char *)name);
	s->addr = addr;
	s->handler = stub_handler;

	setup_stub_insn(s);

	list_add_tail(&s->list, &stubs);

	return 0;
}

int __attribute__((section(".gos_stub_func")))
register_handle_exception_stub_handler(void (*handler)(struct pt_regs * regs))
{
	return register_stub("handle_exception", handler);
}

int __attribute__((section(".gos_stub_func")))
register_ebreak_stub_handler(void (*handler)(struct pt_regs * regs))
{
	return register_stub("do_ebreak", handler);
}
