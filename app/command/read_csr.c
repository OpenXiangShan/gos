#include <asm/type.h>
#include <print.h>
#include <device.h>
#include <string.h>
#include <asm/mmio.h>
#include "../command.h"
#include <asm/csr.h>
#include <linux/string.h>

#if DEBUG
static void read_param(int num, char *argv[])
{
	char i;

	for (i = 0; i < num; i++)
		print("Argument %d is %s\n", i, argv[i]);

}
#endif

static int cmd_read_csr_handler(int argc, char *argv[], void *priv)
{
	unsigned long start;

	if (argc < 1) {
		print("invalid input param.\n");
		return -1;
	}

	if (!is_digit(argv[0])) {
		print("invalid input param.\n");
		return -1;
	}
	start = atoi(argv[0]);
	print("---------> get csr reg:0x%lx \n", start);

#if 1
	switch (start) {
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
	case 0x306:
		print("CSR_MCOUNTEREN value : 0x%lx\n",
		      read_csr(CSR_MCOUNTEREN));
		break;
	default:
		print("-------> get csr not support, try again \n");
		break;

	}
#endif
#if 0
	if (strcmp(*argv, "c00") == 0)
		print("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
	else if (strcmp(*argv, "c01") == 0)
		print("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
	else if (strcmp(*argv, "c02") == 0)
		print("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));
	else if (strcmp(*argv, "c03") == 0)
		print("CSR_HPMCOUNTER3 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER3));
	else if (strcmp(*argv, "c04") == 0)
		print("CSR_HPMCOUNTER4 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER4));
	else if (strcmp(*argv, "c05") == 0)
		print("CSR_HPMCOUNTER5 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER5));
	else if (strcmp(*argv, "c06") == 0)
		print("CSR_HPMCOUNTER6 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER6));
	else if (strcmp(*argv, "c07") == 0)
		print("CSR_HPMCOUNTER7 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER7));
	else if (strcmp(*argv, "c08") == 0)
		print("CSR_HPMCOUNTER8 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER8));
	else if (strcmp(*argv, "c09") == 0)
		print("CSR_HPMCOUNTER9 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER9));
	else if (strcmp(*argv, "c0a") == 0)
		print("CSR_HPMCOUNTER10 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER10));
	else if (strcmp(*argv, "c0b") == 0)
		print("CSR_HPMCOUNTER11 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER11));
	else if (strcmp(*argv, "c0c") == 0)
		print("CSR_HPMCOUNTER12 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER12));
	else if (strcmp(*argv, "c0d") == 0)
		print("CSR_HPMCOUNTER13 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER13));
	else if (strcmp(*argv, "c0e") == 0)
		print("CSR_HPMCOUNTER14 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER14));
	else if (strcmp(*argv, "c0f") == 0)
		print("CSR_HPMCOUNTER15 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER15));
	else if (strcmp(*argv, "c10") == 0)
		print("CSR_HPMCOUNTER16 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER16));
	else if (strcmp(*argv, "c11") == 0)
		print("CSR_HPMCOUNTER17 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER17));
	else if (strcmp(*argv, "c12") == 0)
		print("CSR_HPMCOUNTER18 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER18));
	else if (strcmp(*argv, "c13") == 0)
		print("CSR_HPMCOUNTER19 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER19));
	else if (strcmp(*argv, "c14") == 0)
		print("CSR_HPMCOUNTER20 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER20));
	else if (strcmp(*argv, "c15") == 0)
		print("CSR_HPMCOUNTER21 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER21));
	else if (strcmp(*argv, "c16") == 0)
		print("CSR_HPMCOUNTER22 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER22));
	else if (strcmp(*argv, "c17") == 0)
		print("CSR_HPMCOUNTER23 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER23));
	else if (strcmp(*argv, "c18") == 0)
		print("CSR_HPMCOUNTER24 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER24));
	else if (strcmp(*argv, "c19") == 0)
		print("CSR_HPMCOUNTER25 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER25));
	else if (strcmp(*argv, "c1a") == 0)
		print("CSR_HPMCOUNTER26 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER26));
	else if (strcmp(*argv, "c1b") == 0)
		print("CSR_HPMCOUNTER27 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER27));
	else if (strcmp(*argv, "c1c") == 0)
		print("CSR_HPMCOUNTER28 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER28));
	else if (strcmp(*argv, "c1d") == 0)
		print("CSR_HPMCOUNTER29 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER29));
	else if (strcmp(*argv, "c1e") == 0)
		print("CSR_HPMCOUNTER30 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER30));
	else if (strcmp(*argv, "c1f") == 0)
		print("CSR_HPMCOUNTER31 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER31));
	else if (strcmp(*argv, "da0") == 0)
		print("CSR_SSCOUNTOVF value : 0x%lx\n",
		      read_csr(CSR_SSCOUNTOVF));
	else if (strcmp(*argv, "100") == 0)
		print("CSR_SSTATUS value : 0x%lx\n", read_csr(CSR_SSTATUS));
	else if (strcmp(*argv, "104") == 0)
		print("CSR_SIE  value : 0x%lx\n", read_csr(CSR_SIE));
	else if (strcmp(*argv, "105") == 0)
		print("CSR_STVEC value : 0x%lx\n", read_csr(CSR_STVEC));
	else if (strcmp(*argv, "106") == 0)
		print("CSR_SCOUNTEREN value : 0x%lx\n",
		      read_csr(CSR_SCOUNTEREN));
	else if (strcmp(*argv, "141") == 0)
		print("CSR_SEPC  value : 0x%lx\n", read_csr(CSR_SEPC));
	else if (strcmp(*argv, "142") == 0)
		print("CSR_SCAUSE  value : 0x%lx\n", read_csr(CSR_SCAUSE));
	else if (strcmp(*argv, "144") == 0)
		print("CSR_SIP  value : 0x%lx\n", read_csr(CSR_SIP));
#endif

//      start = atoi(argv[1]);
	//start = (start >> 2) << 2;
//      len = atoi(argv[2]);

//      for(i = 0; i < len; i++)
//              print("0x%x : 0x%lx\n", start+i, read_csr(start +i));

	print("\n");

	return 0;
}

static const struct command cmd_read_csr = {
	.cmd = "read_csr",
	.handler = cmd_read_csr_handler,
	.priv = NULL,
};

int cmd_read_csr_init()
{
	register_command(&cmd_read_csr);

	return 0;
}

APP_COMMAND_REGISTER(read_csr, cmd_read_csr_init);
