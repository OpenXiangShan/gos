#ifndef __USER_VMAP_H
#define __USER_VMAP_H

void user_vmap_free(void *addr, unsigned int size);
void *user_vmap_alloc(unsigned int size);

#endif
