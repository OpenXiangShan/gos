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

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
static const unsigned char _hex_tab[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
	'f'
};
#define BACKSPACE_ASCII 8

#define  MAX_NUMBER_BYTES  64
#define  F_PRECISION       8
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

static int _out_num(char *out, unsigned long n, int base, char lead, int maxwidth)
{
	int ret = 0;
	unsigned long m = 0;
	char buf[MAX_NUMBER_BYTES], *s = buf + sizeof(buf);
	int count = 0, i = 0;

	*--s = '\0';

	if (n < 0)
		m = -n;
	else
		m = n;

	do {
		*--s = _hex_tab[m % base];
		count++;

		if (base == 2 && (count % 8 == 0)) {
			*--s = ' ';
		}
	}
	while ((m /= base) != 0);

	if (maxwidth && count < maxwidth) {
		for (i = maxwidth - count; i; i--) {
			*--s = lead;
			if (base == 2 && ((++count) % 8 == 0)) {
				*--s = ' ';
			}
		}
	}

	if (n < 0)
		*--s = '-';

	while (*s != '\0') {
		*out++ = *s++;
		ret++;
	}

	return ret;
}

static int my_round(double num)
{
	int int_part = (int)num;
	double frac_part = num - int_part;
	if(frac_part >= 0.5){
		return int_part +1;
	}else{
		return int_part;
	}
}
static int my_ftoc(char *out, double value, int base, char lead,  int maxwidth)
{
	int ret = 0;
	int int_part = (int)value;
	double frac_part = value - int_part;
	_out_num(out, int_part, base, lead, maxwidth);

	*out = '.';
	ret++;
	for(int i = 0; i < F_PRECISION; i++){
		frac_part *=10;
	}
	int_part = my_round(frac_part);
	ret += _out_num(out, int_part, base, lead, maxwidth);

	return ret;
}

static int my_vsprintf(char *out, const char *fmt, va_list ap)
{
	char lead = ' ';
	int maxwidth = 0;

	for (; *fmt != '\0'; fmt++) {
		if (*fmt != '%') {
			*out++ = *fmt;
			continue;
		}
		lead = ' ';
		maxwidth = 0;

		fmt++;
		if (*fmt == '0') {
			lead = '0';
			fmt++;
		}

		while (*fmt >= '0' && *fmt <= '9') {
			maxwidth *= 10;
			maxwidth += (*fmt - '0');
			fmt++;
		}

		switch (*fmt) {
		case 'd':
			out += _out_num(out, va_arg(ap, s64), 10, lead, maxwidth);
			break;
		case 'o':
			out += _out_num(out, va_arg(ap, u64), 8, lead, maxwidth);
			break;
		case 'u':
			out += _out_num(out, va_arg(ap, u64), 10, lead, maxwidth);
			break;
		case 'x':
		case 'X':
			out += _out_num(out, va_arg(ap, u64), 16, lead, maxwidth);
			break;
		case 'l':
			if (*(fmt + 1) == 'l') {
				if (*(fmt + 2) == 'u') {
					fmt+=2;
					out += _out_num(out, va_arg(ap, u64), 10, lead, maxwidth);
				} else if (*(fmt + 2) == 'x') {
					fmt+=2;
					out += _out_num(out, va_arg(ap, u64), 16, lead, maxwidth);
				}
			} else if (*(fmt + 1) == 'u') {
				fmt++;
				out += _out_num(out, va_arg(ap, u64), 10, lead, maxwidth);
			} else if (*(fmt + 1) == 'x') {
				fmt++;
				out += _out_num(out, va_arg(ap, u64), 16, lead, maxwidth);
			} else if (*(fmt + 1) == 'd') {
				fmt++;
				out += _out_num(out, va_arg(ap, s64), 10, lead, maxwidth);
			}
			break;
		case 'b':
			out += _out_num(out, va_arg(ap, u64), 2, lead, maxwidth);
			break;
		case 'c':
			*out++ = va_arg(ap, int);
			break;
		case 's':
			char *s = va_arg(ap, char *);
			while (*s != '\0')
				*out++ = *s++;
			break;
		case 'f':
			double value = va_arg(ap, double);
			out += my_ftoc(out, value, 10, lead, maxwidth);
			break;
		default:
			*out++ = *fmt;
			break;
		}
	}

	return 0;
}

int sprintf(char *out, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	my_vsprintf(out, fmt, ap);
	va_end(ap);

	return 0;
}
