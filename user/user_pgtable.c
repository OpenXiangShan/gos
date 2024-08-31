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

#include "mm.h"
#include "asm/pgtable.h"
#include "task.h"

int user_page_mapping(unsigned long phy, unsigned long virt, unsigned int size)
{
	struct task *task = get_current_task();
	pgprot_t pgprot;

	pgprot =
	    __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
		     _PAGE_DIRTY | _PAGE_USER);

	return mmu_user_page_mapping(task->pgdp, phy, virt, size, pgprot);
}

int user_page_mapping_pg(unsigned long phy, unsigned long virt, unsigned int size,
		pgprot_t pgprot)
{
	struct task *task = get_current_task();

	return mmu_user_page_mapping(task->pgdp, phy, virt, size, pgprot);
}
