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

void strcpy(char *dst, char *src);
int strlen(char *input);
int strncmp(const char *cs, const char *ct, int count);
void memset(const char *dst, char val, unsigned int size);
void memcpy(char *dst, char *src, unsigned int size);
unsigned long atoi(char *in);
int is_digit(char *in);

#endif
