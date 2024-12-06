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


// SPDX-License-Identifier: GPL-2.0
/*
 *  ext4.h
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef __EXT4_FS__
#define __EXT4_FS__

#include "asm/type.h"

#define EXT4_GROUP0_PADDING 1024
#define EXT4_N_BLOCKS 15
#define EXT4_NAME_LEN 255

#define EXT4_REGULAR_FILE 0x1
#define EXT4_DIR          0x2

/*
 * Inode flags
 */
#define	EXT4_SECRM_FL			0x00000001	/* Secure deletion */
#define	EXT4_UNRM_FL			0x00000002	/* Undelete */
#define	EXT4_COMPR_FL			0x00000004	/* Compress file */
#define EXT4_SYNC_FL			0x00000008	/* Synchronous updates */
#define EXT4_IMMUTABLE_FL		0x00000010	/* Immutable file */
#define EXT4_APPEND_FL			0x00000020	/* writes to file may only append */
#define EXT4_NODUMP_FL			0x00000040	/* do not dump file */
#define EXT4_NOATIME_FL			0x00000080	/* do not update atime */
/* Reserved for compression usage... */
#define EXT4_DIRTY_FL			0x00000100
#define EXT4_COMPRBLK_FL		0x00000200	/* One or more compressed clusters */
#define EXT4_NOCOMPR_FL			0x00000400	/* Don't compress */
	/* nb: was previously EXT2_ECOMPR_FL */
#define EXT4_ENCRYPT_FL			0x00000800	/* encrypted file */
/* End compression flags --- maybe not all used */
#define EXT4_INDEX_FL			0x00001000	/* hash-indexed directory */
#define EXT4_IMAGIC_FL			0x00002000	/* AFS directory */
#define EXT4_JOURNAL_DATA_FL		0x00004000	/* file data should be journaled */
#define EXT4_NOTAIL_FL			0x00008000	/* file tail should not be merged */
#define EXT4_DIRSYNC_FL			0x00010000	/* dirsync behaviour (directories only) */
#define EXT4_TOPDIR_FL			0x00020000	/* Top of directory hierarchies */
#define EXT4_HUGE_FILE_FL               0x00040000	/* Set to each huge file */
#define EXT4_EXTENTS_FL			0x00080000	/* Inode uses extents */
#define EXT4_VERITY_FL			0x00100000	/* Verity protected inode */
#define EXT4_EA_INODE_FL	        0x00200000	/* Inode used for large EA */
/* 0x00400000 was formerly EXT4_EOFBLOCKS_FL */

#define EXT4_DAX_FL			0x02000000	/* Inode is DAX */

#define EXT4_INLINE_DATA_FL		0x10000000	/* Inode has inline data. */
#define EXT4_PROJINHERIT_FL		0x20000000	/* Create with parents projid */
#define EXT4_CASEFOLD_FL		0x40000000	/* Casefolded directory */
#define EXT4_RESERVED_FL		0x80000000	/* reserved for ext4 lib */

struct ext4_extent {
	u32 ee_block;		/* first logical block extent covers */
	u16 ee_len;		/* number of blocks covered by extent */
	u16 ee_start_hi;	/* high 16 bits of physical block */
	u32 ee_start_lo;	/* low 32 bits of physical block */
};

/*
 * This is index on-disk structure.
 * It's used at all the levels except the bottom.
 */
struct ext4_extent_idx {
	u32 ei_block;		/* index covers logical blocks from 'block' */
	u32 ei_leaf_lo;		/* pointer to the physical block of the next *
				 * level. leaf or next index could be there */
	u16 ei_leaf_hi;		/* high 16 bits of physical block */
	u16 ei_unused;
};

/*
 * Each block (leaves and indexes), even inode-stored has header.
 */
struct ext4_extent_header {
	u16 eh_magic;		/* probably will support different formats */
	u16 eh_entries;		/* number of valid entries */
	u16 eh_max;		/* capacity of store in entries */
	u16 eh_depth;		/* has tree real underlying blocks? */
	u32 eh_generation;	/* generation of the tree */
};

struct ext4_dir_entry_2 {
	u32 inode;
	u16 rec_len;
	u8 name_len;
	u8 file_type;
	char name[EXT4_NAME_LEN];
};

