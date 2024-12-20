#ifndef __STUB_H__
#define __STUB_H__

#include "list.h"
#include "asm/pgtable.h"
#include "asm/ptregs.h"

#define __INSN_LENGTH_MASK  (0x3UL)
#define __INSN_LENGTH_32    (0x3UL)

#define __EBREAK_INSN       (0x00100073)
#define __C_EBREAK_INSN     (0x9002)

struct stub {
	struct list_head list;
	char name[64];
	unsigned int opcode;
	int insn_len;
	void *addr;
	void *previous_insn;
	int previous_insn_size;
	void (*handler)(struct pt_regs * regs);
};

#define STUB_SLOT_SIZE PAGE_SIZE
#define STUB_SLOT_PER_SIZE 2
#define STUB_SLOT_TOTAL (STUB_SLOT_SIZE / STUB_SLOT_PER_SIZE)

struct stub_slot {
	struct list_head list;
	unsigned long free_bitmap[STUB_SLOT_TOTAL / 64];
	void *slot_addr;
};

void default_stub_handler(struct pt_regs *regs);
int gos_stub_do_process(struct pt_regs *regs);
int unregister_stub(const char *name);
int register_stub(const char *name,
		  void (*stub_handler)(struct pt_regs * regs));
int register_handle_exception_stub_handler(void (*handler)(struct pt_regs * regs));
int register_ebreak_stub_handler(void (*handler)(struct pt_regs * regs));
int unregister_handle_exception_stub_handler(void);
int unregister_ebreak_stub_handler(void);

#endif
