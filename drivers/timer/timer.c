#include <irq.h>
#include <device.h>
#include <asm/type.h>
#include <asm/trap.h>
#include <asm/mmio.h>
#include <asm/csr.h>
#include <print.h>
#include <asm/sbi.h>
#include <timer.h>

#define HZ 1000

#define CLINT_TIMER_CMP 0x4000
#define CLINT_TIMER_VAL 0xbff8

unsigned long clint_freq;
unsigned long volatile jiffies;
static unsigned long base_address;

struct clint_priv_data {
	unsigned long clint_freq;
};

unsigned long get_cycles(void)
{
	return readq(base_address + CLINT_TIMER_VAL);
}

static void timer_handle_irq(void *data)
{
	csr_clear(sie, SIE_STIE);
	jiffies++;

	if (jiffies >= get_timer_event_ms()) {
		do_timer_handler();
	}

	sbi_set_timer(get_cycles() + clint_freq / HZ);
	csr_set(sie, SIE_STIE);
}

void __timer_init()
{
	jiffies = 0;
	sbi_set_timer(get_cycles() + clint_freq / HZ);
	csr_set(sie, SIE_STIE);
}

int clint_timer_init(unsigned long base, struct irq_domain *d, void *priv)
{
	struct clint_priv_data *data = (struct clint_priv_data *)priv;

	print("%s -- base:0x%x, clint_freq:0x%x\n", __FUNCTION__, base,
	      data->clint_freq);

	if (!data) {
		print("%s %s %d can not find clint info...\n", __FILE__,
		      __FUNCTION__, __LINE__);
		return -1;
	}

	clint_freq = data->clint_freq;

	base_address = base;

	__timer_init();

	register_device_irq(d, INTERRUPT_CAUSE_TIMER, timer_handle_irq, NULL);

	return 0;
}

TIMER_REGISTER(clint, clint_timer_init, "clint");
