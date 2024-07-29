#ifndef SBI_H
#define SBI_H

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 0x1
#define SBI_CONSOLE_GETCHAR 0x2
#define SBI_SET_MCOUNTEREN 0x3
#define SBI_GET_CPU_CYCLE 0x200
#define SBI_GET_CPU_ID 0x201
#define SBI_GET_CPU_MCOUNTEREN 0x202
#define SBI_HART_START 0x300
#define CSR_MCOUNTEREN 0x306
#define SBI_GET_M_MSI_DATA 0x500
#define SBI_GET_M_MSI_ADDR 0x501

typedef int (*do_ext_irq_t)(void);

struct sbi_trap_hw_context;
struct sbi_trap_regs;

void sbi_jump_to_next(struct sbi_trap_hw_context *ctx);
void sbi_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);
void sbi_trap_handler(struct sbi_trap_regs *regs);
void sbi_secondary_init(unsigned int hart_id, struct sbi_trap_hw_context *ctx);
void sbi_register_ext_irq_handler(do_ext_irq_t fn);

#endif
