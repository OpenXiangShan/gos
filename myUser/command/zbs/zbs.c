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

static rtests tests_zbs[] = {
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
	int r = 0;

	for(int i=0; i < sizeof(tests_zbs)/sizeof(tests_zbs[0]); i++){
		if(!tests_zbs[i].name)
			break;
		if(!tests_zbs[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zbs[i].name);
		}else{
			print("ERROR: %s fail \n", tests_zbs[i].name);
			r++;
		}
	}
	return r;
}
static int cmd_zbs_handler(int argc, char *argv[], void *priv)
{
	int r;

	print("zbs testing ......\n");
	r = test_zbs();
	print("zbs test...... end\n");
	if (r)
		return TEST_FAIL;
	else
		return TEST_PASS;
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
