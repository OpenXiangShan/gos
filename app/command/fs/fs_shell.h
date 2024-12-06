#ifndef __FS_SHELL_H__
#define __FS_SHELL_H__

#include "fs.h"

#define FS_SHELL_EXIT 0x1234

struct fs_command {
	char name[64];
	int (*command)(struct dir_entry ** dir, char *path,
		       int argc, char *argv[]);
};

void enter_fs_shell(struct dir_entry **dir, char *path_name, int id,
		    int (*exec_command)(struct dir_entry ** dir,
					char *path_name, char *command));

#endif
