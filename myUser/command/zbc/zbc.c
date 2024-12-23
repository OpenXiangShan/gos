#include "command.h"
#include <print.h>
#include "type.h"

int clmul_test(void);
int clmulh_test(void);
int clmulr_test(void);

static rtests tests_zbc[] = {
	{"clmul test",clmul_test},
	{"clmulh test",clmulh_test},
	{"clmulr test",clmulr_test},
};
static int test_zbc(void)
{
	int r = 0;

	for(int i=0; i < sizeof(tests_zbc)/sizeof(tests_zbc[0]); i++){
		if(!tests_zbc[i].name)
			break;
		if(!tests_zbc[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zbc[i].name);
		}else{
			print("ERROR: %s fail \n", tests_zbc[i].name);
			r++;
		}
	}

	return r;
}
static int cmd_zbc_handler(int argc, char *argv[], void *priv)
{
	int r;

	print("zbc testing ......\n");
	r = test_zbc();
	print("zbc test...... end\n");
	if (r)
		return TEST_FAIL;
	else
		return TEST_PASS;
}

static const struct command cmd_zbc_test = {
	.cmd = "zbc_test",
	.handler = cmd_zbc_handler,
	.priv = NULL,
};

int user_cmd_zbc_test_init()
{
	register_command(&cmd_zbc_test);

	return 0;
}

APP_COMMAND_REGISTER(zbc_test, user_cmd_zbc_test_init);
