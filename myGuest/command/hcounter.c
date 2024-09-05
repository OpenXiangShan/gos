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

#include "command.h"
#include "print.h"
#include <string.h>
#include "asm/type.h"
#include "asm/csr.h"

static void Usage(void)
{
	print("Usage: csr_ctl [mode] [csr_num] \n");
	print("mode option:\n");
	print("    -- 1 (read csr register)\n");
}

void hcounter_read(unsigned long c_num)
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
	default:
		print("-------> get csr not support, try again \n");
		break;
	}
}

static int cmd_hcounter_ctl_handler(int argc, char *argv[], void *priv)
{
	unsigned int mode;
	unsigned long csr_num;

	if (argc < 1) {
		Usage();
		print("invalid input param.\n");
		return -1;
	}

	mode = atoi(argv[0]);
	csr_num = atoi(argv[1]);
	if (mode == 1)
		hcounter_read(csr_num);

	return 0;
}

static const struct command cmd_hcounter_ctl = {
	.cmd = "hcounter_ctl",
	.handler = cmd_hcounter_ctl_handler,
	.priv = NULL,
};

int cmd_hcounter_ctl_init()
{
	register_command(&cmd_hcounter_ctl);

	return 0;
}

APP_COMMAND_REGISTER(hcounter_ctl, cmd_hcounter_ctl_init);
