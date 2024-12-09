#include "asm/type.h"
#include "fs.h"
#include "mm.h"
#include "string.h"
#include "list.h"
#include "print.h"
#include "gos.h"

static LIST_HEAD(registered_fs_type);
static LIST_HEAD(fs_root_nodes);
static unsigned long fs_node_bitmap = 0;

static int alloc_fs_node_id(void)
{
	unsigned long bp = fs_node_bitmap;
	int pos = 0;

	while (bp & 0x01) {
		if (pos == 64)
			return -1;
		bp = bp >> 1;
		pos++;
	}

	fs_node_bitmap |= (1UL) << pos;

	return pos;
}

static struct fs_type *get_fs_type(u8 type)
{
	struct fs_type *fs;

	list_for_each_entry(fs, &registered_fs_type, list) {
		if (fs->type == type)
			return fs;
	}

	return NULL;
}

static int unflatten_root_dir(struct fs_node *node, struct fs_type *fs)
{
	struct dir_entry *dentry;

	dentry = (struct dir_entry *)mm_alloc(sizeof(struct dir_entry));
	if (!dentry) {
		print("fs: unflatten root dir fail, Out of memory\n");
		return -1;
	}

	strcpy(dentry->name, "/");
	INIT_LIST_HEAD(&dentry->child_entry);
	INIT_LIST_HEAD(&dentry->file_entry);

	if (fs->ops->parse_dir) {
		if (fs->ops->parse_dir(node, dentry->name)) {
			mm_free((void *)dentry, sizeof(struct dir_entry));
			return -1;
		}
	}

	return 0;
}

int load_file(struct file_entry *entry,
	      int (*parse_block_content)(unsigned long start, unsigned int len))
{
	struct fs_type *fs;
	struct fs_node *node;

	node = entry->node;
	if (!node)
		return -1;

	fs = node->fs;
	if (!fs)
		return -1;

	if (!fs->ops->load_file_content)
		return -1;

	return fs->ops->load_file_content(entry, parse_block_content);
}

void ls_all_fs_node(void)
{
	struct fs_node *entry;
	struct fs_type *fs;

	list_for_each_entry(entry, &fs_root_nodes, list) {
		print("fs%d:\n", entry->id);
		print("ID: %d\n", entry->id);
		fs = entry->fs;
		if (fs && fs->ops && fs->ops->print_fs_info)
			fs->ops->print_fs_info(entry);
		print("\n");
	}
}

struct dir_entry *enter_fs(int id)
{
	struct fs_node *entry;

	list_for_each_entry(entry, &fs_root_nodes, list) {
		if (entry->id == id)
			return entry->root_entry;
	}

	return NULL;
}

int load_fs(u8 type, void *blk)
{
	struct fs_type *fs;
	struct fs_node *node;

	fs = get_fs_type(type);
	if (!fs)
		return -1;

	node = (struct fs_node *)mm_alloc(sizeof(struct fs_node));
	if (!node) {
		print("fs: load fs fail.. Out of memory\n");
		return -1;
	}
	memset((char *)node, 0, sizeof(struct fs_node));

	if (fs->ops->parse_meta)
		if (fs->ops->parse_meta(node, blk))
			return -1;

	node->id = alloc_fs_node_id();
	node->fs = fs;

	list_add_tail(&node->list, &fs_root_nodes);

	unflatten_root_dir(node, fs);

	return 0;
}

int register_fs_type(struct fs_type *new)
{
	struct fs_type *fs;

	list_for_each_entry(fs, &registered_fs_type, list) {
		if (new->type == fs->type) {
			print
			    ("fs: warning -- fs type:%d is already registered\n",
			     fs->type);
			return -1;
		}
	}

	list_add_tail(&new->list, &registered_fs_type);

	return 0;
}

#ifdef CONFIG_ENABLE_INIT_FS
void mount_init_fs(void)
{
	extern const char init_fs[];
	print("mount %s as initfs, addr: 0x%lx\n", CONFIG_INIT_FS, init_fs);
	load_fs(FS_TYPE_EXT4, (void *)init_fs);
}
#endif

void fs_init(void)
{
	extern struct fs_init_entry FS_INIT_TABLE, FS_INIT_TABLE_END;
	struct fs_init_entry *entry;
	struct fs_init_entry *from = &FS_INIT_TABLE;
	struct fs_init_entry *to = &FS_INIT_TABLE_END;
	int nr = to - from;

	for (entry = from; nr; entry++, nr--) {
		if (entry->init)
			entry->init();
	}
}
