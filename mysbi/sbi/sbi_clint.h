#ifndef SBI_CLINT_H
#define SBI_CLINT_H

#include "sbi_trap.h"

void sbi_timer_process(void);
void clint_timer_event_start(struct sbi_trap_hw_context *ctx,
			     unsigned long next_event);
int sbi_clint_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);

#endif
