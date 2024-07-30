/*
 * memtester version 4
 *
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2020 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */
#define __version__ "4.5.0"
#include <asm/type.h>
#include <print.h>
#include <device.h>
#include "../command.h"
#include "task.h"
#include "string.h"
#include "mm.h"
#include "clock.h"

typedef unsigned long ul;
typedef unsigned long long ull;
typedef unsigned long volatile ulv;
typedef unsigned char volatile u8v;
typedef unsigned short volatile u16v;
typedef unsigned int  uint32_t;
typedef unsigned long size_t;

#define rand64()    get_clocksource_counter_us()
#define TEST_NARROW_WRITES
#define ULONG_MAX  64UL
#define printf              print
#define fflush(stdout)

#if (ULONG_MAX == 32UL)
    #define FMT_TARGET "0x%08lx"
    #define rand_ul() rand32()
    #define UL_ONEBITS 0xffffffff
    #define UL_LEN 32
    #define CHECKERBOARD1 0x55555555
    #define CHECKERBOARD2 0xaaaaaaaa
    #define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))
#elif (ULONG_MAX == 64UL)
    #define FMT_TARGET "0x%016lx"
    //#define rand64() (((ul) rand32()) << 32 | ((ul) rand32()))
    #define rand_ul() rand64()
    #define UL_ONEBITS 0xffffffffffffffffUL
    #define UL_LEN 64
    #define CHECKERBOARD1 0x5555555555555555
    #define CHECKERBOARD2 0xaaaaaaaaaaaaaaaa
    #define UL_BYTE(x) (((ul)x | (ul)x<<8 | (ul)x<<16 | (ul)x<<24 | (ul)x<<32 | (ul)x<<40 | (ul)x<<48 | (ul)x<<56))
#else
    #error long on this platform is not 32 or 64 bits
#endif

struct test {
    char *name;
    int (*fp)();
};

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04

int test_stuck_address(unsigned long volatile *bufa, size_t count);
int test_random_value(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_xor_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_sub_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_mul_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_div_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_or_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_and_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_seqinc_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_solidbits_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_checkerboard_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_blockseq_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_walkbits0_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_walkbits1_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_bitspread_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_bitflip_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
#ifdef TEST_NARROW_WRITES    
int test_8bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
int test_16bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, size_t count);
#endif

struct test tests[] = {
    { "Random Value",         test_random_value },
    { "Compare XOR",          test_xor_comparison },
    { "Compare SUB",          test_sub_comparison },
    { "Compare MUL",          test_mul_comparison },
    { "Compare DIV",          test_div_comparison },
    { "Compare OR",           test_or_comparison },
    { "Compare AND",          test_and_comparison },
    { "Sequential Increment", test_seqinc_comparison },
    { "Solid Bits",           test_solidbits_comparison },
    { "Block Sequential",     test_blockseq_comparison },
    { "Checkerboard",         test_checkerboard_comparison },
    { "Bit Spread",           test_bitspread_comparison },
    { "Bit Flip",             test_bitflip_comparison },
    { "Walking Ones",         test_walkbits1_comparison },
    { "Walking Zeroes",       test_walkbits0_comparison },
#ifdef TEST_NARROW_WRITES    
    { "8-bit Writes",         test_8bit_wide_random },
    { "16-bit Writes",        test_16bit_wide_random },
#endif
    { NULL, NULL }
};

#ifdef _SC_PAGE_SIZE
int memtester_pagesize(void) {
    int pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        perror("get page size failed");
        exit(EXIT_FAIL_NONSTARTER);
    }
    printf("pagesize is %ld\n", (long) pagesize);
    return pagesize;
}
#else
int memtester_pagesize(void) {
    printf("sysconf(_SC_PAGE_SIZE) not supported; using pagesize of 8192\n");
    return 8192;
}
#endif


/* Global vars - so tests have access to this information */
static int use_phys = 0;
static ul physaddrbase = 0;

