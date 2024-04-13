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
