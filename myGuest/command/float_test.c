/*
 * Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "command.h"
#include "print.h"
#include "asm/type.h"

static int set_f(unsigned long value)
{
	print("value == %0x \n", value);
	asm volatile("fmv.d.x f0, %0" : : "r"(value));
	print("value2 == %0x \n", value);
	asm volatile("fmv.d.x f1, %0" : : "r"(value));
	asm volatile("fmv.d.x f2, %0" : : "r"(value));
	asm volatile("fmv.d.x f3, %0" : : "r"(value));
	asm volatile("fmv.d.x f4, %0" : : "r"(value));
	asm volatile("fmv.d.x f5, %0" : : "r"(value));
	asm volatile("fmv.d.x f6, %0" : : "r"(value));
	asm volatile("fmv.d.x f7, %0" : : "r"(value));
	asm volatile("fmv.d.x f8, %0" : : "r"(value));
	asm volatile("fmv.d.x f9, %0" : : "r"(value));
	asm volatile("fmv.d.x f10, %0" : : "r"(value));
	asm volatile("fmv.d.x f11, %0" : : "r"(value));
	asm volatile("fmv.d.x f12, %0" : : "r"(value));
	asm volatile("fmv.d.x f13, %0" : : "r"(value));
	asm volatile("fmv.d.x f14, %0" : : "r"(value));
	asm volatile("fmv.d.x f15, %0" : : "r"(value));
	asm volatile("fmv.d.x f16, %0" : : "r"(value));
	asm volatile("fmv.d.x f17, %0" : : "r"(value));
	asm volatile("fmv.d.x f18, %0" : : "r"(value));
	asm volatile("fmv.d.x f19, %0" : : "r"(value));
	asm volatile("fmv.d.x f20, %0" : : "r"(value));
	asm volatile("fmv.d.x f21, %0" : : "r"(value));
	asm volatile("fmv.d.x f22, %0" : : "r"(value));
	asm volatile("fmv.d.x f23, %0" : : "r"(value));
	asm volatile("fmv.d.x f24, %0" : : "r"(value));
	asm volatile("fmv.d.x f25, %0" : : "r"(value));
	asm volatile("fmv.d.x f26, %0" : : "r"(value));
	asm volatile("fmv.d.x f27, %0" : : "r"(value));
	asm volatile("fmv.d.x f28, %0" : : "r"(value));
	asm volatile("fmv.d.x f29, %0" : : "r"(value));
	asm volatile("fmv.d.x f30, %0" : : "r"(value));
	asm volatile("fmv.d.x f31, %0" : : "r"(value));
	return 0;
}

static int read_f(int pid, unsigned long value)
{
	unsigned long  f_data[32];
	for(int i=0; i < 32; i++)
	{
		switch (i) {
			case 0: asm volatile("fmv.x.d %0, f0" : "=r"(f_data[i])); break;
			case 1: asm volatile("fmv.x.d %0, f1" : "=r"(f_data[i])); break;
			case 2: asm volatile("fmv.x.d %0, f2" : "=r"(f_data[i])); break;
			case 3: asm volatile("fmv.x.d %0, f3" : "=r"(f_data[i])); break;
			case 4: asm volatile("fmv.x.d %0, f4" : "=r"(f_data[i])); break;
			case 5: asm volatile("fmv.x.d %0, f5" : "=r"(f_data[i])); break;
			case 6: asm volatile("fmv.x.d %0, f6" : "=r"(f_data[i])); break;
			case 7: asm volatile("fmv.x.d %0, f7" : "=r"(f_data[i])); break;
			case 8: asm volatile("fmv.x.d %0, f8" : "=r"(f_data[i])); break;
			case 9: asm volatile("fmv.x.d %0, f9" : "=r"(f_data[i])); break;
			case 10: asm volatile("fmv.x.d %0, f10" : "=r"(f_data[i])); break;
			case 11: asm volatile("fmv.x.d %0, f11" : "=r"(f_data[i])); break;
			case 12: asm volatile("fmv.x.d %0, f12" : "=r"(f_data[i])); break;
			case 13: asm volatile("fmv.x.d %0, f13" : "=r"(f_data[i])); break;
			case 14: asm volatile("fmv.x.d %0, f14" : "=r"(f_data[i])); break;
			case 15: asm volatile("fmv.x.d %0, f15" : "=r"(f_data[i])); break;
			case 16: asm volatile("fmv.x.d %0, f16" : "=r"(f_data[i])); break;
			case 17: asm volatile("fmv.x.d %0, f17" : "=r"(f_data[i])); break;
			case 18: asm volatile("fmv.x.d %0, f18" : "=r"(f_data[i])); break;
			case 19: asm volatile("fmv.x.d %0, f19" : "=r"(f_data[i])); break;
			case 20: asm volatile("fmv.x.d %0, f20" : "=r"(f_data[i])); break;
			case 21: asm volatile("fmv.x.d %0, f21" : "=r"(f_data[i])); break;
			case 22: asm volatile("fmv.x.d %0, f22" : "=r"(f_data[i])); break;
			case 23: asm volatile("fmv.x.d %0, f23" : "=r"(f_data[i])); break;
			case 24: asm volatile("fmv.x.d %0, f24" : "=r"(f_data[i])); break;
			case 25: asm volatile("fmv.x.d %0, f25" : "=r"(f_data[i])); break;
			case 26: asm volatile("fmv.x.d %0, f26" : "=r"(f_data[i])); break;
			case 27: asm volatile("fmv.x.d %0, f27" : "=r"(f_data[i])); break;
			case 28: asm volatile("fmv.x.d %0, f28" : "=r"(f_data[i])); break;
			case 29: asm volatile("fmv.x.d %0, f29" : "=r"(f_data[i])); break;
			case 30: asm volatile("fmv.x.d %0, f30" : "=r"(f_data[i])); break;
			case 31: asm volatile("fmv.x.d %0, f31" : "=r"(f_data[i])); break;
		}
		if(f_data[i] != value)
		{
			print("ERROR:pid=%d f%d data is error data= %X \n",pid,i,f_data[i]);
			return 1;
		}
	}
	return 0;
}
static int float_start1(void* data)
{
	int ret;
	print("Floating3 testing \n");
	set_f(0x1234);
	print("Floating4 testing \n");
	while(1){
		ret = read_f(0x1, 0x1234);
		if(ret !=0){
			print("float_test1 is ERROR\n");
		}else{
			print("float_test1 is ok \n");
		}
	}
	return 0;
}

static int cmd_float_handler(int argc, char *argv[], void *priv)
{
	print("Floating testing \n");
	print("Floating2 testing \n");
	float_start1(NULL);
	return 0;
}

static const struct command cmd_float_test = {
	.cmd = "float_test",
	.handler = cmd_float_handler,
	.priv = NULL,
};

int cmd_float_test_init()
{
	register_command(&cmd_float_test);

	return 0;
}

APP_COMMAND_REGISTER(float_test, cmd_float_test_init);