struct ext4_inode {
	u16 i_mode;		/* File mode */
	u16 i_uid;		/* Low 16 bits of Owner Uid */
	u32 i_size_lo;		/* Size in bytes */
	u32 i_atime;		/* Access time */
	u32 i_ctime;		/* Inode Change time */
	u32 i_mtime;		/* Modification time */
	u32 i_dtime;		/* Deletion Time */
	u16 i_gid;		/* Low 16 bits of Group Id */
	u16 i_links_count;	/* Links count */
	u32 i_blocks_lo;	/* Blocks count */
	u32 i_flags;		/* File flags */
	union {
		struct {
			u32 l_i_version;
		} linux1;
		struct {
			u32 h_i_translator;
		} hurd1;
		struct {
			u32 m_i_reserved1;
		} masix1;
	} osd1;			/* OS dependent 1 */
	u32 i_block[EXT4_N_BLOCKS];	/* Pointers to blocks */
	u32 i_generation;	/* File version (for NFS) */
	u32 i_file_acl_lo;	/* File ACL */
	u32 i_size_high;
	u32 i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			u16 l_i_blocks_high;	/* were l_i_reserved1 */
			u16 l_i_file_acl_high;
			u16 l_i_uid_high;	/* these 2 fields */
			u16 l_i_gid_high;	/* were reserved2[0] */
			u16 l_i_checksum_lo;	/* crc32c(uuid+inum+inode) LE */
			u16 l_i_reserved;
		} linux2;
		struct {
			u16 h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			u16 h_i_mode_high;
			u16 h_i_uid_high;
			u16 h_i_gid_high;
			u32 h_i_author;
		} hurd2;
		struct {
			u16 h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			u16 m_i_file_acl_high;
			u32 m_i_reserved2[2];
		} masix2;
	} osd2;			/* OS dependent 2 */
	u16 i_extra_isize;
	u16 i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	u32 i_ctime_extra;	/* extra Change time      (nsec << 2 | epoch) */
	u32 i_mtime_extra;	/* extra Modification time(nsec << 2 | epoch) */
	u32 i_atime_extra;	/* extra Access time      (nsec << 2 | epoch) */
	u32 i_crtime;		/* File Creation time */
	u32 i_crtime_extra;	/* extra FileCreationtime (nsec << 2 | epoch) */
	u32 i_version_hi;	/* high 32 bits for 64-bit version */
	u32 i_projid;		/* Project ID */
};

struct ext4_super_block {
	u32 s_inodes_count;
	u32 s_blocks_count_lo;
	u32 s_r_blocks_count_lo;
	u32 s_free_blocks_count_lo;
	u32 s_free_inodes_count;
	u32 s_first_data_block;
	u32 s_log_block_size;
	u32 s_log_cluster_size;
	u32 s_blocks_per_group;
	u32 s_clusters_per_group;
	u32 s_inodes_per_group;
	u32 s_mtime;
	u32 s_wtime;
	u16 s_mnt_count;
	u16 s_max_mnt_count;
	u16 s_magic;
	u16 s_state;
	u16 s_errors;
	u16 s_minor_rev_level;
	u32 s_lastcheck;
	u32 s_checkinterval;
	u32 s_creator_os;
	u32 s_rev_level;
	u16 s_def_resuid;
	u16 s_def_resgid;
	u32 s_first_ino;
	u16 s_inode_size;
	u16 s_block_group_nr;
	u32 s_feature_compat;
	u32 s_feature_incompat;
	u32 s_feature_ro_compat;
	u8 s_uuid[16];
	char s_volume_name[16];
	char s_last_mounted[64];
	u32 s_algorithm_usage_bitmap;
	u8 s_prealloc_blocks;
	u8 s_prealloc_dir_blocks;
	u16 s_reserved_gdt_blocks;
	u8 s_journal_uuid[16];
	u32 s_journal_inum;
	u32 s_journal_dev;
	u32 s_last_orphan;
	u32 s_hash_seed[4];
	u8 s_def_hash_version;
	u8 s_jnl_backup_type;
	u16 s_desc_size;
	u32 s_default_mount_opts;
	u32 s_first_meta_bg;
	u32 s_mkfs_time;
	u32 s_jnl_blocks[17];
	u32 s_blocks_count_hi;
	u32 s_r_blocks_count_hi;
	u32 s_free_blocks_count_hi;
	u16 s_min_extra_isize;
	u16 s_want_extra_isize;
	u32 s_flags;
	u16 s_raid_stride;
	u16 s_mmp_update_interval;
	u64 s_mmp_block;
	u32 s_raid_stripe_width;
	u8 s_log_groups_per_flex;
	u8 s_checksum_type;
	u8 s_encryption_level;
	u8 s_reserved_pad;
	u64 s_kbytes_written;
	u32 s_snapshot_inum;
	u32 s_snapshot_id;
	u64 s_snapshot_r_blocks_count;
	u32 s_snapshot_list;
	u32 s_error_count;
	u32 s_first_error_time;
	u32 s_first_error_ino;
	u64 s_first_error_block;
	u8 s_first_error_func[32];
	u32 s_first_error_line;
	u32 s_last_error_time;
	u32 s_last_error_ino;
	u32 s_last_error_line;
	u64 s_last_error_block;
	u8 s_last_error_func[32];
	u8 s_mount_opts[64];
	u32 s_usr_quota_inum;
	u32 s_grp_quota_inum;
	u32 s_overhead_clusters;
	u32 s_backup_bgs[2];
	u8 s_encrypt_algos[4];
	u8 s_encrypt_pw_salt[16];
	u32 s_lpf_ino;
	u32 s_prj_quota_inum;
	u32 s_checksum_seed;
	u8 s_wtime_hi;
	u8 s_mtime_hi;
	u8 s_mkfs_time_hi;
	u8 s_lastcheck_hi;
	u8 s_first_error_time_hi;
	u8 s_last_error_time_hi;
	u8 s_first_error_errcode;
	u8 s_last_error_errcode;
	u16 s_encoding;
	u16 s_encoding_flags;
	u32 s_orphan_file_inum;
	u32 s_reserved[94];
	u32 s_checksum;
};

