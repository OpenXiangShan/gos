#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"
#include "mm.h"
#include "virt.h"
#include "../virt/machine.h"
#include "asm/tlbflush.h"
#include "task.h"
#include "asm/csr.h"
#include "gos.h"
#include "gos/cpu_tlb.h"

static void Usage(void)
{
	print("Usage: g_stage_test [cmd] \n");
	print("cmd option:\n");
	print("    -- hfence_gvma.all\n");
}

static void memory_test(struct vcpu *vcpu)
{
	struct virt_run_params *params;

	params = (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return;
	}

	strcpy((char *)params->command, "memory_test");
	params->argc = 2;
	strcpy(params->argv[0],  "1");	/*flush all dispaly*/
	strcpy(params->argv[1],  "0");	/*not display process print*/
	vcpu_run(vcpu, params);

	mm_free((void *)params, sizeof(struct virt_run_params));
}

static void page_table_remapping_gstage_memory(struct vcpu *vcpu)
{
	unsigned long hpa, hva, hpa1, hva1;
	unsigned int size;

	size = machine_get_memory_test_size(&vcpu->machine);

	hva = (unsigned long)mm_alloc(size);
	if (!hva) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return;
	}
	hpa = virt_to_phy(hva);
	hva1 = hva + 64;/*the next cache line*/
	hpa1 = virt_to_phy(hva1);

	strcpy((char *)hva, "The host memory pa1!");
	strcpy((char *)hva1, "The host memory pa2!");
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp, hpa,
			    vcpu->guest_memory_test_pa, size);
	gstage_page_mapping((unsigned long *)vcpu->machine.gstage_pgdp, hpa1,
			    vcpu->guest_memory_test_pa1, size);
}


static void hfence_gvma_all()
{
	struct vcpu *vcpu;

	vcpu = vcpu_create();
	if (!vcpu)
		return;
	create_task("memory_test1", (void *)memory_test, (void *)vcpu, 0, NULL, 0, NULL);
	sleep_to_timeout(1000);

	page_table_remapping_gstage_memory(vcpu);
	print("----->before fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL, 0, NULL);
	sleep_to_timeout(1000);
	vcpu_set_request(vcpu, VCPU_REQ_FENCE_GVMA_ALL);


	print("----->after fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL, 0, NULL);

}

static int cmd_g_stage_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "hfence_gvma.all", sizeof("hfence_gvma.all"))) {
		hfence_gvma_all();
	} else {
		print("Unsupport command\n");
		Usage();
		return -1;
	}

	return 0;
}

static const struct command cmd_g_stage_test = {
	.cmd = "g_stage_test",
	.handler = cmd_g_stage_test_handler,
	.priv = NULL,
};

int g_stage_init()
{
	register_command(&cmd_g_stage_test);

	return 0;
}

APP_COMMAND_REGISTER(g_stage_test, g_stage_init);
