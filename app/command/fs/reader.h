#ifndef __FS_READER_H__
#define __FS_READER_H__

struct file_reader {
	char *buf;
	int buf_size;
	int width;
	int height;
	int pos;
	int current_line;
};

int reader_load_file(unsigned long start, unsigned int len, void *priv);
int reader_enter(void);
void reader_exit(void);
int reader_init(void);

#endif
