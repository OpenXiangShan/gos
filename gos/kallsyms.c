#include "asm/type.h"
#include "string.h"
#include "kallsyms.h"

extern const unsigned long kallsyms_addresses[];
extern const int kallsyms_offsets[];
extern const char kallsyms_names[];
extern const unsigned int kallsyms_num_syms;
extern const unsigned long kallsyms_relative_base;
extern const char kallsyms_token_table[];
extern const u16 kallsyms_token_index[];
extern const unsigned int kallsyms_markers[];
extern const u8 kallsyms_seqs_of_names[];

static unsigned long kallsyms_sym_address(int idx)
{
	return kallsyms_addresses[idx];
}

static unsigned int get_symbol_seq(int index)
{
	unsigned int i, seq = 0;

	for (i = 0; i < 3; i++)
		seq = (seq << 8) | kallsyms_seqs_of_names[3 * index + i];

	return seq;
}

static unsigned int get_symbol_offset(unsigned long pos)
{
	const char *name;
	int i, len;

	/*
	 * Use the closest marker we have. We have markers every 256 positions,
	 * so that should be close enough.
	 */
	name = &kallsyms_names[kallsyms_markers[pos >> 8]];

	/*
	 * Sequentially scan all the symbols up to the point we're searching
	 * for. Every symbol is stored in a [<len>][<len> bytes of data] format,
	 * so we just need to add the len to the current pointer for every
	 * symbol we wish to skip.
	 */
	for (i = 0; i < (pos & 0xFF); i++) {
		len = *name;

		/*
		 * If MSB is 1, it is a "big" symbol, so we need to look into
		 * the next byte (and skip it, too).
		 */
		if ((len & 0x80) != 0)
			len = ((len & 0x7F) | (name[1] << 7)) + 1;

		name = name + len + 1;
	}

	return name - kallsyms_names;
}

static unsigned int kallsyms_expand_symbol(unsigned int off,
					   char *result, size_t maxlen)
{
	int len, skipped_first = 0;
	const char *tptr;
	const char *data;

	/* Get the compressed symbol length from the first symbol byte. */
	data = &kallsyms_names[off];
	len = *data;
	data++;
	off++;

	/* If MSB is 1, it is a "big" symbol, so needs an additional byte. */
	if ((len & 0x80) != 0) {
		len = (len & 0x7F) | (*data << 7);
		data++;
		off++;
	}

	/*
	 * Update the offset to return the offset for the next symbol on
	 * the compressed stream.
	 */
	off += len;

	/*
	 * For every byte on the compressed symbol data, copy the table
	 * entry for that byte.
	 */
	while (len) {
		tptr =
		    &kallsyms_token_table[kallsyms_token_index[(short)(*data)]];
		data++;
		len--;

		while (*tptr) {
			if (skipped_first) {
				if (maxlen <= 1)
					goto tail;
				*result = *tptr;
				result++;
				maxlen--;
			} else
				skipped_first = 1;
			tptr++;
		}
	}

tail:
	if (maxlen)
		*result = '\0';

	/* Return to offset to the next symbol. */
	return off;
}

static int cleanup_symbol_name(char *s)
{
	return 0;
}

static int compare_symbol_name(const char *name, char *namebuf)
{
	int ret;

	ret = strcmp(name, namebuf);
	if (!ret)
		return ret;

	if (cleanup_symbol_name(namebuf) && !strcmp(name, namebuf))
		return 0;

	return ret;
}

static int kallsyms_lookup_names(const char *name,
				 unsigned int *start, unsigned int *end)
{
	int ret;
	int low, mid, high;
	unsigned int seq, off;
	char namebuf[KSYM_NAME_LEN];

	low = 0;
	high = kallsyms_num_syms - 1;

	while (low <= high) {
		mid = low + (high - low) / 2;
		seq = get_symbol_seq(mid);
		off = get_symbol_offset(seq);
		kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));
		ret = compare_symbol_name(name, namebuf);
		if (ret > 0)
			low = mid + 1;
		else if (ret < 0)
			high = mid - 1;
		else
			break;
	}

	if (low > high)
		return -1;

	low = mid;
	while (low) {
		seq = get_symbol_seq(low - 1);
		off = get_symbol_offset(seq);
		kallsyms_expand_symbol(off, namebuf, ARRAY_SIZE(namebuf));
		if (compare_symbol_name(name, namebuf))
			break;
		low--;
	}
	*start = low;

	if (end) {
		high = mid;
		while (high < kallsyms_num_syms - 1) {
			seq = get_symbol_seq(high + 1);
			off = get_symbol_offset(seq);
			kallsyms_expand_symbol(off, namebuf,
					       ARRAY_SIZE(namebuf));
			if (compare_symbol_name(name, namebuf))
				break;
			high++;
		}
		*end = high;
	}

	return 0;
}

unsigned long __attribute__((section(".gos_stub_func")))
    kallsyms_lookup_name(const char *name)
{
	unsigned int i;

	/* Skip the search for empty string. */
	if (!*name)
		return 0;

	if (kallsyms_lookup_names(name, &i, NULL))
		return 0;

	return kallsyms_sym_address(get_symbol_seq(i));
}
