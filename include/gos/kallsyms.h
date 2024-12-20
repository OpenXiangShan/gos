#ifndef __KALLSYMS_H__
#define __KALLSYMS_H__

#define KSYM_NAME_LEN 512

unsigned long __attribute__((weak, section(".gos_stub_func"))) kallsyms_lookup_name(const char *name);
int __attribute__((weak, section(".gos_stub_func"))) lookup_symbol_name(unsigned long addr, char *symname);

#endif
