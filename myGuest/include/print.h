#ifndef __GUEST_PRINT_H
#define __GUEST_PRINT_H

int myGuest_print(const char *fmt, ...);

#define print myGuest_print

#endif