void sdram_tester(void *aligned, uint32_t bufsize)
{
    ul loops, loop, i;
    size_t halflen, count;

    ulv *bufa, *bufb;

    int exit_code = 0;

    ul testmask = 0;
    
    printf("memtester version " __version__ " (%d-bit)\n", UL_LEN);
    printf("Copyright (C) 2001-2020 Charles Cazabon.\n");
    printf("Licensed under the GNU General Public License version 2 (only).\n");
    printf("\n");
    
    halflen = bufsize / 2;
    count = halflen / sizeof(ul);
    bufa = (ulv *) aligned;
    bufb = (ulv *) ((size_t) aligned + halflen);

    loops = 1;

    for(loop=1; ((!loops) || loop <= loops); loop++) {
        printf("Loop %lu", loop);
        if (loops) {
            printf("/%lu", loops);
        }
        printf(":\n");
        printf("  %s: ", "Stuck Address");
        fflush(stdout);
        if (!test_stuck_address(aligned, bufsize / sizeof(ul))) {
             printf("ok\n");
        } else {
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        for (i=0;;i++) {
            if (!tests[i].name) break;
            /* If using a custom testmask, only run this test if the
               bit corresponding to this test was set by the user.
             */
            if (testmask && (!((1 << i) & testmask))) {
                continue;
            }
            printf("  %s: ", tests[i].name);
            if (!tests[i].fp(bufa, bufb, count)) {
                printf("ok\n");
            } else {
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
    }

    printf("Done.\n");
    fflush(stdout);
}

static char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define ONE 0x00000001L

union {
    unsigned char bytes[UL_LEN/8];
    ul val;
} mword8;

union {
    unsigned short u16s[UL_LEN/16];
    ul val;
} mword16;


/* Function definitions. */

int compare_regions(ulv *bufa, ulv *bufb, size_t count) {
    int r = 0;
    size_t i;
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul physaddr;

    for (i = 0; i < count; i++, p1++, p2++) {
        if (*p1 != *p2) {
            if (use_phys) {
                physaddr = physaddrbase + (i * sizeof(ul));
                printf("FAILURE: " FMT_TARGET " != " FMT_TARGET " at physical address " FMT_TARGET ".\n",
                        (ul) *p1, (ul) *p2, physaddr);
            } else {
                printf("FAILURE: " FMT_TARGET " != " FMT_TARGET " at offset " FMT_TARGET ".\n",
                        (ul) *p1, (ul) *p2, (ul) (i * sizeof(ul)));
            }
            /* printf("Skipping to next test..."); */
            r = -1;
        }
    }
    return r;
}

int test_stuck_address(ulv *bufa, size_t count) {
    ulv *p1 = bufa;
    unsigned int j;
    size_t i;
    ul physaddr;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < 16; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        printf("setting %3u", j);
        fflush(stdout);
        for (i = 0; i < count; i++) {
            *p1 = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            *p1++;
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        p1 = (ulv *) bufa;
        for (i = 0; i < count; i++, p1++) {
            if (*p1 != (((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1))) {
                if (use_phys) {
                    physaddr = physaddrbase + (i * sizeof(ul));
                    printf( "FAILURE: possible bad address line at physical address " FMT_TARGET ".\n",
                            physaddr);
                } else {
                    printf( "FAILURE: possible bad address line at offset " FMT_TARGET ".\n",
                            (ul) (i * sizeof(ul)));
                }
                printf("Skipping to next test...\n");
                fflush(stdout);
                return -1;
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_random_value(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    ul j = 0;
    size_t i;

    printf("%c", ' ');
    fflush(stdout);
    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = rand_ul();
        if (!(i % PROGRESSOFTEN)) {
            printf("%c", '\b');
            printf("%c", progress[++j % PROGRESSLEN]);
            fflush(stdout);
        }
    }
    printf("\b \b");
    fflush(stdout);
    return compare_regions(bufa, bufb, count);
}

int test_xor_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();
    
    for (i = 0; i < count; i++) {
        *p1++ ^= q;
        *p2++ ^= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_sub_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();
    for (i = 0; i < count; i++) {
        *p1++ -= q;
        *p2++ -= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_mul_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ *= q;
        *p2++ *= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_div_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        if (!q) {
            q++;
        }
        *p1++ /= q;
        *p2++ /= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_or_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ |= q;
        *p2++ |= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_and_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ &= q;
        *p2++ &= q;
    }
    return compare_regions(bufa, bufb, count);
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    size_t i;
    ul q = rand_ul();

    for (i = 0; i < count; i++) {
        *p1++ = *p2++ = (i + q);
    }
    return compare_regions(bufa, bufb, count);
}

int test_solidbits_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < 64; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        printf("setting %3u", j);
        fflush(stdout);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    ul q;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < 64; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
        printf("setting %3u", j);
        fflush(stdout);
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < 256; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        fflush(stdout);
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = ONE << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (j = 0; j < UL_LEN * 2; j++) {
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        printf("setting %3u", j);
        fflush(stdout);
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << j) | (ONE << (j + 2))
                    : UL_ONEBITS ^ ((ONE << j)
                                    | (ONE << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
                                    | (ONE << (UL_LEN * 2 + 1 - j)));
            }
        }
        printf("\b\b\b\b\b\b\b\b\b\b\b");
        printf("testing %3u", j);
        fflush(stdout);
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa;
    ulv *p2 = bufb;
    unsigned int j, k;
    ul q;
    size_t i;

    printf("           ");
    fflush(stdout);
    for (k = 0; k < UL_LEN; k++) {
        q = ONE << k;
        for (j = 0; j < 8; j++) {
            printf("\b\b\b\b\b\b\b\b\b\b\b");
            q = ~q;
            printf("setting %3u", k * 8 + j);
            fflush(stdout);
            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;
            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            }
            printf("\b\b\b\b\b\b\b\b\b\b\b");
            printf("testing %3u", k * 8 + j);
            fflush(stdout);
            if (compare_regions(bufa, bufb, count)) {
                return -1;
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
    fflush(stdout);
    return 0;
}

#ifdef TEST_NARROW_WRITES    
int test_8bit_wide_random(ulv* bufa, ulv* bufb, size_t count) {
    u8v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    printf("%c", ' ');
    fflush(stdout);
    for (attempt = 0; attempt < 2;  attempt++) {
        if (attempt & 1) {
            p1 = (u8v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u8v *) bufb;
            p2 = bufa;
        }
        for (i = 0; i < count; i++) {
            t = mword8.bytes;
            *p2++ = mword8.val = rand_ul();
            for (b=0; b < UL_LEN/8; b++) {
                *p1++ = *t++;
            }
            if (!(i % PROGRESSOFTEN)) {
                printf("%c", '\b');
                printf("%c", progress[++j % PROGRESSLEN]);
                fflush(stdout);
            }
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b \b");
    fflush(stdout);
    return 0;
}

int test_16bit_wide_random(ulv* bufa, ulv* bufb, size_t count) {
    u16v *p1, *t;
    ulv *p2;
    int attempt;
    unsigned int b, j = 0;
    size_t i;

    printf("%c", ' ' );
    fflush( stdout );
    for (attempt = 0; attempt < 2; attempt++) {
        if (attempt & 1) {
            p1 = (u16v *) bufa;
            p2 = bufb;
        } else {
            p1 = (u16v *) bufb;
            p2 = bufa;
        }
        for (i = 0; i < count; i++) {
            t = mword16.u16s;
            *p2++ = mword16.val = rand_ul();
            for (b = 0; b < UL_LEN/16; b++) {
                *p1++ = *t++;
            }
            if (!(i % PROGRESSOFTEN)) {
                printf("%c", '\b');
                printf("%c", progress[++j % PROGRESSLEN]);
                fflush(stdout);
            }
        }
        if (compare_regions(bufa, bufb, count)) {
            return -1;
        }
    }
    printf("\b \b");
    fflush(stdout);
    return 0;
}
#endif
void print_mem(unsigned long address, unsigned int num)
{
	print("address 0x=%lx, num=0x%0x \r\n", address, num);
}
static int cmd_mem_tester_handler(int argc, char *argv[], void *priv)
{
	unsigned long start, address;
	unsigned int len=0;

	if (argc != 2) {
		print("invalid input param.\n");
		return -1;
	}

	if (!is_digit(argv[0]) || !is_digit(argv[1])) {
		print("invalid input param.\n");
		return -1;
	}

	start = atoi(argv[0]);
	start = (start >> 2) << 2;
	if(!(mem_range_is_free(start, len))){
        print("please use unused mem:\n");
        walk_unused_mem_and_print();
        return 0;
	}
	address = phy_to_virt(start);
	len = atoi(argv[1]);

	sdram_tester((void*)address, len);

	return 0;
}

static const struct command cmd_mem_tester = {
	.cmd = "mem_tester",
	.handler = cmd_mem_tester_handler,
	.priv = NULL,
};

int cmd_mem_tester_init()
{
	register_command(&cmd_mem_tester);

	return 0;
}

APP_COMMAND_REGISTER(mem_tester, cmd_mem_tester_init);
