#include "command.h"
#include <print.h>
#include "type.h"

int bclr_test(void);
int bclri_test(void);
int bext_test(void);
int bexti_test(void);
int binv_test(void);
int binvi_test(void);
int bset_test(void);
int bseti_test(void);

rtests tests_zbs[] = {
	{"bclr test",bclr_test},
	{"bclri test",bclri_test},
	{"bext test",bext_test},
	{"bexti test",bexti_test},
	{"binv test",binv_test},
	{"binvi test",binvi_test},
	{"bset test",bset_test},
	{"bseti test",bseti_test},
};
static int test_zbs(void)
{
	for(int i=0;; i++){
		if(!tests_zbs[i].name)
			break;
		if(!tests_zbs[i].fp()){
			printf("%s pass \n", tests_zbs[i].name);
		}else{
			printf("ERROR: %s fail \n", tests_zbs[i].name);
		}
	}
	return 0;
}
static int cmd_zbs_handler(int argc, char *argv[], void *priv)
{
	printf("zbs testing ......\n");
	test_zbs();
	printf("zbs test...... end\n");
	return 0;
}

static const struct command cmd_zbs_test = {
	.cmd = "zbs_test",
	.handler = cmd_zbs_handler,
	.priv = NULL,
};

int user_cmd_zbs_test_init()
{
	register_command(&cmd_zbs_test);

	return 0;
}

APP_COMMAND_REGISTER(zbs_test, user_cmd_zbs_test_init);
