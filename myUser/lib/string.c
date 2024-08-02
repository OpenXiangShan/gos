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

int strlen(char *input)
{
	int len = 0;

	while (*input++)
		len++;

	return len;
}

int strncmp(const char *cs, const char *ct, int count)
{
	unsigned char c1, c2;

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
	int val = 0;
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
