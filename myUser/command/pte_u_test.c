#include "command.h"
#include "print.h"
#include "type.h"
#include "../asm/pgtable.h"
#include "malloc.h"
#include <string.h>

pgprot_t pgprot =
	__pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
                     _PAGE_DIRTY | _PAGE_USER);
pgprot_t pgprot_no_user =
	__pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE | _PAGE_EXEC |
                     _PAGE_DIRTY);

char *str[]={"----> _PAGE_USER is enalbe", "----> _PAGE_USER is disable \n"};
static int set_pte(pgprot_t pgprot, char *c)
{
	void *va;

	printf("%s \n", (char *)c);
	va = malloc_pte(PAGE_SIZE, pgprot);
	if (!va) {
		printf("%s -- page mapping failed\n", __FUNCTION__);
		return -1;
	}
	strcpy(va, "Hello");
	printf("%s\n", va);
	memset((void *)va, 0, PAGE_SIZE);

	return 0;
}


static int user_cmd_pte_handler(int argc, char *argv[], void *priv)
{
	int ret = 0;

	printf("Hello MyUesr!! argc:%d\n", argc);

	for (int i = 0; i < argc; i++)
		printf("cmd%d: %s\n", i, argv[i]);

	ret = set_pte(pgprot, str[0]);
	if (ret == -1)
		return -1;
	ret = set_pte(pgprot_no_user, str[1]);
	if (ret == -1)
		return -1;

	return 0;
}

static const struct command user_cmd_pte = {
	.cmd = "pte",
	.handler = user_cmd_pte_handler,
	.priv = NULL,
};

int user_cmd_pte_init()
{
	register_command(&user_cmd_pte);

	return 0;
}

APP_COMMAND_REGISTER(pte_u_test, user_cmd_pte_init);
