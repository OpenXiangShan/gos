#include "command.h"
#include <print.h>
#include "type.h"

int fli_test(void);
int fminm_test(void);
int fmaxm_test(void);
int fround_test(void);
int fcvtmod_test(void);
int fleq_test(void);
int fltq_test(void);

static rtests tests_zfa[] = {
	{"fli test",fli_test},
	{"fminm test",fminm_test},
	{"fmaxm test",fmaxm_test},
	{"fround test",fmaxm_test},
	{"fleq test",fleq_test},
	{"fltq test",fleq_test},
};
static int test_zfa(void)
{
	for(int i = 0;i < (sizeof(tests_zfa)/sizeof(rtests)); i++){
		if(!tests_zfa[i].name)
			break;
		if(!tests_zfa[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zfa[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zfa[i].name);
		}
	}
	return 0;
}
static int cmd_zfa_handler(int argc, char *argv[], void *priv)
{
	print("zfa testing ......\n");
	test_zfa();
	print("zfa test...... end\n");
	return 0;
}

static const struct command cmd_zfa_test = {
	.cmd = "zfa_test",
	.handler = cmd_zfa_handler,
	.priv = NULL,
};

int user_cmd_zfa_test_init()
{
	register_command(&cmd_zfa_test);

	return 0;
}

APP_COMMAND_REGISTER(zfa_test, user_cmd_zfa_test_init);
