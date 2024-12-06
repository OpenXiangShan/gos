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
#include "mm.h"
#include "print.h"
#include "string.h"
#include "device.h"
#include "gos.h"
#include "fs.h"
#include "fs_shell.h"

void enter_fs_shell(struct dir_entry **dir, char *path_name, int id,
		    int (*exec_command)(struct dir_entry ** cur_dir,
					char *path_name, char *command))
{
	int fd, i;
	int ret;
	char buf[64];
	char *tmp;
	char *shell_command = NULL;
	int input_count = 0;
	char path[128];

	print("Enter exit to return to Shell...\n");
	sprintf(path, "FS%d:%s >> ", id, path_name);
	print("%s", path);
	fd = open("UART0");
	if (fd < 0) {
		print("open %s failed...\n", "UART0");
		return;
	}

	shell_command = (char *)mm_alloc(PAGE_SIZE);
	if (!shell_command) {
		print("alloc shell command buffer failed...\n");
		return;
	}

	tmp = shell_command;
	while (1) {
#if CONFIG_USE_UART_POLL
		ret = read(fd, buf, 0, 64, NONBLOCK);
#else
		ret = read(fd, buf, 0, 64, BLOCK);
#endif
		for (i = 0; i < ret; i++) {
			if (buf[i] < 0 || buf[i] == 255)
				continue;
			else if (buf[i] == 13 /* Enter */ ) {
				*tmp++ = 0;
				input_count = 0;
				print("\n");
				if (exec_command(dir, path_name, shell_command)
				    == FS_SHELL_EXIT)
					return;

				sprintf(path, "FS%d:%s/ >> ", id, path_name);
				print("%s", path);
				memset(shell_command, 0, PAGE_SIZE);
				tmp = shell_command;
			} else if (buf[i] == 127
				   || buf[i] == 8 /* Backspace */ ) {
				if (input_count == 0)
					continue;
				input_count--;
				print_backspace(1);
				tmp--;
				*tmp = 0;
			} else {
				*tmp++ = buf[i];
				input_count++;
				print("%c", buf[i]);
			}
		}
	}

	return;
}
