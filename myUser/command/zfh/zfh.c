#include "command.h"
#include <print.h>
#include "type.h"

int fadd_test(void);
int fclass_test(void);
int fcmp_test(void);
int fcvt_test(void);
int fcvt_w_test(void);
int fdiv_test(void);
int fmadd_test(void);
int fmin_test(void);
int ldst_test(void);
int move_test(void);
int recoding_test(void);

static rtests tests_zfh[] = {
	{"fadd test",fadd_test},
	{"fclass test",fclass_test},
	{"fcmp test",fcmp_test},
	{"fcvt_w test",fcvt_w_test},
	{"fdiv test",fdiv_test},
	{"fmadd test",fmadd_test},
	{"fmin test",fmin_test},
	{"ldst test",ldst_test},
	{"move test",move_test},
	{"recoding test",recoding_test},
};
static int test_zfh(void)
{
	for(int i=0;i < (sizeof(tests_zfh)/sizeof(rtests)); i++){
		if(!tests_zfh[i].name)
			break;
		if(!tests_zfh[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zfh[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zfh[i].name);
		}
	}
	return 0;
}
static int cmd_zfh_handler(int argc, char *argv[], void *priv)
{
	print("zfh testing ......\n");
	test_zfh();
	print("zfh test...... end\n");
	return 0;
}

static const struct command cmd_zfh_test = {
	.cmd = "zfh_test",
	.handler = cmd_zfh_handler,
	.priv = NULL,
};

int user_cmd_zfh_test_init()
{
	register_command(&cmd_zfh_test);

	return 0;
}

APP_COMMAND_REGISTER(zfh_test, user_cmd_zfh_test_init);
