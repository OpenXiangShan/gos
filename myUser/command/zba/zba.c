#include "command.h"
#include <print.h>
#include "type.h"

int sh1add_test(void);
int add_uw_test(void);
int sh1add_uw_test(void);
int sh2add_test(void);
int sh2add_uw_test(void);
int sh3add_test(void);
int sh3add_uw_test(void);
int slli_uw_test(void);

static rtests tests_zba[] = {
	{"sh1add test",sh1add_test},
	{"add_uw test",add_uw_test},
	{"sh1add_uw test",sh1add_uw_test},
	{"sh2add test",sh2add_test},
	{"sh2add_uw",sh2add_uw_test},
	{"sh3add_test",sh3add_test},
	{"sh3add_uw test",sh3add_uw_test},
	{"slli_uw test",slli_uw_test},
};
static int test_zba(void)
{
	for(int i=0;i < (sizeof(tests_zba)/sizeof(rtests)); i++){
		if(!tests_zba[i].name)
			break;
		if(!tests_zba[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zba[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zba[i].name);
		}
	}
	return 0;
}
static int cmd_zba_handler(int argc, char *argv[], void *priv)
{
	print("zba testing ......\n");
	test_zba();
	print("zba test...... end\n");
	return 0;
}

static const struct command cmd_zba_test = {
	.cmd = "zba_test",
	.handler = cmd_zba_handler,
	.priv = NULL,
};

int user_cmd_zba_test_init()
{
	register_command(&cmd_zba_test);

	return 0;
}

APP_COMMAND_REGISTER(zba_test, user_cmd_zba_test_init);
