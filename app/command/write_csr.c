#include <asm/type.h>
#include <print.h>
#include <device.h>
#include <string.h>
#include <asm/mmio.h>
#include "../command.h"
#include <asm/csr.h>

#define csr_write(csr, val)                                     \
({                                                              \
        unsigned long __v = (unsigned long)(val);               \
        __asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0"     \
                              : : "rK" (__v)                    \
                              : "memory");                      \
})

static int cmd_write_csr_handler(int argc, char *argv[], void *priv)
{
	unsigned long start;
	unsigned long value;

	if (argc != 2) {
		print("invalid input param.\n");
		return -1;
	}

	if (!is_digit(argv[0]) || !is_digit(argv[1])) {
		print("invalid input param.\n");
		return -1;
	}
	start = atoi(argv[0]);
	value = atoi(argv[1]);
	print("-----> Modfiy csr %d, value:0x%lx \n", start, value);
	switch (start) {
	case 0xc00:
		csr_write(CSR_CYCLE, value);
		print("CSR_CYCLE value : 0x%lx\n", read_csr(CSR_CYCLE));
		break;
	case 0xc01:
		csr_write(CSR_TIME, value);
		print("CSR_TIME value : 0x%lx\n", read_csr(CSR_TIME));
		break;
	case 0xc02:
		csr_write(CSR_INSTRET, value);
		print("CSR_INSTRET value : 0x%lx\n", read_csr(CSR_INSTRET));
		break;
	case 0xc03:
		csr_write(CSR_HPMCOUNTER3, value);
		print("CSR_HPMCOUNTER3 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER3));
		break;
	case 0xc04:
		csr_write(CSR_HPMCOUNTER4, value);
		print("CSR_HPMCOUNTER4 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER4));
		break;
	case 0xc05:
		csr_write(CSR_HPMCOUNTER5, value);
		print("CSR_HPMCOUNTER5 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER5));
		break;
	case 0xc06:
		csr_write(CSR_HPMCOUNTER6, value);
		print("CSR_HPMCOUNTER6 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER6));
		break;
	case 0xc07:
		csr_write(CSR_HPMCOUNTER7, value);
		print("CSR_HPMCOUNTER7 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER7));
		break;
	case 0xc08:
		csr_write(CSR_HPMCOUNTER8, value);
		print("CSR_HPMCOUNTER8 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER8));
		break;
	case 0xc09:
		csr_write(CSR_HPMCOUNTER9, value);
		print("CSR_HPMCOUNTER9 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER9));
		break;
	case 0xc0a:
		csr_write(CSR_HPMCOUNTER10, value);
		print("CSR_HPMCOUNTER10 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER10));
		break;
	case 0xc0b:
		csr_write(CSR_HPMCOUNTER11, value);
		print("CSR_HPMCOUNTER11 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER11));
		break;
	case 0xc0c:
		csr_write(CSR_HPMCOUNTER12, value);
		print("CSR_HPMCOUNTEr12 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER12));
		break;
	case 0xc0d:
		csr_write(CSR_HPMCOUNTER13, value);
		print("CSR_HPMCOUNTER13 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER13));
		break;
	case 0xc0e:
		csr_write(CSR_HPMCOUNTER14, value);
		print("CSR_HPMCOUNTER14 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER14));
		break;
	case 0xc0f:
		csr_write(CSR_HPMCOUNTER15, value);
		print("CSR_HPMCOUNTER15 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER15));
		break;
	case 0xc10:
		csr_write(CSR_HPMCOUNTER16, value);
		print("CSR_HPMCOUNTER16 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER16));
		break;
	case 0xc11:
		csr_write(CSR_HPMCOUNTER17, value);
		print("CSR_HPMCOUNTER17 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER17));
		break;
	case 0xc12:
		csr_write(CSR_HPMCOUNTER18, value);
		print("CSR_HPMCOUNTER18 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER18));
		break;
	case 0xc13:
		csr_write(CSR_HPMCOUNTER19, value);
		print("CSR_HPMCOUNTER19 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER19));
		break;
	case 0xc14:
		csr_write(CSR_HPMCOUNTER20, value);
		print("CSR_HPMCOUNTER20 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER20));
		break;
	case 0xc15:
		csr_write(CSR_HPMCOUNTER21, value);
		print("CSR_HPMCOUNTER21 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER21));
		break;
	case 0xc16:
		csr_write(CSR_HPMCOUNTER22, value);
		print("CSR_HPMCOUNTER22 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER22));
		break;
	case 0xc17:
		csr_write(CSR_HPMCOUNTER23, value);
		print("CSR_HPMCOUNTER23 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER23));
		break;
	case 0xc18:
		csr_write(CSR_HPMCOUNTER24, value);
		print("CSR_HPMCOUNTER24 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER24));
		break;
	case 0xc19:
		csr_write(CSR_HPMCOUNTER25, value);
		print("CSR_HPMCOUNTER25 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER25));
		break;
	case 0xc1a:
		csr_write(CSR_HPMCOUNTER26, value);
		print("CSR_HPMCOUNTER26 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER26));
		break;
	case 0xc1b:
		csr_write(CSR_HPMCOUNTER27, value);
		print("CSR_HPMCOUNTER27 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER27));
		break;
	case 0xc1c:
		csr_write(CSR_HPMCOUNTER28, value);
		print("CSR_HPMCOUNTER28 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER28));
		break;
	case 0xc1d:
		csr_write(CSR_HPMCOUNTER29, value);
		print("CSR_HPMCOUNTER29 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER29));
		break;
	case 0xc1e:
		csr_write(CSR_HPMCOUNTER30, value);
		print("CSR_HPMCOUNTER30 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER30));
		break;
	case 0xc1f:
		csr_write(CSR_HPMCOUNTER31, value);
		print("CSR_HPMCOUNTER31 value : 0x%lx\n",
		      read_csr(CSR_HPMCOUNTER31));
		break;
	case 0xda0:
		csr_write(CSR_SSCOUNTOVF, value);
		print("CSR_SSCOUNTOVF value : 0x%lx\n",
		      read_csr(CSR_SSCOUNTOVF));
		break;
	case 0x100:
		csr_write(CSR_SSTATUS, value);
		print("CSR_SSTATUS value : 0x%lx\n", read_csr(CSR_SSTATUS));
		break;
	case 0x106:
		csr_write(CSR_SCOUNTEREN, value);
		print("CSR_SCOUNTEREN value : 0x%lx\n",
		      read_csr(CSR_SCOUNTEREN));
		break;
	default:
		print("-------> get csr not support, try again \n");
		break;

	}

	print("\n");

	return 0;
}

static const struct command cmd_write_csr = {
	.cmd = "write_csr",
	.handler = cmd_write_csr_handler,
	.priv = NULL,
};

int cmd_write_csr_init()
{
	register_command(&cmd_write_csr);

	return 0;
}

APP_COMMAND_REGISTER(write_csr, cmd_write_csr_init);
