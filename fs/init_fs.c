#include "asm/type.h"
#include "fs.h"
#include "string.h"
#include "list.h"
#include "print.h"
#include "gos.h"

static struct dir_entry *init_fs_root = NULL;

struct dir_entry *get_init_fs_root(void)
{
	return init_fs_root;
}

void mount_init_fs(void)
{
	extern const char init_fs[];
	int id;

	print("mount %s as initfs, addr: 0x%lx\n", CONFIG_INIT_FS, init_fs);

	id = load_fs(FS_TYPE_EXT4, (void *)init_fs);
	if (id == -1)
		return;

	init_fs_root = enter_fs(id);
}
