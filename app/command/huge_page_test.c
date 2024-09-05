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
#include "vmap.h"
#include "asm/pgtable.h"
#include "string.h"
#include "mm.h"

static int cmd_huge_page_test_handler(int argc, char *argv[], void *priv)
{
	int size = 2 * PAGE_2M_SIZE;
	char *addr_1g;
	char *addr1_2m, *addr2_2m, *addr3_2m;
	char *addr1_4k, *addr2_4k, *addr3_4k;

	print("----------------------- mapping 4k -----------------------\n");
	addr1_4k = (char *)vmem_alloc(size, NULL);
	addr1_4k[0] = 40;
	addr1_4k[size - 1] = 41;

	addr2_4k = (char *)vmem_alloc(size, NULL);
	addr2_4k[0] = 50;
	addr2_4k[size - 1] = 51;

	addr3_4k = (char *)vmem_alloc(size, NULL);
	addr3_4k[0] = 60;
	addr3_4k[size - 1] = 61;

	print("4k mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr1_4k, 0, addr1_4k[0], size - 1, addr1_4k[size - 1]);
	print("4k mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr2_4k, 0, addr2_4k[0], size - 1, addr2_4k[size - 1]);
	print("4k mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr3_4k, 0, addr3_4k[0], size - 1, addr3_4k[size - 1]);

	vmem_free(addr1_4k, size);
	vmem_free(addr2_4k, size);
	vmem_free(addr3_4k, size);

	print("----------------------- mapping 2m -----------------------\n");
	addr1_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr1_2m[0] = 10;
	addr1_2m[size - 1] = 11;
	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr1_2m, 0, addr1_2m[0], size - 1, addr1_2m[size - 1]);
	print("free addr:0x%lx\n", addr1_2m);
	vmem_free_huge(addr1_2m, size, PAGE_2M_SIZE);

	addr2_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr2_2m[0] = 20;
	addr2_2m[size - 1] = 21;
	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr2_2m, 0, addr2_2m[0], size - 1, addr2_2m[size - 1]);
	print("free addr:0x%lx\n", addr2_2m);
	vmem_free_huge(addr2_2m, size, PAGE_2M_SIZE);

	addr3_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr3_2m[0] = 30;
	addr3_2m[size - 1] = 31;
	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr3_2m, 0, addr3_2m[0], size - 1, addr3_2m[size - 1]);
	print("free addr:0x%lx\n", addr3_2m);
	vmem_free_huge(addr3_2m, size, PAGE_2M_SIZE);

	addr1_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr1_2m[0] = 10;
	addr1_2m[size - 1] = 11;

	addr2_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr2_2m[0] = 20;
	addr2_2m[size - 1] = 21;

	addr3_2m = (char *)vmem_alloc_huge(size, PAGE_2M_SIZE, NULL);
	addr3_2m[0] = 30;
	addr3_2m[size - 1] = 31;

	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr1_2m, 0, addr1_2m[0], size - 1, addr1_2m[size - 1]);
	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr2_2m, 0, addr2_2m[0], size - 1, addr2_2m[size - 1]);
	print("2M mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr3_2m, 0, addr3_2m[0], size - 1, addr3_2m[size - 1]);

	print("----------------------- mapping 1g -----------------------\n");
	size = PAGE_1G_SIZE;
	addr_1g = (char *)vmem_alloc_huge(size, PAGE_1G_SIZE, NULL);
	addr_1g[0] = 66;
	addr_1g[size - 1] = 88;
	print("1G mapping -- addr: 0x%lx —— addr[%d]:%d addr[%d]:%d\n",
	      addr_1g, 0, addr_1g[0], size - 1, addr_1g[size - 1]);

	print("free addr:0x%lx\n", addr1_2m);
	vmem_free_huge(addr1_2m, PAGE_2M_SIZE, PAGE_2M_SIZE);
	print("free addr:0x%lx\n", addr2_2m);
	vmem_free_huge(addr2_2m, PAGE_2M_SIZE, PAGE_2M_SIZE);
	print("free addr:0x%lx\n", addr3_2m);
	vmem_free_huge(addr3_2m, PAGE_2M_SIZE, PAGE_2M_SIZE);
	print("free addr:0x%lx\n", addr_1g);
	vmem_free_huge(addr_1g, PAGE_1G_SIZE, PAGE_1G_SIZE);

	print("free unmap test...\n");
	addr_1g[0] = 1;

	print("TEST PASS\n");

	return 0;
}

static const struct command cmd_huge_page_test = {
	.cmd = "huge_page_test",
	.handler = cmd_huge_page_test_handler,
	.priv = NULL,
};

int huge_page_test_init()
{
	return register_command(&cmd_huge_page_test);
}

APP_COMMAND_REGISTER(huge_page_test, huge_page_test_init);
