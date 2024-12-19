#include "command.h"
#include <print.h>
#include "type.h"

int andn_test(void);
int clz_test(void);
int clzw_test(void);
int cpop_test(void);
int cpopw_test(void);
int ctz_test(void);
int ctzw_test(void);
int max_test(void);
int maxu_test(void);
int min_test(void);
int minu_test(void);
int orc_b_test(void);
int orn_test(void);
int rev8_test(void);
int rol_test(void);
int rolw_test(void);
int ror_test(void);
int rori_test(void);
int roriw_test(void);
int rorw_test(void);
int sext_b_test(void);
int sext_h_test(void);
int xnor_test(void);
int zext_h_test(void);

static rtests tests_zbb[] = {
	{"andn test",andn_test},
	{"clz test",clz_test},
	{"clzw test",clzw_test},
	{"cpop test",cpop_test},
	{"cpopw test",cpopw_test},
	{"ctz test",ctz_test},
	{"ctzw test",ctzw_test},
	{"max test",max_test},
	{"maxu test",maxu_test},
	{"min test",min_test},
	{"minu test",minu_test},
	{"orc_b test",orc_b_test},
	{"orn test",orn_test},
	{"rev8 test",rev8_test},
	{"rol test",rol_test},
	{"rolw test",rolw_test},
	{"ror test",ror_test},
	{"rori test",rori_test},
	{"roriw test",roriw_test},
	{"rorw test",rorw_test},
	{"sext_b test",sext_b_test},
	{"sext_h test",sext_h_test},
	{"xnor test",xnor_test},
	{"zext_h test",zext_h_test},
};
static int test_zbb(void)
{
	for(int i=0;i < (sizeof(tests_zbb)/sizeof(rtests)); i++){
		if(!tests_zbb[i].name)
			break;
		if(!tests_zbb[i].fp()){
			print("%s TEST_SUCCESS \n", tests_zbb[i].name);
		}else{
			print("ERROR: %s TEST FAIL \n", tests_zbb[i].name);
		}
	}
	return 0;
}
static int cmd_zbb_handler(int argc, char *argv[], void *priv)
{
	print("zbb testing ......\n");
	test_zbb();
	print("zbb test...... end\n");
	return 0;
}

static const struct command cmd_zbb_test = {
	.cmd = "zbb_test",
	.handler = cmd_zbb_handler,
	.priv = NULL,
};

int user_cmd_zbb_test_init()
{
	register_command(&cmd_zbb_test);

	return 0;
}

APP_COMMAND_REGISTER(zbb_test, user_cmd_zbb_test_init);
