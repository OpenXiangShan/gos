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

#ifndef __USER_COMMAND_H
#define __USER_COMMAND_H

#include "result.h"

#define COMMAND_INIT_TABLE __command_init_table
#define COMMAND_INIT_TABLE_END __command_init_table_end

struct user_run_params {
	char command[64];
	int argc;
	char argv[16][64];
	int busy;
	int userid;
	int cpu;
	int bg;
};

struct command {
	char cmd[64];
	int (*handler)(int argc, char *argv[], void *priv);
	void *priv;
};

struct command_info {
	int in_used;
	const struct command *command;
};

struct commands {
	struct command_info *p_commands;
	int total;
	int avail;
};

struct app_command_entry {
	int (*init)(void);
};

#define for_each_command(entry, commands, n)                                \
	for (entry = (struct command_info *)commands; n > 0; entry++, n--)

#define APP_COMMAND_REGISTER(name, init_fn)                           \
	static const struct app_command_entry __attribute__((used))   \
		__command_entry_##name                                \
		__attribute__((section(".command_init_table"))) = {   \
			.init = init_fn,                              \
		}

void walk_and_print_command(void);
int register_command(const struct command *command);
int command_init(void);
int do_command(struct user_run_params *params);

#endif
