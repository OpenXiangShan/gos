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

#include <asm/barrier.h>
#include "spinlocks.h"
#include "gos.h"

#if CONFIG_USE_TICKET_SPINLOCK
void spin_lock(spinlock_t * lock)
{
	unsigned int _lock_value = 0;

	__asm__ __volatile__ (
		"lock: lr.w.aq %1, (%2)\n"
		"addi %0, %1, 1\n"
		"sc.w.rl %1, %0, (%2)\n"
		"bnez %1, lock\n"
		: "=&r"(_lock_value)
		: "r"(1), "r"(&lock->lock)
		: "memory"
	);

	while (_lock_value != lock->in_turn) ;
}

void spin_unlock(spinlock_t * lock)
{
	unsigned int temp;
	__asm__ __volatile__ (
		"amoadd.w.rl %0, %1, (%2)\n"
		: "=&r"(temp)
		: "r"(1), "r"(&lock->in_turn)
		: "memory"
	);
	mb();
}
#endif
