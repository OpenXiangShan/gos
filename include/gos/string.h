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

#ifndef STRING_H
#define STRING_H

int strcmp(const char *cs, const char *ct);
unsigned long strtoul(const char *cp, char *endp, unsigned int base);
char *strrchr(const char *s, int c);
int strnlen(const char *s, int count);
char *strchr(const char *s, int c);
void *memchr(const void *s, int c, int n);
void *memmove(void *dest, const void *src, int count);
int memcmp(const void *cs, const void *ct, int count);
void strcpy(char *dst, char *src);
int strlen(const char *input);
int strncmp(const char *cs, const char *ct, int count);
void memset(const char *dst, char val, unsigned int size);
void memcpy(char *dst, char *src, unsigned int size);
unsigned long atoi(char *in);
int is_digit(char *in);
int sprintf(char *out, const char *fmt, ...);

#endif
