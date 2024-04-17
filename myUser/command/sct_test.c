#include <print.h>
#include "command.h"
#include "print.h"
#include <../asm/csr.h>

#define  U_mode     (1 << 8)
#define  en_SPIE    (1 << 5)

void scounter_test(void)
{
	unsigned long value;
	printf("=============== 1 \n");
	value = read_csr(CSR_CYCLE);
	printf("CSR_status value : 0x%lx\n", value);
	printf("=============== 2 \n");
	value = read_csr(CSR_TIME);
	printf("CSR_status value : 0x%lx\n", value);
	printf("=============== 3 \n");
	//value = read_csr(CSR_INSTRET);
	//printf("CSR_status value : 0x%lx\n",  value);
#if 0
	value = read_csr(CSR_SSTATUS);
	printf("CSR_status value : 0x%lx\n", value);

	printf("CSR_scounteren: 0x%lx \n", read_csr(CSR_SCOUNTEREN));
	csr_write(CSR_SCOUNTEREN, 0xffff);
	printf("CSR_scounteren: 0x%lx \n", read_csr(CSR_SCOUNTEREN));
	printf("CSR_Satp: 0x%lx \n", read_csr(CSR_SATP));

	value &= ~U_mode;
	printf(" == value :0x%lx \n", value);
	csr_write(CSR_SSTATUS, value);
	value = read_csr(CSR_SSTATUS);
	printf("CSR_status value : 0x%lx\n", value);
	//csr_write(CSR_SEPC, &kyo_test);
#endif
	//csr_read(CSR_SSCOUNTOVF);
	//csr_read(CSR_CYCLE);
	//csr_write(CSR_SEPC, &scounter_check);
	//printf("CSR_Sepc: 0x%lx \n", read_csr(CSR_SEPC));

}

static int cmd_scounter_test_handler(int argc, char *argv[], void *priv)
{

	scounter_test();

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
