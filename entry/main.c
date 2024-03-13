#include <uart.h>
#include <device.h>
#include <print.h>
#include <mm.h>
#include <irq.h>
#include <trap.h>
#include <asm/csr.h>
#include <timer.h>
#include <shell.h>

extern const char logo[];

void start_gos(unsigned int hart_id, struct device_init_entry *hw)
{
	early_print_setup(hw);

	print(logo);
	print("Hello, gos!\n");

	//__asm__ __volatile__ ("j  .\n");

	mm_init(hw);
	trap_init();
	irq_init();
	irqchip_setup(hw);
	init_timer(hw);
	device_driver_init(hw);

	print("local irq enable!!!\n");
	enable_local_irq();

	shell_init();

#if 0				//mm_alloc mm_free test
	void *addr;
	int size;

	size = 1024;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);

	mm_free(addr, size);

	size = 4097;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);

	size = 8192;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);

	size = 4096;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);

	size = 64 * 4096;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);
	mm_free(addr, size);

	size = 4096;
	addr = mm_alloc(size);
	print("mm_alloc size:%d addr: 0x%x\n", size, addr);

#endif

}
