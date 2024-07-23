#ifndef __GUEST_PRINT_H
#define __GUEST_PRINT_H

int myGuest_print(const char *fmt, ...);
void myGuest_print_init(int vmid);

#define print myGuest_print

#endif
