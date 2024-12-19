#include "command.h"
#include <print.h>
#include "type.h"

int zacas_test(void);

static rtests tests_zacas[] = {
	{"zacas test",zacas_test},
};
static int test_zacas(void)
{
	for(int i=0;i < (sizeof(tests_zacas)/sizeof(rtests)); i++){
		if(!tests_zacas[i].name)
			break;
		if(!tests_zacas[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zacas[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zacas[i].name);
		}
	}
	return 0;
}
static int cmd_zacas_handler(int argc, char *argv[], void *priv)
{
	print("zacas testing ......\n");
	test_zacas();
	print("zacas test...... end\n");
	return 0;
}

static const struct command cmd_zacas_test = {
	.cmd = "zacas_test",
	.handler = cmd_zacas_handler,
	.priv = NULL,
};

int user_cmd_zacas_test_init()
{
	register_command(&cmd_zacas_test);

	return 0;
}

APP_COMMAND_REGISTER(zacas_test, user_cmd_zacas_test_init);
