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

#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "tiny_mm.h"
#include "string.h"

static int cmd_tiny_test_handler(int argc, char *argv[], void *priv)
{
	void *addr1;
	void *addr2;
	void *addr3;
	void *addr4;
	void *addr5;
	void *addr6;
	void *addr7;
	void *addr8;
	void *addr9;

	for (int i = 0; i < 9; i++) {
		int size = 1UL << (i + 3);
		print("/*------------- tiny_%d -------------*/\n", size);
		print("/* tiny_alloc and tiny_free */\n");
		addr1 = tiny_alloc(size);
		addr2 = tiny_alloc(size);
		addr3 = tiny_alloc(size);
		addr4 = tiny_alloc(size);
		addr5 = tiny_alloc(size);
		addr6 = tiny_alloc(size);
		addr7 = tiny_alloc(size);
		addr8 = tiny_alloc(size);
		addr9 = tiny_alloc(size);

		print("addr: 0x%lx\n", addr1);
		strcpy((char *)addr1, "Hello1");
		print("%s\n", addr1);

		print("addr: 0x%lx\n", addr2);
		strcpy((char *)addr2, "Hello2");
		print("%s\n", addr2);

		print("addr: 0x%lx\n", addr3);
		strcpy((char *)addr3, "Hello3");
		print("%s\n", addr3);

		print("addr: 0x%lx\n", addr4);
		strcpy((char *)addr4, "Hello4");
		print("%s\n", addr4);

		print("addr: 0x%lx\n", addr5);
		strcpy((char *)addr5, "Hello5");
		print("%s\n", addr5);

		print("addr: 0x%lx\n", addr6);
		strcpy((char *)addr6, "Hello6");
		print("%s\n", addr6);

		print("addr: 0x%lx\n", addr7);
		strcpy((char *)addr7, "Hello7");
		print("%s\n", addr7);

		print("addr: 0x%lx\n", addr8);
		strcpy((char *)addr8, "Hello8");
		print("%s\n", addr8);

		print("addr: 0x%lx\n", addr9);
		strcpy((char *)addr9, "Hello9");
		print("%s\n", addr9);

		tiny_free(addr1);
		tiny_free(addr2);
		tiny_free(addr3);
		tiny_free(addr4);
		tiny_free(addr5);
		tiny_free(addr6);
		tiny_free(addr7);
		tiny_free(addr8);
		tiny_free(addr9);

		print("/* alloc and free interleave */\n");
		addr1 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr1);
		strcpy((char *)addr1, "Hello1");
		print("%s\n", addr1);
		tiny_free(addr1);

		addr2 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr2);
		strcpy((char *)addr2, "Hello2");
		print("%s\n", addr2);
		tiny_free(addr2);

		addr3 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr3);
		strcpy((char *)addr3, "Hello3");
		print("%s\n", addr3);
		tiny_free(addr3);

		addr4 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr4);
		strcpy((char *)addr4, "Hello4");
		print("%s\n", addr4);
		tiny_free(addr4);

		addr5 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr5);
		strcpy((char *)addr5, "Hello5");
		print("%s\n", addr5);
		tiny_free(addr5);

		addr6 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr6);
		strcpy((char *)addr6, "Hello6");
		print("%s\n", addr6);
		tiny_free(addr6);

		addr7 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr7);
		strcpy((char *)addr7, "Hello7");
		print("%s\n", addr7);
		tiny_free(addr7);

		addr8 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr8);
		strcpy((char *)addr8, "Hello8");
		print("%s\n", addr8);
		tiny_free(addr8);

		addr9 = tiny_alloc(size);
		print("addr: 0x%lx\n", addr9);
		strcpy((char *)addr9, "Hello9");
		print("%s\n", addr9);
		tiny_free(addr9);
	}

	return 0;
}

static const struct command cmd_tiny_test = {
	.cmd = "tiny_test",
	.handler = cmd_tiny_test_handler,
	.priv = NULL,
};

int cmd_tiny_test_init()
{
	register_command(&cmd_tiny_test);

	return 0;
}

APP_COMMAND_REGISTER(tiny_test, cmd_tiny_test_init);
