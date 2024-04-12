#ifndef __SPINLOCK_H
#define __SPINLOCK_H

typedef unsigned int irq_flags_t;

typedef struct {
	volatile long lock;
} spinlock_t;

#define __SPINLOCK_INIT(lock_ptr) \
	(lock_ptr)->lock = 0

#define __SPINLOCK_INITIALIZER   \
	{ .lock = 0, }

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
