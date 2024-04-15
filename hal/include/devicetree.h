#ifndef __DEVICETREE_H
#define __DEVICETREE_H

int dtb_scan_memory(void *dtb_addr,
		    void (*fn)(unsigned long base, unsigned long size));
void parse_dtb();

#endif
