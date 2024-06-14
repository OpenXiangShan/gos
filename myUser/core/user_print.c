#include "syscall.h"
#include "type.h"
#include "string.h"
#include "spinlocks.h"

#define MAX_PRINT_LENGTH 128

static char log_buf[MAX_PRINT_LENGTH];
static unsigned int pos;

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

const unsigned char hex_tab[] =
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
	'f'
};

spinlock_t _lock __attribute__((section(".data"))) = __SPINLOCK_INITIALIZER;

#define  MAX_NUMBER_BYTES  64
#define  F_PRECISION       8
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

static void __putc(char c)
{
	if (pos >= MAX_PRINT_LENGTH) {
		log_buf[MAX_PRINT_LENGTH - 1] = 0;
		return;
	}

	if (c == '\n') {
		log_buf[pos++] = '\r';
		log_buf[pos++] = '\n';
	} else
		log_buf[pos++] = c;
}

static void __puts(char *str)
{
	while (*str != '\0')
		__putc(*str++);
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

	__puts(s);
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

	__putc('.');
	for(int i = 0; i < F_PRECISION; i++){
		frac_part *=10;
	}
	int_part = my_round(frac_part);
	out_num(int_part, base, lead, maxwidth);
}
static int my_vprintf(const char *fmt, va_list ap)
{
	char lead = ' ';
	int maxwidth = 0;

	for (; *fmt != '\0'; fmt++) {
		if (*fmt != '%') {
			__putc(*fmt);
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
			out_num(va_arg(ap, u64), 10, lead, maxwidth);
			break;
		case 'x':
		case 'X':
			out_num(va_arg(ap, u64), 16, lead, maxwidth);
			break;
		case 'l':
			if (*(fmt + 1) == 'x') {
				fmt++;
				out_num(va_arg(ap, u64), 16, lead, maxwidth);
			}
			break;
		case 'b':
			out_num(va_arg(ap, u64), 2, lead, maxwidth);
			break;
		case 'c':
			__putc(va_arg(ap, int));
			break;
		case 's':
			__puts(va_arg(ap, char *));
			break;
		case 'f':
			double value = va_arg(ap, double);
			my_ftoc(value, 10, lead, maxwidth);
			break;
		default:
			__putc(*fmt);
			break;
		}
	}

	return 0;
}

void printf(const char *fmt, ...)
{
	va_list ap;
	char user_log[32];
	char *tmp = user_log;
	char *buf = log_buf;

	spin_lock(&_lock);
	pos = 0;

	strcpy(tmp, "[User log] ");
	__puts(tmp);

	va_start(ap, fmt);
	my_vprintf(fmt, ap);
	va_end(ap);

	log_buf[pos++] = 0;

	syscall(__NR_print, buf, pos);
	spin_unlock(&_lock);
}
