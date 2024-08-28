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

#ifndef _COMMAND_H
#define _COMMAND_H

#define COMMAND_INIT_TABLE __command_init_table
#define COMMAND_INIT_TABLE_END __command_init_table_end

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

struct cmd_name {
	char name[64];
};

struct cmd_history {
	struct cmd_name *p_cmds;
	int head;
	int tail;
	int last;
	int max_index;
};

#define for_each_history_cmd(cmd, p_history, n) \
	for (n = p_history->head, cmd = &p_history->p_cmds[n]; n != p_history->tail; n == p_history->max_index ? (n = 0) : n++, cmd = &p_history->p_cmds[n])

#define for_each_command(entry, commands, n)                                \
	for (entry = (struct command_info *)commands; n > 0; entry++, n--)

#define APP_COMMAND_REGISTER(name, init_fn)                           \
	static const struct app_command_entry __attribute__((used))   \
		__command_entry_##name                                \
		__attribute__((section(".command_init_table"))) = {   \
			.init = init_fn,                              \
		}

void exec_command(char *cmd);
int register_command(const struct command *command);
int command_init(void);
void test_cmd_auto_run(void);
void walk_and_print_command(void);
struct cmd_history *command_get_history(void);
void set_last_cmd_pos(void);
struct cmd_name *get_last_cmd_pos(void);
struct cmd_name *get_last_next_cmd_pos(void);
void command_history_init(void);
int do_command(char *cmd, int argc, char *argv[], void *priv);
void set_test_cmd_ptr(char *boot_option);

#endif
