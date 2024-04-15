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

#endif
