#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "irq.h"
#include "string.h"
#include "../command.h"
#include "../drivers/irqchip/plic/plic.h"

static void Usage()
{
	print("plic_set_affinity [hwirq] [cpu_id]\n");
}

static int cmd_plic_set_affinity_handler(int argc, char *argv[], void *priv)
{
	struct irq_domain *irq_domain = plic_get_irq_domain();
	int cpu, hwirq;

	if (argc != 2) {
		print("Invalid input params.\n");
		Usage();
		return -1;
	}

	if (!irq_domain)
		return -1;

	hwirq = atoi(argv[0]);
	cpu = atoi(argv[1]);

	irq_domain_set_affinity(irq_domain, hwirq, cpu);

	return 0;
}

static const struct command cmd_plic_set_affinity = {
	.cmd = "plic_set_affinity",
	.handler = cmd_plic_set_affinity_handler,
	.priv = NULL,
};

int plic_set_affinity_init()
{
	register_command(&cmd_plic_set_affinity);

	return 0;
}

APP_COMMAND_REGISTER(plic_set_affinity, plic_set_affinity_init);