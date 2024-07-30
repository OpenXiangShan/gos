#include "asm/type.h"
#include "print.h"

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

unsigned long strtoul(const char *cp, char *endp, unsigned int base)
{
	//ToDo
	print("warning!!!!!!!!!! %s ToDo...\n", __FUNCTION__);
	return 0;
}

char *strrchr(const char *s, int c)
{
	const char *last = NULL;
	do {
		if (*s == (char)c)
			last = s;
	} while (*s++);
	return (char *)last;
}

int strnlen(const char *s, int count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */ ;
	return sc - s;
}

char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}

void *memchr(const void *s, int c, int n)
{
	const unsigned char *p = s;
	while (n-- != 0) {
		if ((unsigned char)c == *p++) {
			return (void *)(p - 1);
		}
	}
	return NULL;
}

void *memmove(void *dest, const void *src, int count)
{
	char *tmp;
	const char *s;

	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

int memcmp(const void *cs, const void *ct, int count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

int strlen(const char *input)
{
	int len = 0;

	while (*input++)
		len++;

	return len;
}

int strncmp(const char *cs, const char *ct, int count)
{
	unsigned char c1, c2;

	if (!cs || !ct)
		return -1;

	while (count) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}

	return 0;
}

void strcpy(char *dst, char *src)
{
	while (*src) {
		*dst++ = *src++;
	}
	*dst = 0;
}

void memset(char *dst, char val, unsigned int size)
{
	while (size) {
		*dst++ = val;
		size--;
	}
}

void memcpy(char *dst, char *src, unsigned int size)
{
	while (size) {
		*dst++ = *src++;
		size--;
	}
}

#define TYPE_DECIMAL 0
#define TYPE_HEX 1
unsigned long atoi(char *in)
{
	char *tmp = in;
	int i = 0;
	int ratio = 1;
	unsigned long val = 0;
	unsigned long ret = 0;
	int nbit = 0, hex_xbit = 0;
	char type = TYPE_DECIMAL;

	while (*tmp != '\0') {
		if (*tmp == 'x' || *tmp == 'X') {
			type = TYPE_HEX;
			hex_xbit = nbit;
		}
		nbit++;
		tmp++;
	}

	if (type == TYPE_DECIMAL) {
		for (i = nbit - 1; i >= 0; i--) {
			val = in[i] - '0';
			ret += val * ratio;
			ratio *= 10;
		}
	} else if (type == TYPE_HEX) {
		for (i = nbit - 1; i > hex_xbit; i--) {
			if (in[i] >= 'a' && in[i] <= 'f')
				val = (in[i] - 'a') + 10;
			else if (in[i] >= 'A' && in[i] <= 'F')
				val = (in[i] - 'A') + 10;
			else if (in[i] >= '0' && in[i] <= '9')
				val = in[i] - '0';
			ret += val * ratio;
			ratio *= 16;
		}
	}

	return ret;
}

int is_digit(char *in)
{
	int i = 0;
	char *tmp = in;
	char type = TYPE_DECIMAL;
	int nbit = 0, hex_xbit = 0;

	while (*tmp) {
		if (*tmp == 'x' || *tmp == 'X') {
			type = TYPE_HEX;
			hex_xbit = nbit;
		}
		nbit++;
		tmp++;
	}

	if (type == TYPE_DECIMAL) {
		for (i = nbit - 1; i >= 0; i--) {
			if (in[i] < '0' || in[i] > '9')
				return 0;
		}
	} else if (type == TYPE_HEX) {
		for (i = nbit - 1; i > hex_xbit; i--) {
			if (!((in[i] >= 'a' && in[i] <= 'f') ||
			      (in[i] >= 'A' && in[i] <= 'F') ||
			      (in[i] >= '0' && in[i] <= '9')))
				return 0;
		}
	}

	return 1;
}
