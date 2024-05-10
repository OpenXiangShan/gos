#include <event.h>
#include <print.h>
#include <device.h>
#include <mm.h>
#include <string.h>
#include <asm/type.h>
#include "shell.h"
#include "command.h"

#define ESC_ASCII 27
#define SHELL_DEVICE_NAME "UART0"

int shell_init(void *data)
{
#ifndef NO_SHELL
	int i;
	int left = 5;
	int fd;
	char buf[64];
	char *shell_command = NULL;
	int input_count = 0;
	char *tmp;
	int ret;
	struct cmd_name *cmd;
	int arrow_flag = 0;
#endif
	command_init();

#ifdef NO_SHELL
	test_cmd_auto_run();
#else
	command_history_init();

	shell_command = (char *)mm_alloc(PAGE_SIZE);
	if (!shell_command) {
		print("alloc shell command buffer failed...\n");
		return -1;
	}

	fd = open(SHELL_DEVICE_NAME);
	if (fd < 0) {
		print("open %s failed...\n", SHELL_DEVICE_NAME);
		return -1;
	}

	print("open %s as console...\n", SHELL_DEVICE_NAME);

	wait_for_ms(1000);
	for (; left > 0; left--) {
		print_backspace(sizeof
				("5s left to excute test command, press ESC to enter the shell..."));
		print
		    ("%ds left to excute test command, press ESC to enter the shell...",
		     left);

		if (ESC_ASCII == wait_for_input_timeout(fd, 1000)) {
			print("\n");
			goto run_shell;
		}
	}
	print_backspace(sizeof
			("5s left to excute test command, press ESC to enter the shell..."));
	print("\n");

	test_cmd_auto_run();

run_shell:
	print("Shell >> ");
	tmp = shell_command;
	while (1) {
		ret = read(fd, buf, 0, 64, BLOCK);
		for (i = 0; i < ret; i++) {
			if (buf[i] < 0 || buf[i] == 255)
				continue;
			else if (buf[i] == 13 /* Enter */ ) {
				*tmp++ = 0;
				input_count = 0;
				print("\n");
				exec_command(shell_command);
				set_last_cmd_pos();
				print("Shell >> ");
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
			} else if (buf[i] == 9 /* Tab */ ) {
				print("\n");
				walk_and_print_command();
				print("\n");
				print("Shell >> ");
				tmp = shell_command;
			}
			/* process arrow which are formed by three keys */
			else if (buf[i] == 27 /* arrow */ ) {
				arrow_flag = 1;
			} else if (buf[i] == 91 && arrow_flag == 1) {
				arrow_flag = 2;
			} else if (arrow_flag == 2) {
				arrow_flag = 0;
				if (buf[i] == 65 /* up */ ) {	//use to show last history cmd
					cmd = get_last_cmd_pos();
					if (cmd) {
						while (input_count--)
							print_backspace(1);
						print("%s", cmd->name);
						input_count = strlen(cmd->name);
						tmp = shell_command;
						strcpy(tmp, cmd->name);
						tmp += input_count;
					}
				} else if (buf[i] == 66 /* down */ ) {	// use to show next history cmd
					cmd = get_last_next_cmd_pos();
					if (cmd) {
						while (input_count--)
							print_backspace(1);
						print("%s", cmd->name);
						input_count = strlen(cmd->name);
						tmp = shell_command;
						strcpy(tmp, cmd->name);
						tmp += input_count;
					}
				} else if (buf[i] == 67 /* right */ ) {

				} else if (buf[i] == 68 /* right */ ) {

				}
			}	/* end process arrow */
			else {
				*tmp++ = buf[i];
				input_count++;
				print("%c", buf[i]);
			}
		}
	}

	mm_free(shell_command, PAGE_SIZE);
	shell_command = NULL;
#endif
	return 0;
}
