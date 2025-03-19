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
#include "string.h"
#include "mm.h"
#include "print.h"
#include "reader.h"

static struct file_reader reader = { 0 };

int reader_load_file(unsigned long start, unsigned int len, void *priv)
{
	char *ptr;

	if (!reader.buf) {
		reader.buf_size = len;
		reader.buf = (char *)mm_alloc(reader.buf_size);
		if (!reader.buf) {
			print("%s: Out of memory\n");
			return -1;
		}
		ptr = reader.buf;
	} else {
		char *new;
		int new_size = reader.buf_size + len;

		new = (char *)mm_alloc(new_size);
		if (!new) {
			print("%s: Out of memory\n");
			return -1;
		}
		memcpy(new, reader.buf, reader.buf_size);

		mm_free((void *)reader.buf, reader.buf_size);

		ptr = new + reader.buf_size;
		reader.buf_size = new_size;
		reader.buf = new;
	}

	memcpy(ptr, (char *)start, len);

	return 0;
}

static int reader_get_last_line(char *buf, int width)
{
	int l_pos = 0;
	int _pos = 0;

	while (*buf) {
		if (*buf == '\n') {
			_pos = 0;
			l_pos++;
			goto next;
		} else if (((_pos % width) == 0) && (_pos != 0)) {
			_pos = 0;
			l_pos++;
			goto next;
		}

		_pos++;
next:
		buf++;
	}

	return (l_pos - 1);
}

static char *reader_get_start_pos(char *buf, int line, int width)
{
	int l_pos = 0;
	int _pos = 0;

	while (*buf) {
		if (line == l_pos)
			return buf;

		if (*buf == '\n') {
			_pos = 0;
			l_pos++;
		} else if (((_pos % width) == 0) && (_pos != 0)) {
			_pos = 0;
			l_pos++;
		}

		_pos++;
		buf++;
	}

	return NULL;
}

static void print_line_num(int line, int h_pos, int count)
{
	if (count == 1)
		print("%d ", line + h_pos);
	else if (count == 2)
		print("%2d ", line + h_pos);
	else if (count == 3)
		print("%3d ", line + h_pos);
	else if (count == 4)
		print("%4d ", line + h_pos);
	else if (count == 5)
		print("%5d ", line + h_pos);
	else if (count == 6)
		print("%6d ", line + h_pos);

}

static int __get_line_num_count(int line)
{
	int ret = 0;

	while (line) {
		ret++;
		line /= 10;
	}

	return ret;
}

static void reader_print(char *ptr, int line, int width, int height,
			 int show_line_num, int line_num_count)
{
	int h_pos, w_pos;

	print("\033c");
	if (show_line_num)
		print_line_num(line + 1, 0, line_num_count);

	for (h_pos = 0; h_pos < height; h_pos++) {
		for (w_pos = 0; w_pos < width; w_pos++) {
			if (*ptr == 0)
				return;

			print("%c", *ptr);

			if (*ptr == '\n') {
				ptr++;
				goto next_line;
			}
			ptr++;
		}

		if (*ptr != '\n')
			print("\n");
next_line:
		if (show_line_num)
			print_line_num(line + 2, h_pos, line_num_count);
	}
}

int reader_enter(void)
{
	char *ptr;
	int s_step = 10;
	int fd, step = s_step;
	int line = 0, pre_line = line, last_line = 0;
	int show_line_num = 0, line_num_count = 0;
	char buf[64], last_input;
	unsigned long single = 0;

	fd = open("UART0");
	if (fd < 0) {
		print("open %s failed...\n", "UART0");
		return -1;
	}

	last_line = reader_get_last_line(reader.buf, reader.width);
	line_num_count = __get_line_num_count(last_line);

	while (1) {
		char input;

		if ((line == pre_line) && (single++ != 0))
			goto wait_for_input;

		ptr = reader_get_start_pos(reader.buf, line, reader.width);
		if (!ptr) {
			line = pre_line;
			goto wait_for_input;
		}

		pre_line = line;

		reader_print(ptr, line, reader.width, reader.height,
			     show_line_num, line_num_count);

wait_for_input:
		read(fd, buf, 0, 64, BLOCK);
		input = buf[0];

		if (input == 'j') {
			line++;
		} else if (input == 'k') {
			if (line > 0)
				line--;
		} else if (input == 'G') {
			line = last_line;
		} else if (input == 'g') {
			line = 0;
		} else if (input == 'd') {
			if (last_input == input)
				step += 1;
			else
				step = s_step;
			line += step;
		} else if (input == 'w') {
			if (last_input == input)
				step += 1;
			else
				step = s_step;

			if (line > step)
				line -= step;
			else
				line = 0;
		} else if (input == 't') {
			show_line_num = 1;
			reader_print(ptr, line, reader.width, reader.height,
				     show_line_num, line_num_count);
		} else if (input == 'q') {
			print("\n");
			break;
		}

		last_input = input;
	}

	return 0;
}

void reader_exit(void)
{
	mm_free((void *)reader.buf, reader.buf_size);
	reader.buf = NULL;
	reader.buf_size = 0;
}

int reader_init(void)
{
	reader.buf = NULL;
	reader.buf_size = 0;
	reader.width = 200;
	reader.height = 40;

	return 0;
}
