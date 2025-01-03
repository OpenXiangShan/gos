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

#include <print.h>
#include <mm.h>
#include <string.h>
#include <asm/type.h>
#include "command.h"
#include "vmap.h"
#include "asm/pgtable.h"

static struct cmd_history history;
static struct commands _commands;
static char *test_cmd;

static int command_save_into_history(char *cmd)
{
	struct cmd_name *p_cmds = history.p_cmds;

	if (!p_cmds)
		return -1;

	strcpy(p_cmds[history.tail].name, cmd);

	if (history.tail == history.max_index)
		history.tail = 0;
	else
		history.tail++;

	if (history.tail == history.head) {
		if (history.head == history.max_index)
			history.head = 0;
		else
			history.head++;
	}

	return 0;
}

void test_cmd_auto_run()
{
	char *tmp = test_cmd;
	char command[64] = { 0, };
	char *cmd = command;

	while (*tmp) {
		if (*tmp == '\r' || *tmp == '\n') {
			*cmd = 0;
			print("run test command: %s\n", command);
			exec_command(command);
			cmd = command;
			print("\n");
		} else
			*cmd++ = *tmp;

		tmp++;
	}
}

int do_command(char *cmd, int argc, char *argv[], void *priv)
{
	struct command_info *entry;
	int nr = _commands.avail;

	for_each_command(entry, _commands.p_commands, nr) {
		if (!entry->in_used)
			continue;
		if (!strcmp(entry->command->cmd, cmd)) {
			if (entry->command->handler) {
				entry->command->handler(argc, argv,
							entry->command->priv);
				command_save_into_history(cmd);
			}
			return 0;
		}
	}

	return -1;
}

void walk_and_print_command()
{
	struct command_info *entry;
	int nr = _commands.avail;
	int count = 0;

	for_each_command(entry, _commands.p_commands, nr) {
		if (!entry->in_used)
			continue;
		print("%24s", entry->command->cmd);
		if (count++ == 128 / 24) {
			count = 0;
			print("\n");
			continue;
		}
		print(" ");
	}
	print("\n");
}

void exec_command(char *input)
{
	char *tmp = input;
	char command[128] = { 0 };
	char params[128] = { 0 };
	char *cmd_tmp, *arg_tmp, **argv_tmp, *arg_start;
	struct command_info *entry;
	int nr = _commands.avail;
	int argc = 0;
	char *cmd;
	char *args;
	char *argv[16] = { 0 };
	int is_arg = 0;
	int n_cmd = 0, n_par = 0;

	if (*tmp == 0)
		return;

	cmd = command;
	args = params;

	cmd_tmp = cmd;
	arg_tmp = args;
	argv_tmp = argv;

	while (*tmp == ' ')
		tmp++;

	while (*tmp) {
		arg_start = arg_tmp;
		while (*tmp != ' ' && *tmp != 0) {
			if (is_arg) {
				if (n_par == 128) {
					*(arg_tmp - 1) = 0;
					goto next;
				}
				*arg_tmp++ = *tmp;
				n_par++;
			} else {
				if (n_cmd == 128) {
					*(cmd_tmp - 1) = 0;
					goto next;
				}
				*cmd_tmp++ = *tmp;
				*cmd_tmp = 0;
				n_cmd++;
			}
next:
			tmp++;
		}

		if (is_arg) {
			if (argc == 16)
				goto next_arg;
			argc++;
			*arg_tmp++ = 0;
			*argv_tmp++ = arg_start;
		} else
			is_arg = 1;
next_arg:
		while (*tmp == ' ')
			tmp++;
	}

	for_each_command(entry, _commands.p_commands, nr) {
		if (!entry->in_used)
			continue;

		if (!strncmp(entry->command->cmd, cmd, 128)) {
			if (entry->command->handler) {
				entry->command->handler(argc, argv,
							entry->command->priv);
				command_save_into_history(cmd);
			}
			return;
		}
	}

	print("can not find command: %s\n", cmd);

	return;
}

static void command_setup()
{
	extern unsigned long COMMAND_INIT_TABLE, COMMAND_INIT_TABLE_END;
	int nr =
	    (struct app_command_entry *)&COMMAND_INIT_TABLE_END -
	    (struct app_command_entry *)&COMMAND_INIT_TABLE;
	struct app_command_entry *entry;
	struct app_command_entry *head =
	    (struct app_command_entry *)&COMMAND_INIT_TABLE;

	for (entry = head; nr; entry++, nr--)
		entry->init();
}

void command_history_init()
{
	struct cmd_name *p_cmds;

	history.head = 0;
	history.tail = 0;
	history.last = 0;

	p_cmds = (struct cmd_name *)mm_alloc(PAGE_SIZE);
	if (!p_cmds)
		return;
	memset((char *)p_cmds, 0, PAGE_SIZE);

	history.p_cmds = p_cmds;
	history.max_index = PAGE_SIZE / sizeof(struct cmd_name) - 1;
}

int command_init()
{
	struct command_info *p_commands =
	    (struct command_info *)mm_alloc(PAGE_SIZE);
	if (!p_commands)
		return -1;

	memset((char *)p_commands, 0, PAGE_SIZE);
	_commands.p_commands = p_commands;
	_commands.total = PAGE_SIZE;
	_commands.avail = 0;

	command_setup();

	return 0;
}

int register_command(const struct command *command)
{
	struct command_info *cmd, *new;
	int remain = _commands.total;
	int new_size;

retry:
	cmd = _commands.p_commands;
	remain = _commands.total;
	while (remain > sizeof(struct command_info)) {
		if (!cmd->in_used)
			goto found;
		cmd++;
		remain -= sizeof(struct command_info);
	}

	new_size = _commands.total + PAGE_SIZE;
	new = mm_alloc(new_size);
	if (!new)
		return -1;
	memset((char *)new, 0, new_size);
	memcpy((char *)new, (char *)_commands.p_commands, _commands.total);
	mm_free(_commands.p_commands, _commands.total);
	_commands.p_commands = new;
	_commands.total = new_size;

	goto retry;

found:
	cmd->in_used = 1;
	cmd->command = command;
	_commands.avail++;

	return 0;
}

struct cmd_history *command_get_history()
{
	if (!history.p_cmds)
		return NULL;

	return &history;
}

struct cmd_name *get_last_cmd_pos()
{
	struct cmd_name *cmd;

	if (!history.p_cmds)
		return NULL;

	if (history.head == history.tail)
		return NULL;

	if (history.last == history.head)
		history.last = history.tail - 1;
	else if (history.last == 0)
		history.last = history.max_index;
	else
		history.last--;

	cmd = &history.p_cmds[history.last];

	return cmd;
}

struct cmd_name *get_last_next_cmd_pos()
{
	struct cmd_name *cmd;

	if (!history.p_cmds)
		return NULL;

	if (history.head == history.tail)
		return NULL;

	if (history.last == history.tail)
		history.last = history.head;
	else if (history.last == history.max_index)
		history.last = 0;
	else
		history.last++;

	cmd = &history.p_cmds[history.last];

	return cmd;
}

void set_last_cmd_pos()
{
	if (history.tail == 0)
		history.last = history.max_index;
	else
		history.last = history.tail;
}

void set_test_cmd_ptr(char *boot_option)
{
	extern int mmu_is_on;

	if (mmu_is_on)
		test_cmd = (char *)ioremap((void *)boot_option, PAGE_SIZE, NULL);
	else
		test_cmd = boot_option;
}
