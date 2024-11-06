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
#include "task.h"
#include "mm.h"
#include "string.h"

struct cmd_loader_param {
	char cmd[64];
	int argc;
	char argv[16][64];
};

static void Usage()
{
	print("load cpuid=[cpuid] [command] [params1] [params2] ...\n");
}

static int cmd_loader_start(void *data)
{
	struct cmd_loader_param *lp = (struct cmd_loader_param *)data;
	char *cmd = lp->cmd;
	int argc = lp->argc;
	char *argv[16] = { 0 };

	for (int i = 0; i < argc; i++)
		argv[i] = lp->argv[i];

	if (do_command(cmd, argc, argv, NULL))
		print("Unknown command: %d\n", cmd);

	mm_free((void *)lp, sizeof(struct cmd_loader_param));

	return 0;
}

static int cmd_loader_handler(int argc, char *argv[], void *priv)
{
	int hart, i;
	char command[64];
	struct cmd_loader_param *lp;

	if (argc < 2) {
		print("Invalid input params...\n");
		Usage();
		return -1;
	}

	if (!strncmp(argv[0], "cpu=", sizeof("cpu=") - 1)) {
		char *tmp = argv[0];
		tmp += sizeof("cpu=") - 1;
		hart = atoi(tmp);
	} else {
		print("Invalid params...\n");
		Usage();
		return -1;
	}

	lp = (struct cmd_loader_param *)
	    mm_alloc(sizeof(struct cmd_loader_param));

	strcpy(lp->cmd, argv[1]);
	lp->argc = argc - 2;
	for (i = 0; i < lp->argc; i++) {
		strcpy(lp->argv[i], argv[i + 2]);
	}

	create_task(command, cmd_loader_start, (void *)lp, hart, NULL, 0, NULL);

	return 0;
}

static const struct command cmd_loader = {
	.cmd = "load",
	.handler = cmd_loader_handler,
	.priv = NULL,
};

int cmd_loader_init()
{
	register_command(&cmd_loader);

	return 0;
}

APP_COMMAND_REGISTER(loader, cmd_loader_init);
