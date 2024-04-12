#ifndef __USER_MEMORY_H
#define __USER_MEMORY_H

struct user_memory_region *find_user_memory_region(struct user *user,
						   unsigned long va);
int add_user_space_memory(struct user *user, unsigned long start,
			  unsigned int len);
void *user_space_mmap(unsigned int size);
void user_space_unmap(void *addr, unsigned int size);

#endif
