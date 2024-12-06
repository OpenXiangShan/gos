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

#include "asm/type.h"
#include "print.h"
#include "device.h"
#include "string.h"
#include "list.h"
#include "../../command.h"
#include "fs.h"
#include "fs_shell.h"

static int fs_ls(struct dir_entry **dir, char *path, int argc, char *argv[])
{
	struct file_entry *file_e;
	struct dir_entry *dir_e;
	struct dir_entry *_dir = *dir;
	int count = 0;

	list_for_each_entry(dir_e, &_dir->child_entry, list) {
		print("%24s", dir_e->name);
		if (count++ == 128 / 24) {
			count = 0;
			print("\n");
			continue;
		}
		print(" ");
	}

	list_for_each_entry(file_e, &_dir->file_entry, list) {
		print("%24s", file_e->name);
		if (count++ == 128 / 24) {
			count = 0;
			print("\n");
			continue;
		}
		print(" ");
	}

	print("\n");

	return 0;
}

static int fs_cd(struct dir_entry **dir, char *path, int argc, char *argv[])
{
	struct dir_entry *dir_e;
	struct dir_entry *_dir = *dir;
	int found = 0;

	if (argc == 0)
		return 0;

	if (!strcmp("..", argv[0])) {
		if (!_dir->parent)
			return 0;
		*dir = _dir->parent;
		strcpy(path, _dir->parent->name);
		found = 1;
	} else {
		list_for_each_entry(dir_e, &_dir->child_entry, list) {
			if (!strcmp(dir_e->name, argv[0])) {
				*dir = dir_e;
				strcpy(path, dir_e->name);
				found = 1;
			}
		}
	}

	if (!found) {
		print("No Such file or director...\n");
		return -1;
	}

	return 0;
}

static int fs_file_parse_content(unsigned long start, unsigned int len)
{
	char *tmp = (char *)start;

	while (len--) {
		if (*tmp == 0)
			return 0;
		print("%c", *tmp);
		tmp++;
	}
	return 0;
}

static int fs_cat(struct dir_entry **dir, char *path, int argc, char *argv[])
{
	char *file_name;
	struct dir_entry *_dir = *dir;
	struct file_entry *file_e;

	if (argc == 0)
		return -1;

	file_name = argv[0];

	list_for_each_entry(file_e, &_dir->file_entry, list) {
		if (!strcmp(file_e->name, file_name)) {
			load_file(file_e, fs_file_parse_content);
			return 0;
		}
	}

	return 0;
}

static int fs_exit(struct dir_entry **dir, char *path, int argc, char *argv[])
{
	return FS_SHELL_EXIT;
}

struct fs_command command_fs[] = {
	{ "ls", fs_ls },
	{ "cd", fs_cd },
	{ "cat", fs_cat },
	{ "exit", fs_exit },
};

#define FS_COMMAND_COUNT (sizeof(command_fs) / sizeof(command_fs[0]))

static void fs_command_parse_params(char *input, char *command, int *argc,
				    char **argv, char *params)
{
	char *tmp = input;
	char *cmd_tmp, *arg_tmp, **argv_tmp, *arg_start;
	int _argc = 0;
	char *cmd;
	char *args;
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
			if (_argc == 16)
				goto next_arg;
			_argc++;
			*arg_tmp++ = 0;
			*argv_tmp++ = arg_start;
		} else
			is_arg = 1;
next_arg:
		while (*tmp == ' ')
			tmp++;
	}

	*argc = _argc;

}

static int fs_exec_command(struct dir_entry **dir, char *path_name, char *input)
{
	int i;
	int argc = 0;
	char *argv[16] = { 0 };
	char params[128] = { 0 };
	char command[64];

	fs_command_parse_params(input, command, &argc, argv, params);

	for (i = 0; i < FS_COMMAND_COUNT; i++) {
		if (!strcmp(command_fs[i].name, command)) {
			if (command_fs[i].command)
				return command_fs[i].command(dir, path_name,
							     argc, argv);
		}
	}

	return 0;
}

static int cmd_fs_handler(int argc, char *argv[], void *priv)
{
	int id;
	struct dir_entry *root;
	char name[64];

	if (argc == 0)
		return -1;

	id = atoi(argv[0]);

	root = enter_fs(id);
	if (!root) {
		print("enter fs%d fail\n", id);
		return -1;
	}

	strcpy(name, "/");

	enter_fs_shell(&root, name, id, fs_exec_command);

	return 0;
}

static const struct command cmd_fs = {
	.cmd = "fs",
	.handler = cmd_fs_handler,
	.priv = NULL,
};

static int cmd_lsfs_handler(int argc, char *argv[], void *priv)
{

	ls_all_fs_node();

	return 0;
}

static const struct command cmd_lsfs = {
	.cmd = "lsfs",
	.handler = cmd_lsfs_handler,
	.priv = NULL,
};

int cmd_fs_init()
{
	register_command(&cmd_lsfs);
	register_command(&cmd_fs);

	return 0;
}

APP_COMMAND_REGISTER(fs, cmd_fs_init);
