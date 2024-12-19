#include "command.h"
#include <print.h>
#include "type.h"

int min_fcvt_test(void);
int min_ldst_test(void);

static rtests tests_zfhmin[] = {
	{"min_fcvt test",min_fcvt_test},
	{"min_ldst test",min_ldst_test},
};
static int test_zfhmin(void)
{
	for(int i=0;i < (sizeof(tests_zfhmin)/sizeof(rtests)); i++){
		if(!tests_zfhmin[i].name)
			break;
		if(!tests_zfhmin[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zfhmin[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zfhmin[i].name);
		}
	}
	return 0;
}
static int cmd_zfhmin_handler(int argc, char *argv[], void *priv)
{
	print("zfhmin testing ......\n");
	test_zfhmin();
	print("zfhmin test...... end\n");
	return 0;
}

static const struct command cmd_zfhmin_test = {
	.cmd = "zfhmin_test",
	.handler = cmd_zfhmin_handler,
	.priv = NULL,
};

int user_cmd_zfhmin_test_init()
{
	register_command(&cmd_zfhmin_test);

	return 0;
}

APP_COMMAND_REGISTER(zfh_test, user_cmd_zfhmin_test_init);
