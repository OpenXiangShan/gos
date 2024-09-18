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

#include <asm/type.h>
#include <uart.h>
#include "clock.h"
#include "string.h"

static int print_time = 1;

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

const unsigned char hex_tab[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
	'f'
};

#define BACKSPACE_ASCII 8

#define  MAX_NUMBER_BYTES  64
#define  F_PRECISION       8
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

static void out_string(char *str, char lead, int maxwidth)
{
	int count = 0, i;
	char *tmp = str;

	while (*tmp++)
		count++;

	uart_puts(str);

	for (i = 0; i < maxwidth - count; i++)
		uart_putc(lead);
}

static void out_num(unsigned long n, int base, char lead, int maxwidth)
{
	unsigned long m = 0;
	char buf[MAX_NUMBER_BYTES], *s = buf + sizeof(buf);
	int count = 0, i = 0;

	*--s = '\0';

	if (n < 0)
		m = -n;
	else
		m = n;

	do {
		*--s = hex_tab[m % base];
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

	uart_puts(s);
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
static void my_ftoc(double value, int base, char lead,  int maxwidth)
{
	int int_part = (int)value;
	double frac_part = value - int_part;
	out_num(int_part, base, lead, maxwidth);

	uart_putc('.');
	for(int i = 0; i < F_PRECISION; i++){
		frac_part *=10;
	}
	int_part = my_round(frac_part);
	out_num(int_part, base, lead, maxwidth);
}

static int my_vprintf(const char *fmt, va_list ap)
{
	char lead = ' ';
	unsigned int maxwidth = 0;

	for (; *fmt != '\0'; fmt++) {
		if (*fmt != '%') {
			uart_putc(*fmt);
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
			out_num(va_arg(ap, s64), 10, lead, maxwidth);
			break;
		case 'o':
			out_num(va_arg(ap, u64), 8, lead, maxwidth);
			break;
		case 'u':
			out_num(va_arg(ap, u32), 10, lead, maxwidth);
			break;
		case 'x':
		case 'X':
			out_num(va_arg(ap, u32), 16, lead, maxwidth);
			break;
		case 'l':
			if (*(fmt + 1) == 'l') {
				if (*(fmt + 2) == 'u') {
					fmt+=2;
					out_num(va_arg(ap, u64), 10, lead, maxwidth);
				} else if (*(fmt + 2) == 'x') {
					fmt+=2;
					out_num(va_arg(ap, u64), 16, lead, maxwidth);
				}
			} else if (*(fmt + 1) == 'u') {
				fmt++;
				out_num(va_arg(ap, u64), 10, lead, maxwidth);
			} else if (*(fmt + 1) == 'x') {
				fmt++;
				out_num(va_arg(ap, u64), 16, lead, maxwidth);
			} else if (*(fmt + 1) == 'd') {
				fmt++;
				out_num(va_arg(ap, s64), 10, lead, maxwidth);
			}
			break;
		case 'b':
			out_num(va_arg(ap, u32), 2, lead, maxwidth);
			break;
		case 'c':
			uart_putc(va_arg(ap, int));
			break;
		case 's':
			out_string(va_arg(ap, char *), lead, maxwidth);
			//uart_puts(va_arg(ap, char *));
			break;
		case 'f':
			double value = va_arg(ap, double);
			my_ftoc(value, 10, lead, maxwidth);
			break;
		default:
			uart_putc(*fmt);
			break;
		}
	}

	return 0;
}

void set_print_time(int en)
{
	print_time = en;
}

int print(const char *fmt, ...)
{
	va_list ap;
	char prefix[128] = { 0 };
	char *tmp = prefix;
	unsigned long time;

	if (print_time) {
		time = get_clocksource_counter_us();

		sprintf(tmp, "[%6ld.%06ld] ", time / 1000000, time % 1000000);
		uart_puts(tmp);
	}

	va_start(ap, fmt);
	my_vprintf(fmt, ap);
	va_end(ap);

	return 0;
}

int print_backspace(int n)
{
	int i;

	for (i = 0; i < n; i++)
		print("%c", BACKSPACE_ASCII);

	for (i = 0; i < n; i++)
		print(" ");

	for (i = 0; i < n; i++)
		print("%c", BACKSPACE_ASCII);

	return 0;
}
