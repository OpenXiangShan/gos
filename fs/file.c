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
#include "fs.h"
#include "string.h"
#include "list.h"
#include "print.h"
#include "mm.h"
#include "spinlocks.h"

static DEFINE_SPINLOCK(lock);
static LIST_HEAD(file_list);

static struct dir_entry *get_dir_entry(struct dir_entry *parent,
				       char *filename)
{
	struct dir_entry *dir_e;

	list_for_each_entry(dir_e, &parent->child_entry, list) {
		if (!strcmp(dir_e->name, filename))
			return dir_e;
	}

	return NULL;
}

static struct file_entry *get_file_entry(struct dir_entry *parent,
					 char *filename)
{
	struct file_entry *file_e;

	list_for_each_entry(file_e, &parent->file_entry, list) {
		if (!strcmp(file_e->name, filename))
			return file_e;
	}

	return NULL;
}

int fread(void *handle, void *buf, int count)
{
	struct file *file = (struct file *)handle;
	struct file_buffer *f_buffer;
	int read_count = 0;

	if (!file)
		return 0;

	list_for_each_entry(f_buffer, &file->buffer, list) {
		if ((file->pos >= f_buffer->pos) &&
		    (file->pos < (f_buffer->pos + f_buffer->len))) {
			if ((file->pos + count) < (f_buffer->pos + f_buffer->len)) {
				memcpy((char *)buf,
				       (char *)f_buffer->buffer + (file->pos - f_buffer->pos),
				       count);
				file->pos += count;
				read_count += count;
			}
			else {
				memcpy((char *)buf,
				       (char *)f_buffer->buffer + (file->pos - f_buffer->pos),
				       f_buffer->len - (file->pos - f_buffer->pos));
				file->pos += f_buffer->len - (file->pos - f_buffer->pos);
				read_count += f_buffer->len - (file->pos - f_buffer->pos);
			}
		}
		if (read_count == count)
			goto ret;
	}

ret:
	return read_count;
}

int ftell(void *handle)
{
	struct file *file = (struct file *)handle;

	if (!file)
		return -1;

	return file->pos;
}

int fseek(void *handle, unsigned long offset, int whence)
{
	struct file *file = (struct file *)handle;

	if (!file)
		return -1;

	if (whence == SEEK_SET)
		file->pos = offset;
	else if (whence == SEEK_CUR)
		file->pos += offset;
	else if (whence == SEEK_END)
		file->pos = file->end_pos + offset;
	else
		return -1;

	return 0;
}

int fwrite(void* handle, const void *buf, int count)
{
	struct file *file = (struct file *)handle;

	if (!file)
		return 0;

	return 0;
}

void fclose(void *handle)
{
	struct file *file = (struct file *)handle;

	if (!file)
		return;

	spin_lock(&lock);
	list_del(&file->list);
	spin_unlock(&lock);

	mm_free(file, sizeof(struct file));
}

static void set_file_end_pos(struct file *file)
{
	struct file_buffer *f_buffer;

	list_for_each_entry(f_buffer, &file->buffer, list) {
		file->end_pos += f_buffer->len;
	}
}

static void set_file_buffer_pos(struct file *file)
{
	struct file_buffer *f_buffer;

	list_for_each_entry(f_buffer, &file->buffer, list) {
		struct file_buffer *prev;

		if (f_buffer == list_first_entry(&file->buffer, struct file_buffer, list))
			continue;

		prev = list_entry(f_buffer->list.prev, struct file_buffer, list);
		f_buffer->pos = prev->pos + prev->len;
	}
}

static int file_parse_content(unsigned long start, unsigned int len, void *priv)
{
	struct file *file = (struct file *)priv;
	struct file_buffer *f_buffer;

	if (!file)
		return -1;

	f_buffer = (struct file_buffer *)mm_alloc(sizeof(struct file_buffer));
	if (!f_buffer)
		return -1;

	f_buffer->buffer = (void *)start;
	f_buffer->len = len;
	f_buffer->pos = 0;

	spin_lock(&file->lock);
	list_add_tail(&f_buffer->list, &file->buffer);
	spin_unlock(&file->lock);

	return 0;
}

void *fopen(const char *filename, const char *mode)
{
	char file_dir[64];
	char *ptr = (char *)filename;
	char *tmp = file_dir;
	struct dir_entry *parent = get_init_fs_root();
	struct file_entry *file_e;
	struct file *file;

	if (!parent) {
		print("Can not find init fs root...\n");
		return NULL;
	}

	while (*ptr != 0) {
		if (*ptr == '/') {
			if (tmp == file_dir) {
				ptr++;
				continue;
			}
			*tmp = 0;
			parent = get_dir_entry(parent, file_dir);
			if (!parent)
				return NULL;

			tmp = file_dir;
			ptr++;
			continue;
		}

		*tmp++ = *ptr++;
	}
	*tmp = 0;

	spin_lock(&lock);
	list_for_each_entry(file, &file_list, list) {
		if (!strcmp(file->name, filename))
			return file;
	}
	spin_unlock(&lock);

	file_e = get_file_entry(parent, file_dir);
	if (!file_e)
		return NULL;

	file = mm_alloc(sizeof (struct file));
	if (!file)
		return NULL;
	INIT_LIST_HEAD(&file->buffer);
	file->file_e = file_e;
	file->pos = 0;
	file->end_pos = 0;
	strcpy(file->name, (char *)filename);
	__SPINLOCK_INIT(&file->lock);

	spin_lock(&lock);
	list_add_tail(&file->list, &file_list);
	spin_unlock(&lock);

	load_file(file_e, file_parse_content, (void *)file);
	set_file_buffer_pos(file);
	set_file_end_pos(file);

	return file;
}
