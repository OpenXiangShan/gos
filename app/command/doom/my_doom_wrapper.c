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

#include "print.h"
#include "clock.h"
#include "mm.h"
#include "fs.h"
#include "string.h"
#include "DOOM/DOOM.h"
#include "display.h"
#include "event.h"
#include "timer.h"

static char buf[320*200*2] = { 0 };
static char ascii_render_map[] = { ' ', '.', ':', '-', '=', '+', '*', '#', '%', '@'};
#define ASCII_RENDER_MAP_U (sizeof(ascii_render_map) / sizeof(ascii_render_map[0]))

static struct display_device *dp;

static char last_key = 0;

static void do_ascii_render(char *out_buf)
{
	char pix;
	int w = 320;
	int h = 200;
	int factor = 255 / ASCII_RENDER_MAP_U;
	int index, n = 0;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			pix = out_buf[i * w + j];
			index = pix / factor;
			if (index >= ASCII_RENDER_MAP_U)
				index = ASCII_RENDER_MAP_U - 1;
			buf[n++] = ascii_render_map[index];
		//	print("%c", ascii_render_map[index]);
		}
		buf[n++] = '\n';
		//print("\n");
	}
	buf[n++] = 0;

	display(dp, buf);
}

static void my_doom_print(const char* str)
{
	print(str);
}

static void* my_doom_malloc(int size)
{
	return mm_alloc(size);
}

static void my_doom_free(void* ptr)
{

}

static void my_doom_gettime(int* sec, int* usec)
{
	unsigned long time;

	time = get_clocksource_counter_us();
	*sec = time / 1000000;
	*usec = time % 1000000;
}
static void *my_doom_open(const char* filename, const char* mode)
{
	return fopen(filename, mode);
}
static void my_doom_close(void* handle)
{
	fclose(handle);
}
static int my_doom_read(void* handle, void *buf, int count)
{
	return fread(handle, buf, count);
}
static int my_doom_write(void* handle, const void *buf, int count)
{
	print("%s --\n", __FUNCTION__);
	return 0;
}
static int my_doom_seek(void* handle, int offset, doom_seek_t origin)
{
	int whence;

	if (origin == DOOM_SEEK_CUR)
		whence = SEEK_CUR;
	else if (origin == DOOM_SEEK_SET)
		whence = SEEK_SET;
	else if (origin == DOOM_SEEK_END)
		whence = SEEK_END;
	else
		return -1;

	return fseek(handle, offset, whence);
}
static int my_doom_tell(void* handle)
{
	return ftell(handle);
}
static int my_doom_eof(void* handle)
{
	print("%s --\n", __FUNCTION__);
	return 0;
}
static char* my_doom_getenv(const char* var)
{
	char *ret;

	ret = (char *)mm_alloc(16);
	memset(ret, 0, 16);

	if (!strcmp((char *)var, "HOME")) {
		*ret = '/';
		return ret;
	}

	return NULL;
}

static void display_init(void)
{
	dp = get_display_device("MY_CHR_DISPLAY");
	request_display_buffer(dp, 320*200 + 200);
}

static void doom_render_timer_handler(void *data)
{
	char *out_buf;

	doom_update();
	out_buf = (char *)doom_get_framebuffer(1);
	do_ascii_render(out_buf);
}

static int event_process(char command)
{
	static int arrow_flag = 1;

	if (command == 27) {
		arrow_flag = 1;
	} else if (command == 91 && arrow_flag == 1) {
		arrow_flag = 2;
	} else if (arrow_flag == 2) {
		arrow_flag = 0;
		if (command == 65 /* up */ ) {
			doom_key_down(DOOM_KEY_UP_ARROW);
		} else if (command == 66 /* down */) {
			doom_key_down(DOOM_KEY_DOWN_ARROW);
		} else if (command == 67 /* down */) {
			doom_key_down(DOOM_KEY_LEFT_ARROW);
		} else if (command == 68 /* down */) {
			doom_key_down(DOOM_KEY_RIGHT_ARROW);
		}
	} else {
		doom_key_down(command);
	}

	return 0;
}

static void event_loop_start(int (*process)(char command))
{
	int fd, i;
	int ret;
	char buf[64];

	fd = open("UART0");
	if (fd < 0) {
		print("open %s failed...\n", "UART0");
		return;
	}

	while (1) {
		ret = read(fd, buf, 0, 64, BLOCK);
		for (i = 0; i < ret; i++) {
			if (process(buf[i]))
				goto end;
		}
	}
end:
	return;
}

void doom_run(int argc, char **argv)
{
	struct timer_event_info *timer;

	doom_set_print(my_doom_print);
	doom_set_malloc(my_doom_malloc, my_doom_free);
	doom_set_file_io(my_doom_open,
			 my_doom_close,
			 my_doom_read,
			 my_doom_write,
			 my_doom_seek,
			 my_doom_tell,
			 my_doom_eof);
	doom_set_gettime(my_doom_gettime);
	doom_set_getenv(my_doom_getenv);

	doom_set_default_int("key_up", DOOM_KEY_W);
	doom_set_default_int("key_down", DOOM_KEY_S);
	doom_set_default_int("key_strafeleft", DOOM_KEY_A);
	doom_set_default_int("key_straferight", DOOM_KEY_D);
	doom_set_default_int("key_left", DOOM_KEY_J);
	doom_set_default_int("key_right", DOOM_KEY_K);
	doom_set_default_int("key_use", DOOM_KEY_E);
	doom_set_default_int("key_fire", DOOM_KEY_SPACE);

	doom_init(argc, argv, 0);

	display_init();

	timer = set_timer_restart_cpu(500, doom_render_timer_handler, NULL, 1);
	event_loop_start(event_process);

	del_timer_cpu(timer, 1);
	wait_for_ms(1000);
}
