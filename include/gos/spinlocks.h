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

#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#include "asm/asm-irq.h"
#include "gos.h"

typedef unsigned int irq_flags_t;

#if CONFIG_USE_TICKET_SPINLOCK
typedef struct {
	volatile long lock;
	volatile long in_turn;
} spinlock_t;
#define __SPINLOCK_INIT(lock_ptr) \
	(lock_ptr)->lock = 0; \
	(lock_ptr)->in_turn = 1

#define __SPINLOCK_INITIALIZER   \
	{ .lock = 0, .in_turn = 1}

#else
typedef struct {
	volatile long lock;
} spinlock_t;

#define __SPINLOCK_INIT(lock_ptr) \
	(lock_ptr)->lock = 0

#define __SPINLOCK_INITIALIZER   \
	{ .lock = 0, }
#endif

#define DEFINE_SPINLOCK(lock) spinlock_t lock = __SPINLOCK_INITIALIZER

#define spin_lock_irqsave(lock, flags) \
do {                                   \
	local_irq_save(flags);         \
	spin_lock(lock);              \
} while(0)                             \

#define spin_unlock_irqrestore(lock, flags) \
do {                                        \
	spin_unlock(lock);                 \
	local_irq_restore(flags);	    \
} while(0)                                  \

void spin_lock(spinlock_t * lock);
void spin_unlock(spinlock_t * lock);

#endif
