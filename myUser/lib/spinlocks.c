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

#include "barrier.h"
#include "spinlocks.h"

static int spin_lock_check(spinlock_t * lock)
{
	__smp_mb();
	return (lock->lock == 0) ? 0 : 1;
}

static int spin_trylock(spinlock_t * lock)
{
	int tmp = 1, busy;

	__asm__ __volatile__("	amoswap.w %0, %2, %1\n"
			     RISCV_ACQUIRE_BARRIER:"=r"(busy), "+A"(lock->lock)
			     :"r"(tmp)
			     :"memory");

	return !busy;
}

void spin_lock(spinlock_t * lock)
{
	while (1) {
		if (spin_lock_check(lock))
			continue;

		if (spin_trylock(lock))
			break;
	}
}

void spin_unlock(spinlock_t * lock)
{
	__smp_store_release(&lock->lock, 0);
}
