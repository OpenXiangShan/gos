#ifndef SBI_H
#define SBI_H

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_GET_CPU_CYCLE 0x200

struct sbi_trap_hw_context;
struct sbi_trap_regs;

void sbi_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);
void sbi_trap_handler(struct sbi_trap_regs *regs);

#endif
