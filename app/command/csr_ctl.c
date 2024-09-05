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

#include <print.h>
#include <device.h>
#include <string.h>
#include <asm/mmio.h>
#include "../command.h"
#include <asm/csr.h>
#include <asm/sbi.h>
#include "virt.h"

static void Usage(void)
{
	print("Usage: csr_ctl [mode] [csr_num] [value]\n");
	print("mode option:\n");
	print("    -- 1 (read csr register)\n");
	print("    -- 2 (write csr register)\n");
	print("    -- 3 (read counteren register)\n");
	print("        (read mcounteren register)\n");
	print("         --> csr_ctl  3  0  \n");
	print("        (read hcounteren register)\n");
	print("         --> csr_ctl  3  1  \n");
	print("    -- 4 (write counteren register)\n");
	print("        (write mcounteren register)\n");
	print("         --> csr_ctl  4  0 value\n");
	print("        (write hcounteren register)\n");
	print("         --> csr_ctl  4  1 value\n");
	print("----------------------------------------------\n");
	print("csr_num option:\n");
	print("    -- csr register \n");
	print("----------------------------------------------\n");
	print("value :\n");
	print("    -- write csr_register value \n");
}

static void csr_write(unsigned long c_num, unsigned long value)
{
	switch (c_num) {
	case 0xc00:
		write_csr(CSR_CYCLE, value);
		print("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
		break;
	case 0xc01:
		write_csr(CSR_TIME, value);
		print("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
		break;
	case 0xc02:
		write_csr(CSR_INSTRET, value);
		print("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));
		break;
	case 0xc03:
		write_csr(CSR_HPMCOUNTER3, value);
		print("CSR_HPMCOUNTER3 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER3));
		break;
	case 0xc04:
		write_csr(CSR_HPMCOUNTER4, value);
		print("CSR_HPMCOUNTER4 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER4));
		break;
	case 0xc05:
		write_csr(CSR_HPMCOUNTER5, value);
		print("CSR_HPMCOUNTER5 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER5));
		break;
	case 0xc06:
		write_csr(CSR_HPMCOUNTER6, value);
		print("CSR_HPMCOUNTER6 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER6));
		break;
	case 0xc07:
		write_csr(CSR_HPMCOUNTER7, value);
		print("CSR_HPMCOUNTER7 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER7));
		break;
	case 0xc08:
		write_csr(CSR_HPMCOUNTER8, value);
		print("CSR_HPMCOUNTER8 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER8));
		break;
	case 0xc09:
		write_csr(CSR_HPMCOUNTER9, value);
		print("CSR_HPMCOUNTER9 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER9));
		break;
	case 0xc0a:
		write_csr(CSR_HPMCOUNTER10, value);
		print("CSR_HPMCOUNTER10 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER10));
		break;
	case 0xc0b:
		write_csr(CSR_HPMCOUNTER11, value);
		print("CSR_HPMCOUNTER11 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER11));
		break;
	case 0xc0c:
		write_csr(CSR_HPMCOUNTER12, value);
		print("CSR_HPMCOUNTEr12 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER12));
		break;
	case 0xc0d:
		write_csr(CSR_HPMCOUNTER13, value);
		print("CSR_HPMCOUNTER13 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER13));
		break;
	case 0xc0e:
		write_csr(CSR_HPMCOUNTER14, value);
		print("CSR_HPMCOUNTER14 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER14));
		break;
	case 0xc0f:
		write_csr(CSR_HPMCOUNTER15, value);
		print("CSR_HPMCOUNTER15 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER15));
		break;
	case 0xc10:
		write_csr(CSR_HPMCOUNTER16, value);
		print("CSR_HPMCOUNTER16 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER16));
		break;
	case 0xc11:
		write_csr(CSR_HPMCOUNTER17, value);
		print("CSR_HPMCOUNTER17 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER17));
		break;
	case 0xc12:
		write_csr(CSR_HPMCOUNTER18, value);
		print("CSR_HPMCOUNTER18 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER18));
		break;
	case 0xc13:
		write_csr(CSR_HPMCOUNTER19, value);
		print("CSR_HPMCOUNTER19 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER19));
		break;
	case 0xc14:
		write_csr(CSR_HPMCOUNTER20, value);
		print("CSR_HPMCOUNTER20 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER20));
		break;
	case 0xc15:
		write_csr(CSR_HPMCOUNTER21, value);
		print("CSR_HPMCOUNTER21 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER21));
		break;
	case 0xc16:
		write_csr(CSR_HPMCOUNTER22, value);
		print("CSR_HPMCOUNTER22 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER22));
		break;
	case 0xc17:
		write_csr(CSR_HPMCOUNTER23, value);
		print("CSR_HPMCOUNTER23 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER23));
		break;
	case 0xc18:
		write_csr(CSR_HPMCOUNTER24, value);
		print("CSR_HPMCOUNTER24 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER24));
		break;
	case 0xc19:
		write_csr(CSR_HPMCOUNTER25, value);
		print("CSR_HPMCOUNTER25 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER25));
		break;
	case 0xc1a:
		write_csr(CSR_HPMCOUNTER26, value);
		print("CSR_HPMCOUNTER26 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER26));
		break;
	case 0xc1b:
		write_csr(CSR_HPMCOUNTER27, value);
		print("CSR_HPMCOUNTER27 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER27));
		break;
	case 0xc1c:
		write_csr(CSR_HPMCOUNTER28, value);
		print("CSR_HPMCOUNTER28 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER28));
		break;
	case 0xc1d:
		write_csr(CSR_HPMCOUNTER29, value);
		print("CSR_HPMCOUNTER29 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER29));
		break;
	case 0xc1e:
		write_csr(CSR_HPMCOUNTER30, value);
		print("CSR_HPMCOUNTER30 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER30));
		break;
	case 0xc1f:
		write_csr(CSR_HPMCOUNTER31, value);
		print("CSR_HPMCOUNTER31 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER31));
		break;
	case 0xda0:
		write_csr(CSR_SSCOUNTOVF, value);
		print("CSR_SSCOUNTOVF value : 0x%lx\n",
		      read_csr(CSR_SSCOUNTOVF));
		break;
	case 0x100:
		write_csr(CSR_SSTATUS, value);
		print("CSR_SSTATUS value : 0x%lx\n", read_csr(CSR_SSTATUS));
		break;
	case 0x106:
		write_csr(CSR_SCOUNTEREN, value);
		print("CSR_SCOUNTEREN value : 0x%lx\n",
		      read_csr(CSR_SCOUNTEREN));
		break;
	case 0x606:
		write_csr(CSR_HCOUNTEREN, value);
		print("CSR_HCOUNTEREN value : 0x%lx\n", read_csr(CSR_HCOUNTEREN));
		break;
	default:
		print("-------> get csr not support, try again \n");
		break;

	}
	
	print("TEST PASS\n");
}

void csr_read(unsigned long c_num)
{

	switch (c_num) {
	case 0xc00:
		print("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
		break;
	case 0xc01:
		print("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
		break;
	case 0xc02:
		print("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));
		break;
	case 0xc03:
		print("CSR_HPMCOUNTER3 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER3));
		break;
	case 0xc04:
		print("CSR_HPMCOUNTER4 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER4));
		break;
	case 0xc05:
		print("CSR_HPMCOUNTER5 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER5));
		break;
	case 0xc06:
		print("CSR_HPMCOUNTER6 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER6));
		break;
	case 0xc07:
		print("CSR_HPMCOUNTER7 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER7));
		break;
	case 0xc08:
		print("CSR_HPMCOUNTER8 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER8));
		break;
	case 0xc09:
		print("CSR_HPMCOUNTER9 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER9));
		break;
	case 0xc0a:
		print("CSR_HPMCOUNTER10 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER10));
		break;
	case 0xc0b:
		print("CSR_HPMCOUNTER11 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER11));
		break;
	case 0xc0c:
		print("CSR_HPMCOUNTEr12 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER12));
		break;
	case 0xc0d:
		print("CSR_HPMCOUNTER13 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER13));
		break;
	case 0xc0e:
		print("CSR_HPMCOUNTER14 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER14));
		break;
	case 0xc0f:
		print("CSR_HPMCOUNTER15 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER15));
		break;
	case 0xc10:
		print("CSR_HPMCOUNTER16 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER16));
		break;
	case 0xc11:
		print("CSR_HPMCOUNTER17 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER17));
		break;
	case 0xc12:
		print("CSR_HPMCOUNTER18 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER18));
		break;
	case 0xc13:
		print("CSR_HPMCOUNTER19 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER19));
		break;
	case 0xc14:
		print("CSR_HPMCOUNTER20 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER20));
		break;
	case 0xc15:
		print("CSR_HPMCOUNTER21 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER21));
		break;
	case 0xc16:
		print("CSR_HPMCOUNTER22 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER22));
		break;
	case 0xc17:
		print("CSR_HPMCOUNTER23 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER23));
		break;
	case 0xc18:
		print("CSR_HPMCOUNTER24 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER24));
		break;
	case 0xc19:
		print("CSR_HPMCOUNTER25 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER25));
		break;
	case 0xc1a:
		print("CSR_HPMCOUNTER26 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER26));
		break;
	case 0xc1b:
		print("CSR_HPMCOUNTER27 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER27));
		break;
	case 0xc1c:
		print("CSR_HPMCOUNTER28 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER28));
		break;
	case 0xc1d:
		print("CSR_HPMCOUNTER29 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER29));
		break;
	case 0xc1e:
		print("CSR_HPMCOUNTER30 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER30));
		break;
	case 0xc1f:
		print("CSR_HPMCOUNTER31 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER31));
		break;
	case 0xda0:
		print("CSR_SSCOUNTOVF value : 0x%lx\n",
		      read_csr(CSR_SSCOUNTOVF));
		break;
	case 0x106:
		print("CSR_SCOUNTEREN value : 0x%lx\n",
		      read_csr(CSR_SCOUNTEREN));
		break;
	case 0x100:
		print("CSR_SSTATUS value : 0x%lx\n", read_csr(CSR_SSTATUS));
		break;
	case 0x606:
		print("CSR_HCOUNTEREN value : 0x%lx\n", read_csr(CSR_HCOUNTEREN));
		break;
	default:
		print("-------> get csr not support, try again \n");
		break;

	}
	
	print("TEST PASS\n");
}

static int cmd_csr_ctl_handler(int argc, char *argv[], void *priv)
{
	unsigned int mode;
	unsigned long csr_num;
	unsigned long value;
	struct vcpu *vcpu;

	if (argc < 2) {
		Usage();
		print("invalid input param.\n");
		return -1;
	}

	if (!is_digit(argv[0]) || !is_digit(argv[1])) {
		print("invalid input param.\n");
		return -1;
	}
	mode = atoi(argv[0]);
	csr_num = atoi(argv[1]);
	if ((mode == 2) || (mode == 4))
		value = atoi(argv[2]);
#if DEBUG
	print("-----> Modfiy mode:%d ,csr %d, value:0x%lx \n", mode, csr_num,
	      value);
#endif
	vcpu = vcpu_create();
	if (!vcpu)
		return -1;

	if (mode == 1)
		csr_read(csr_num);
	else if (mode == 2)
		csr_write(csr_num, value);
	else if (mode == 3){
		if (csr_num == 0)
			print("mcounteren:0x%lx \n", sbi_get_cpu_mcounteren());
		else if (csr_num == 1)
			print("hcounteren: 0x%lx \n", vcpu->cpu_ctx.hcounteren);
	}else if (mode == 4) {
		if (csr_num == 0)
			sbi_set_mcounteren(value);
		else if (csr_num == 1)
			vcpu->cpu_ctx.hcounteren =  value;
	}

	print("\n");

	return 0;
}

static const struct command cmd_csr_ctl = {
	.cmd = "csr_ctl",
	.handler = cmd_csr_ctl_handler,
};

int cmd_csr_ctl_init()
{
	register_command(&cmd_csr_ctl);

	return 0;
}

APP_COMMAND_REGISTER(csr_ctl, cmd_csr_ctl_init);
