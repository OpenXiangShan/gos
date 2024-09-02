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

/* 1. flush all display
*  2. gpa display
*/
static char c_flag = 0;
static char d_flag = 0;

static void Usage(void)
{
	print("Usage: g_stage_test [cmd] \n");
	print("cmd option:\n");
	print("    -- hfence_gvma.all\n");
	print("    -- hfence_gvma.gpa\n");
}

static void memory_test(struct vcpu *vcpu)
{
	struct virt_run_params *params;

	params =
	    (struct virt_run_params *)mm_alloc(sizeof(struct virt_run_params));
	if (!params) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return;
	}

	strcpy((char *)params->command, "memory_test");
	params->argc = 2;
	if (c_flag == 1)
		strcpy(params->argv[0], "1");	/*flush all dispaly */
	else if (c_flag == 2)
		strcpy(params->argv[0], "2");	/*flush gva dispaly */

	if (d_flag == 0)
		strcpy(params->argv[1], "0");	/*not display process print */
	else if (d_flag == 1)
		strcpy(params->argv[1], "1");	/*display test result */

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
	hva1 = hva + 64;	/*the next cache line */
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
	c_flag = 1;
	create_task("memory_test1", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);
	sleep_to_timeout(1000);

	page_table_remapping_gstage_memory(vcpu);
	print("----->before fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);
	sleep_to_timeout(1000);
	vcpu_set_request(vcpu, VCPU_REQ_FENCE_GVMA_ALL);

	d_flag = 1;
	print("----->after fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);

}

static void hfence_gvma_gpa()
{
	struct vcpu *vcpu;

	vcpu = vcpu_create();
	if (!vcpu)
		return;
	c_flag = 2;

	create_task("memory_test1", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);
	sleep_to_timeout(1000);

	page_table_remapping_gstage_memory(vcpu);
	vcpu->v_gpa = (struct vcpu_gpa *)mm_alloc(sizeof(struct vcpu_gpa));
	if (!vcpu->v_gpa) {
		print("%s -- Out of memory\n", __FUNCTION__);
		return;
	}

	vcpu->v_gpa->gpa = vcpu->host_memory_test_pa;
	append_vcpu_vgpalist(vcpu->v_gpa);
	print("----->before fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);
	sleep_to_timeout(1000);

	vcpu_set_request(vcpu, VCPU_REQ_FENCE_GVMA_GPA);

	d_flag = 1;
	print("----->after fence_gvma.all va value:\n");
	create_task("memory_test2", (void *)memory_test, (void *)vcpu, 0, NULL,
		    0, NULL);
}

static int cmd_g_stage_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		print("Invalid input params\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "hfence_gvma.all", sizeof("hfence_gvma.all")))
		hfence_gvma_all();
	else if (!strncmp
		 (argv[0], "hfence_gvma.gpa", sizeof("hfence_gvma.gpa")))
		hfence_gvma_gpa();
	else {
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
