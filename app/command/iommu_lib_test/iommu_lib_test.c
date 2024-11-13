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
#include "../../command.h"
#include "iommu-lib.h"
#include "asm/mmio.h"
#include "dmac.h"
#include "my_dmac.h"
#include "mm.h"

static int cmd_iommu_lib_test_handler(int argc, char *argv[], void *priv)
{
	struct rv_iommu_pgtable_setup_struct info;
	struct rv_iommu *iommu;
	int err = 0;
	unsigned long base;
	void *ddtp;
	struct rv_iommu_lib_ops ops;
	char *src, *dst;
	unsigned long src_iova = 0, dst_iova = 0x1000, src_gpa =
	    0x7000, dst_gpa = 0x8000;

	base = (unsigned long)mm_alloc(2 * 1024 * 1024);

	ops.print = print;
	rv_iommu_lib_init(base, 2 * 1024 * 1024, &ops);

	info.fstage_en = 1;
	info.gstage_en = 1;
	info.pasid_en = 1;
	info.dpe_en = 1;
	info.devid = 8;
	info.gscid = 0;
	info.pscid = 0;
	info.pasid = 0;
	info.sv_mode = RISCV_IOMMU_DC_FSC_IOSATP_MODE_SV39;
	info.ddt_mode = DDTP_MODE_3LEVEL;
	info.pdt_mode = PDTP_MODE_1LEVEL;
	info.cap = 0;

	iommu = rv_iommu_pgtable_setup_init(&info, &err);
	if (!iommu) {
		print("%s %d err:%d\n", __FUNCTION__, __LINE__, err);
		goto end;
	}
	print("iommu:0x%lx\n", iommu);

	ddtp = rv_iommu_pgtable_setup(iommu, &err);
	if (!ddtp) {
		print("rv_iommu_pgtable_setup fail, err:%d\n", err);
		goto end;
	}

	writeq(0x10001000 + 0x10, (unsigned long)ddtp);

	src = (char *)mm_alloc(4096);
	dst = (char *)mm_alloc(4096);
	print("src:0x%lx dst:0x%lx\n", src, dst);

	err =
	    rv_iommu_page_mapping(iommu, src_iova, (unsigned long)src_gpa,
				  (unsigned long)src, 4096);
	if (err) {
		print("rv_iommu_page_mapping fail, err:%d\n", err);
		goto end2;
	}
	err =
	    rv_iommu_page_mapping(iommu, dst_iova, (unsigned long)dst_gpa,
				  (unsigned long)dst, 4096);
	if (err) {
		print("rv_iommu_page_mapping fail, err:%d\n", err);
		goto end2;
	}

	rv_iommu_show_pte_addr(iommu);

	dmac_test_data_init(src);
	dmac_ch1_single_transfer(0, 0, 0, (unsigned int)src_iova,
				 (unsigned int)dst_iova, 63, 0, 0, 2, 2, 0, 0);
	dmac_wait_for_complete();
	if (dmac_check_data(src, dst, 64))
		print("test pass1!!\n");
end2:
	mm_free((void *)src, 4096);
	mm_free((void *)dst, 4096);
end:
	mm_free((void *)base, 2 * 1024 * 1024);

	return -1;
}

static const struct command cmd_iommu_lib_test = {
	.cmd = "iommu_lib_test",
	.handler = cmd_iommu_lib_test_handler,
	.priv = NULL,
};

int cmd_iommu_lib_test_init()
{
	register_command(&cmd_iommu_lib_test);

	return 0;
}

APP_COMMAND_REGISTER(iommu_lib_test, cmd_iommu_lib_test_init);
