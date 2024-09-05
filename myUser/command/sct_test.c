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
#include "command.h"
#include "printf.h"
#include <../asm/csr.h>
#include "string.h"

#define is_csr_change(__reg)	\
({						\
	unsigned long __v;			\
	__v = read_csr(__reg);		\
	__asm__ volatile ("nop");	\
	__asm__ volatile ("nop");	\
	__asm__ volatile ("nop");	\
	__v = read_csr(__reg) - __v;	\
	__v;					\
})

void scounter_test(int val)
{
	switch (val) {
	case 0xc00:
		printf("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
		break;
	case 0xc01:
		printf("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
		break;
	case 0xc02:
		printf("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));
		break;
	case 0xc03:
		printf("CSR_HPMCOUNTER3 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER3));
		break;
	case 0xc04:
		printf("CSR_HPMCOUNTER4 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER4));
		break;
	case 0xc05:
		printf("CSR_HPMCOUNTER5 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER5));
		break;
	case 0xc06:
		printf("CSR_HPMCOUNTER6 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER6));
		break;
	case 0xc07:
		printf("CSR_HPMCOUNTER7 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER7));
		break;
	case 0xc08:
		printf("CSR_HPMCOUNTER8 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER8));
		break;
	case 0xc09:
		printf("CSR_HPMCOUNTER9 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER9));
		break;
	case 0xc0a:
		printf("CSR_HPMCOUNTER10 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER10));
		break;
	case 0xc0b:
		printf("CSR_HPMCOUNTER11 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER11));
		break;
	case 0xc0c:
		printf("CSR_HPMCOUNTEr12 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER12));
		break;
	case 0xc0d:
		printf("CSR_HPMCOUNTER13 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER13));
		break;
	case 0xc0e:
		printf("CSR_HPMCOUNTER14 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER14));
		break;
	case 0xc0f:
		printf("CSR_HPMCOUNTER15 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER15));
		break;
	case 0xc10:
		printf("CSR_HPMCOUNTER16 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER16));
		break;
	case 0xc11:
		printf("CSR_HPMCOUNTER17 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER17));
		break;
	case 0xc12:
		printf("CSR_HPMCOUNTER18 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER18));
		break;
	case 0xc13:
		printf("CSR_HPMCOUNTER19 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER19));
		break;
	case 0xc14:
		printf("CSR_HPMCOUNTER20 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER20));
		break;
	case 0xc15:
		printf("CSR_HPMCOUNTER21 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER21));
		break;
	case 0xc16:
		printf("CSR_HPMCOUNTER22 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER22));
		break;
	case 0xc17:
		printf("CSR_HPMCOUNTER23 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER23));
		break;
	case 0xc18:
		printf("CSR_HPMCOUNTER24 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER24));
		break;
	case 0xc19:
		printf("CSR_HPMCOUNTER25 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER25));
		break;
	case 0xc1a:
		printf("CSR_HPMCOUNTER26 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER26));
		break;
	case 0xc1b:
		printf("CSR_HPMCOUNTER27 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER27));
		break;
	case 0xc1c:
		printf("CSR_HPMCOUNTER28 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER28));
		break;
	case 0xc1d:
		printf("CSR_HPMCOUNTER29 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER29));
		break;
	case 0xc1e:
		printf("CSR_HPMCOUNTER30 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER30));
		break;
	case 0xc1f:
		printf("CSR_HPMCOUNTER31 value : 0x%lx\n",
		       read_csr(CSR_HPMCOUNTER31));
		break;
	case 0xda0:
		printf("CSR_SSCOUNTOVF value : 0x%lx\n",
		       read_csr(CSR_SSCOUNTOVF));
		break;
	case 0x106:
		printf("CSR_SCOUNTEREN value : 0x%lx\n",
		       read_csr(CSR_SCOUNTEREN));
		break;
	case 0x100:
		printf("CSR_SSTATUS value : 0x%lx\n", read_csr(CSR_SSTATUS));
		break;
	case 0x306:
		printf("CSR_MCOUNTEREN value : 0x%lx\n",
		       read_csr(CSR_MCOUNTEREN));
		break;
	default:
		printf("-------> get csr not support, try again \n");
		break;
	}

        printf("TEST PASS\n");
}

static void read_Zicntr()
{
	printf("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
	printf("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
	printf("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));

	if(is_csr_change(CSR_CYCLE)&&
	   is_csr_change(CSR_TIME)&&
	   is_csr_change(CSR_INSTRET))
	{
		printf("TEST PASS\n");
	}
	else
	{
		printf("TEST FAIL\n");
	}
}

static void read_Zihpm()
{

	printf("CSR_HPMCOUNTER3 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER3));
	printf("CSR_HPMCOUNTER4 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER4));
	printf("CSR_HPMCOUNTER5 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER5));
	printf("CSR_HPMCOUNTER6 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER6));
	printf("CSR_HPMCOUNTER7 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER7));
	printf("CSR_HPMCOUNTER8 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER8));
	printf("CSR_HPMCOUNTER9 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER9));
	printf("CSR_HPMCOUNTER10 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER10));
	printf("CSR_HPMCOUNTER11 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER11));
	printf("CSR_HPMCOUNTEr12 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER12));
	printf("CSR_HPMCOUNTER13 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER13));
	printf("CSR_HPMCOUNTER14 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER14));
	printf("CSR_HPMCOUNTER15 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER15));
	printf("CSR_HPMCOUNTER16 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER16));
	printf("CSR_HPMCOUNTER17 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER17));
	printf("CSR_HPMCOUNTER18 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER18));
	printf("CSR_HPMCOUNTER19 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER19));
	printf("CSR_HPMCOUNTER20 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER20));
	printf("CSR_HPMCOUNTER21 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER21));
	printf("CSR_HPMCOUNTER22 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER22));
	printf("CSR_HPMCOUNTER23 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER23));
	printf("CSR_HPMCOUNTER24 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER24));
	printf("CSR_HPMCOUNTER25 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER25));
	printf("CSR_HPMCOUNTER26 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER26));
	printf("CSR_HPMCOUNTER27 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER27));
	printf("CSR_HPMCOUNTER28 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER28));
	printf("CSR_HPMCOUNTER29 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER29));
	printf("CSR_HPMCOUNTER30 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER30));
	printf("CSR_HPMCOUNTER31 value : 0x%lx\n", read_csr(CSR_HPMCOUNTER31));
	
	printf("TEST PASS\n");
}

static void Usage(void)
{
	printf("Usage: sct_test [mode] [csr_num] \n");
	printf("mode option:\n");
	printf("    -- 0 (RAV23 profile Zicntr test)\n");
	printf("         --> user_run sct_test 0\n");
	printf("    -- 1 (RAV23 profile Zihpm test)\n");
	printf("         --> user_run sct_test 1\n");
	printf("    -- 2 (read csr register)\n");
	printf("----------------------------------------------\n");
	printf("csr_num option:\n");
	printf("    -- csr register \n");
}

static int cmd_scounter_test_handler(int argc, char *argv[], void *priv)
{
	if (argc < 1) {
		Usage();
		printf("invalid input param. \n");
		return -1;
	}
#if DEBUG
	int i;
	for (i = 0; i < argc; i++)
		printf("param is argv[%d]=%d \n", i, argv[i]);
#endif
	if (0 == atoi(argv[0]))
		read_Zicntr();
	else if (1 == atoi(argv[0]))
		read_Zihpm();
	else if (2 == atoi(argv[0]))
		scounter_test(atoi(argv[1]));

	return 0;
}

static const struct command cmd_scounter_test = {
	.cmd = "sct_test",
	.handler = cmd_scounter_test_handler,
};

int cmd_scounter_test_init()
{

	register_command(&cmd_scounter_test);

	return 0;
}

APP_COMMAND_REGISTER(sct_test, cmd_scounter_test_init);
