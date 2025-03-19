#include "asm/type.h"
#include "print.h"
#include "fs.h"
#include "string.h"
#include "mm.h"
#include "ext4.h"
#include "math.h"

static int __parse_inode_entry(struct fs_node *node, struct dir_entry *parent,
			       struct ext4_dir_entry_2 *entry);

static struct ext4_bg_info *get_block_group(struct fs_node *node, int id)
{
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_bg_info *entry;

	list_for_each_entry(entry, &ext4->block_groups, list) {
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

static void *get_block_addr(struct fs_node *node, int block_num)
{
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	int block_size = pow(2, 10 + sb->s_log_block_size);

	return (void *)(node->base + block_size * block_num);
}

static struct ext4_inode *get_ext4_inode(struct fs_node *node, unsigned int no)
{
	struct ext4_inode *inode;
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	struct ext4_bg_info *bg = get_block_group(node, 0);
	int block_size = pow(2, 10 + sb->s_log_block_size);

	if (!bg)
		return NULL;

	inode =
	    (struct ext4_inode *)(node->base + bg->inode_table_at * block_size +
				  sb->s_inode_size * (no - 1));

	return inode;
}

static int fill_ext4_fs_info(struct fs_node *node)
{
	int i;
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	struct ext4_group_desc *desc = ext4->group_desc, *ptr;
	struct ext4_bg_info *bg;
	int desc_count = sb->s_blocks_count_lo / sb->s_blocks_per_group + 1;

	for (i = 0; i < desc_count; i++) {
		bg = (struct ext4_bg_info *)
		    mm_alloc(sizeof(struct ext4_bg_info));
		if (!bg) {
			print
			    ("ext4: warning -- fill_ext4_fs_info fail, Out of memory\n");
			return -1;
		}

		ptr = &desc[i];
		bg->id = i;
		bg->block_bitmap_at =
		    ptr->bg_block_bitmap_lo +
		    ((unsigned long)ptr->bg_block_bitmap_hi << 32);
		bg->block_bitmap_csum =
		    ptr->bg_block_bitmap_csum_lo +
		    ((unsigned int)ptr->bg_block_bitmap_csum_hi << 16);
		bg->inode_bitmap_at =
		    ptr->bg_inode_bitmap_lo +
		    ((unsigned long)ptr->bg_inode_bitmap_hi << 32);
		bg->inode_bitmap_csum =
		    ptr->bg_inode_bitmap_csum_lo +
		    ((unsigned int)ptr->bg_inode_bitmap_csum_hi << 16);
		bg->inode_table_at =
		    ptr->bg_inode_table_lo +
		    ((unsigned long)ptr->bg_inode_table_hi << 32);
		bg->inode_table_size =
		    sb->s_inode_size * sb->s_inodes_per_group / pow(2, 10 + sb->s_log_block_size);
		bg->free_blocks =
		    ptr->bg_free_blocks_count_lo +
		    ((unsigned long)ptr->bg_free_blocks_count_hi << 32);
		bg->free_inodes =
		    ptr->bg_free_inodes_count_lo +
		    ((unsigned long)ptr->bg_free_inodes_count_hi << 32);
		bg->unused_inodes =
		    ptr->bg_itable_unused_lo +
		    ((unsigned long)ptr->bg_itable_unused_hi << 32);

		list_add_tail(&bg->list, &ext4->block_groups);
	}

	return 0;
}

static void print_ext4_group_desc(struct fs_node *node, int which)
{
	struct ext4_bg_info *bg;

	bg = get_block_group(node, which);
	if (!bg)
		return;

	print("=============== ext4 group desc Group%d ===============\n",
	      which);
	print("Block bitmap at %d, csum 0x%04x%04x\n", bg->block_bitmap_at,
	      bg->block_bitmap_csum);
	print("Inode bitmap at %d, csum 0x%04x%04x\n", bg->inode_bitmap_at,
	      bg->inode_bitmap_csum);
	print("Inode table at %d-%d\n", bg->inode_table_at,
	      bg->inode_table_at + bg->inode_table_size - 1);
	print("%d free blocks, %d free inodes, %d unused inodes\n",
	      bg->free_blocks, bg->free_inodes, bg->unused_inodes);
}

static void print_ext4_super_block(struct fs_node *node)
{
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	char uuid[37];

	if (!sb)
		return;

	print("=============== ext4 super block ===============\n");
	char_array_to_uuid((const char *)sb->s_uuid, uuid);
	print("Filesystem UUID:         %s\n", uuid);
	print("Filesystem magic number: 0x%x\n", sb->s_magic);
	if (sb->s_creator_os == 0)
		print("Filesystem os:           Linux\n");
	else if (sb->s_creator_os == 1)
		print("Filesystem os:           Hurd\n");
	else if (sb->s_creator_os == 2)
		print("Filesystem os:           Masix\n");
	else if (sb->s_creator_os == 3)
		print("Filesystem os:           FreeBSD\n");
	else if (sb->s_creator_os == 4)
		print("Filesystem os:           Lites\n");
	print("Inode count:             %d\n", sb->s_inodes_count);
	print("Block count:             %d\n", sb->s_blocks_count_lo);
	print("Reserved block count:    %d\n", sb->s_r_blocks_count_lo);
	print("Free blocks:             %d\n", sb->s_free_blocks_count_lo);
	print("Free Inodes:             %d\n", sb->s_free_inodes_count);
	print("First block:             %d\n", sb->s_first_data_block);
	print("Block size:              %d\n",
	      pow(2, 10 + sb->s_log_block_size));
	print("Group descriptor size:   %d\n", sb->s_desc_size);
	print("Reserved GDT group:      %d\n", sb->s_reserved_gdt_blocks);
	print("Blocks per group:        %d\n", sb->s_blocks_per_group);
	print("Inodes per group:        %d\n", sb->s_inodes_per_group);
	print("First inode:             %d\n", sb->s_first_ino);
	print("Inode size:              %d\n", sb->s_inode_size);
}

static int ext4_fs_parse_meta(struct fs_node *node, void *blk)
{
	struct ext4_priv *ext4_priv;
	struct ext4_super_block *sb;
	struct ext4_group_desc *gp_desc;
	char *ptr = (char *)blk;

	node->base = blk;

	ext4_priv = (struct ext4_priv *)mm_alloc(sizeof(struct ext4_priv));
	if (!ext4_priv) {
		print("ext4: params meta fail. Out of memory\n");
		goto err;
	}

	sb = (struct ext4_super_block *)
	    mm_alloc(sizeof(struct ext4_super_block));
	if (!sb) {
		print("ext4: params meta fail. Out of memory\n");
		goto err2;
	}

	gp_desc =
	    (struct ext4_group_desc *)mm_alloc(sizeof(struct ext4_group_desc));
	if (!gp_desc) {
		print("ext4: params meta fail. Out of memory\n");
		goto err3;
	}

	ptr += EXT4_GROUP0_PADDING;
	memcpy((char *)sb, ptr, sizeof(struct ext4_super_block));
	ext4_priv->sb = sb;

	ptr += pow(2, 10 + sb->s_log_block_size) - EXT4_GROUP0_PADDING;
	memcpy((char *)gp_desc, ptr, sizeof(struct ext4_group_desc));
	ext4_priv->group_desc = gp_desc;

	node->priv_data = (void *)ext4_priv;

	INIT_LIST_HEAD(&ext4_priv->block_groups);

	fill_ext4_fs_info(node);

	return 0;

err3:
	mm_free((void *)gp_desc, sizeof(struct ext4_group_desc));
err2:
	mm_free((void *)ext4_priv, sizeof(struct ext4_priv));
err:
	return -1;
}

static int
__parse_ext4_extent_file(struct ext4_extent_header *eh,
			 int (*parse_file_entry)(unsigned long start,
						 unsigned int len,
						 void *priv),
			 struct fs_node *node, void *priv)
{
	struct ext4_extent_idx *idx;
	struct ext4_extent *ex;
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	int block_size = pow(2, 10 + sb->s_log_block_size);
	int i;

	if (eh->eh_depth == 0) {
		unsigned long start_block;
		int block_count;

		ex = (struct ext4_extent *)(eh + 1);
		for (i = 0; i < eh->eh_entries; i++) {
			start_block = ex[i].ee_start_lo |
			    (((unsigned long)ex[i].ee_start_hi) << 32);
			block_count = ex[i].ee_len;

			if (parse_file_entry
			    ((unsigned long)get_block_addr(node, start_block),
			     block_count * block_size, priv))
				return -1;
		}

		return 0;
	}

	idx = (struct ext4_extent_idx *)(eh + 1);
	for (i = 0; i < eh->eh_entries; i++) {
		unsigned long leaf_block;
		struct ext4_extent_header *h;

		leaf_block = idx[i].ei_leaf_lo |
		    (((unsigned long)idx[i].ei_leaf_hi) << 32);
		h = (struct ext4_extent_header *)get_block_addr(node,
								leaf_block);

		__parse_ext4_extent_file(h, parse_file_entry, node, priv);
	}

	return 0;
}

static int
__parse_ext4_extent_dir(struct ext4_extent_header *eh,
			int (*parse_dir_entry)(struct fs_node * node,
					       unsigned int start,
					       unsigned int count,
					       struct dir_entry *parent),
			struct fs_node *node,
			struct dir_entry *parent)
{
	struct ext4_extent_idx *idx;
	struct ext4_extent *ex;
	int i;

	if (eh->eh_depth == 0) {
		unsigned long start_block;
		int block_count;

		ex = (struct ext4_extent *)(eh + 1);
		for (i = 0; i < eh->eh_entries; i++) {
			start_block = ex[i].ee_start_lo |
			    (((unsigned long)ex[i].ee_start_hi) << 32);
			block_count = ex[i].ee_len;

			if (parse_dir_entry
			    (node, start_block, block_count, parent))
				return -1;
		}

		return 0;
	}

	idx = (struct ext4_extent_idx *)(eh + 1);
	for (i = 0; i < eh->eh_entries; i++) {
		unsigned long leaf_block;
		struct ext4_extent_header *h;

		leaf_block = idx[i].ei_leaf_lo |
		    (((unsigned long)idx[i].ei_leaf_hi) << 32);
		h = (struct ext4_extent_header *)get_block_addr(node,
								leaf_block);

		__parse_ext4_extent_dir(h, parse_dir_entry, node, parent);
	}

	return 0;
}

static int
ext4_parse_extent_inode_file(struct ext4_inode *dir,
			     int (*parse_file_entry)(unsigned long start,
						     unsigned int len,
						     void *priv),
			     struct fs_node *node, void *priv)
{
	struct ext4_extent_header *eh =
	    (struct ext4_extent_header *)dir->i_block;

	if (eh->eh_magic != 0xf30a) {
		print("%s -- invalid magic!!\n", __FUNCTION__);
		return -1;
	}

	return __parse_ext4_extent_file(eh, parse_file_entry, node, priv);
}

static unsigned int
ext4_parse_extent_inode_dir(struct ext4_inode *dir,
			    int (*parse_dir_entry)(struct fs_node *node,
						   unsigned int start,
						   unsigned int count,
						   struct dir_entry *parent),
			    struct fs_node *node,
			    struct dir_entry *parent)
{
	struct ext4_extent_header *eh =
	    (struct ext4_extent_header *)dir->i_block;

	if (eh->eh_magic != 0xf30a) {
		print("%s -- invalid magic!!\n", __FUNCTION__);
		return -1;
	}

	return __parse_ext4_extent_dir(eh, parse_dir_entry, node, parent);
}

static int parse_dentry(struct fs_node *node, unsigned int start,
			unsigned int count, struct dir_entry *parent)
{
	struct ext4_priv *ext4 = (struct ext4_priv *)node->priv_data;
	struct ext4_super_block *sb = ext4->sb;
	void *addr;
	int n = count;
	char name[64];

	addr = get_block_addr(node, start);

	while (n--) {
		int remain = pow(2, 10 + sb->s_log_block_size);
		struct ext4_dir_entry_2 *entry;

		while (remain > 0) {
			entry = (struct ext4_dir_entry_2 *)addr;
			if (entry->rec_len == 0)
				break;

			memcpy(name, entry->name, entry->name_len);
			name[entry->name_len] = 0;

			if (__parse_inode_entry(node, parent, entry))
				return -1;

			remain -= entry->rec_len;
			addr += entry->rec_len;
		}
	}

	return 0;
}

static int __parse_inode_entry(struct fs_node *node, struct dir_entry *parent,
			       struct ext4_dir_entry_2 *entry)
{
	struct file_entry *file_e;
	struct dir_entry *dir_e;

	if (entry->file_type == EXT4_REGULAR_FILE) {
		file_e =
		    (struct file_entry *)mm_alloc(sizeof(struct file_entry));
		if (!file_e)
			return -1;

		memcpy(file_e->name, entry->name, entry->name_len);
		file_e->name[entry->name_len] = 0;
		file_e->data = get_ext4_inode(node, entry->inode);
		file_e->node = node;

		list_add_tail(&file_e->list, &parent->file_entry);
	} else if (entry->file_type == EXT4_DIR) {
		struct ext4_inode *dir;
		char name[64];

		memcpy(name, entry->name, entry->name_len);
		name[entry->name_len] = 0;

		if ((!strcmp(name, ".")) || (!strcmp(name, "..")))
			return 0;

		dir_e = (struct dir_entry *)mm_alloc(sizeof(struct dir_entry));
		if (!dir_e)
			return -1;

		INIT_LIST_HEAD(&dir_e->child_entry);
		INIT_LIST_HEAD(&dir_e->file_entry);
		name[entry->name_len] = 0;
		strcpy(dir_e->name, name);
		dir_e->parent = parent;

		list_add_tail(&dir_e->list, &parent->child_entry);

		dir = get_ext4_inode(node, entry->inode);
		if (!dir)
			return -1;

		if (dir->i_flags & EXT4_EXTENTS_FL) {
			ext4_parse_extent_inode_dir(dir, parse_dentry, node,
						    dir_e);
		} else
			print("%s %d Warning!! ToDo... i_flags:0x%lx\n",
			      __FUNCTION__, __LINE__, dir->i_flags);
	} else
		return 0;

	return 0;
}

static int parse_root_dentry(struct fs_node *node, unsigned int start,
			     unsigned int count, struct dir_entry *parent)
{
	struct dir_entry *root;

	if (!node->root_entry) {
		root = mm_alloc(sizeof(struct dir_entry));
		if (!root) {
			print("fs: %s fail, Out of memory\n");
			return -1;
		}
		INIT_LIST_HEAD(&root->child_entry);
		INIT_LIST_HEAD(&root->file_entry);
		strcpy(root->name, "/");

		node->root_entry = root;
	}

	return parse_dentry(node, start, count, root);
}

static int ext4_fs_parse_dir(struct fs_node *node, char *dir_name)
{
	struct ext4_inode *root;

	root = get_ext4_inode(node, 2);
	if (!root)
		return -1;

	if (root->i_flags & EXT4_EXTENTS_FL) {
		ext4_parse_extent_inode_dir(root, parse_root_dentry, node,
					    NULL);
	} else {
		print("ext4: warning!! ToDo... iflags:%d\n", root->i_flags);
		return -1;
	}

	return 0;
}

static int ext4_fs_load_file(struct file_entry *entry,
			     int (*parse_block_content)(unsigned long start,
							unsigned int len,
							void *priv),
			     void *priv)
{
	struct ext4_inode *inode =(struct ext4_inode *)entry->data;
	struct fs_node *node = entry->node;

	if (!node)
		return -1;

	if (!inode)
		return -1;

	if (inode->i_flags & EXT4_EXTENTS_FL) {
		ext4_parse_extent_inode_file(inode, parse_block_content, node, priv);
	} else {
		print("ext4: warning!! ToDo... iflags:%d\n", inode->i_flags);
		return -1;
	}

	return 0;
}

static void ext4_fs_print_info(struct fs_node *node)
{
	print_ext4_super_block(node);
	print_ext4_group_desc(node, 0);
}

struct fs_type_ops ext4_fs_type_ops = {
	.print_fs_info = ext4_fs_print_info,
	.parse_meta = ext4_fs_parse_meta,
	.parse_dir = ext4_fs_parse_dir,
	.load_file_content = ext4_fs_load_file,
};

static struct fs_type ext4_fs_type = {
	.type = FS_TYPE_EXT4,
	.ops = &ext4_fs_type_ops,
};

static void ext4_fs_init(void)
{
	register_fs_type(&ext4_fs_type);
}

FS_REGISTER(ext4, ext4_fs_init);