struct ext4_group_desc {
	u32 bg_block_bitmap_lo;	/* Blocks bitmap block */
	u32 bg_inode_bitmap_lo;	/* Inodes bitmap block */
	u32 bg_inode_table_lo;	/* Inodes table block */
	u16 bg_free_blocks_count_lo;	/* Free blocks count */
	u16 bg_free_inodes_count_lo;	/* Free inodes count */
	u16 bg_used_dirs_count_lo;	/* Directories count */
	u16 bg_flags;		/* EXT4_BG_flags (INODE_UNINIT, etc) */
	u32 bg_exclude_bitmap_lo;	/* Exclude bitmap for snapshots */
	u16 bg_block_bitmap_csum_lo;	/* crc32c(s_uuid+grp_num+bbitmap) LE */
	u16 bg_inode_bitmap_csum_lo;	/* crc32c(s_uuid+grp_num+ibitmap) LE */
	u16 bg_itable_unused_lo;	/* Unused inodes count */
	u16 bg_checksum;	/* crc16(sb_uuid+group+desc) */
	u32 bg_block_bitmap_hi;	/* Blocks bitmap block MSB */
	u32 bg_inode_bitmap_hi;	/* Inodes bitmap block MSB */
	u32 bg_inode_table_hi;	/* Inodes table block MSB */
	u16 bg_free_blocks_count_hi;	/* Free blocks count MSB */
	u16 bg_free_inodes_count_hi;	/* Free inodes count MSB */
	u16 bg_used_dirs_count_hi;	/* Directories count MSB */
	u16 bg_itable_unused_hi;	/* Unused inodes count MSB */
	u32 bg_exclude_bitmap_hi;	/* Exclude bitmap block MSB */
	u16 bg_block_bitmap_csum_hi;	/* crc32c(s_uuid+grp_num+bbitmap) BE */
	u16 bg_inode_bitmap_csum_hi;	/* crc32c(s_uuid+grp_num+ibitmap) BE */
	u32 bg_reserved;
};

struct ext4_bg_info {
	struct list_head list;
	int id;
	int block_bitmap_at;
	unsigned int block_bitmap_csum;
	int inode_bitmap_at;
	unsigned int inode_bitmap_csum;
	int inode_table_at;
	int inode_table_size;
	int free_blocks;
	int free_inodes;
	int unused_inodes;
};

struct ext4_priv {
	struct ext4_super_block *sb;
	struct ext4_group_desc *group_desc;
	struct list_head block_groups;
};

#endif
