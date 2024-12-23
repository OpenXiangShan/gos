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
	int r = 0;

	for(int i=0; i < sizeof(tests_zba)/sizeof(tests_zba[0]); i++){
		if(!tests_zba[i].name)
			break;
		if(!tests_zba[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zba[i].name);
		}else{
			print("ERROR: %s fail \n", tests_zba[i].name);
			r++;
		}
	}
	return r;
}
static int cmd_zba_handler(int argc, char *argv[], void *priv)
{
	int r;

	print("zba testing ......\n");
	r = test_zba();
	print("zba test...... end\n");
	if (r)
		return TEST_FAIL;
	else
		return TEST_PASS;
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
