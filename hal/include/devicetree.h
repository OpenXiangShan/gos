#ifndef __DEVICETREE_H
#define __DEVICETREE_H

int dtb_scan_memory(void *dtb_addr,
		    void (*fn)(unsigned long base, unsigned long size));
int dtb_scan_cpus(void *dtb_addr,
		  void (*fn)(void *dtb, int offset, void *data), void *data);
void parse_dtb();

#endif
