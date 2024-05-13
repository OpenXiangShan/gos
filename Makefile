GNU = riscv64-unknown-linux-gnu-

TOPDIR := $(shell pwd)

CC := $(GNU)gcc
LD := $(GNU)ld
RISCV_COPY := $(GNU)objcopy
RISCV_COPY_FLAGS := --set-section-flags .bss=alloc,contents --set-section-flags .sbss=alloc,contents -O binary
RISCV_DUMP := $(GNU)objdump

COPS := -g -O0 -Wall -nostdlib -mcmodel=medany -mabi=lp64d -march=rv64imafdc -fno-PIE -fomit-frame-pointer -Wno-builtin-declaration-mismatch

.PHONY: FORCE

export TOPDIR
export CC
export LD
export COPS
export DEBUG
export RISCV_COPY

BUILD_DIR = build
MYSBI_DIR = mysbi
BSP_DIR   = bsp
MYUSER_DIR = myUser

default: gos pack.sh
	./pack.sh

fpga:
	./bin2fpgadata -i out/Image.bin

defconfig: scripts/kconfig/conf FORCE
	scripts/kconfig/conf -D configs/$@ Kconfig

%_defconfig: scripts/kconfig/conf FORCE
	scripts/kconfig/conf -D configs/$@ Kconfig

autoconf: scripts/kconfig/conf FORCE
	mkdir -p include/config
	mkdir -p include/linux
	$< -s Kconfig
	rm -rf include/gos
	mv include/linux include/gos

menuconfig: scripts/kconfig/mconf
	$< Kconfig

# build gos
GOS_ENTRY_DIR = entry
GOS_LIB_DIR = lib
GOS_CORE_DIR = $(TOPDIR)/core
APP_DIR   = app
GOS_FDT_DIR = fdt

GOS_VIRT_DIR = virt
GOS_USER_DIR = user

export GOS_CORE_DIR

GOS_TARGET := gos.elf
GOS_TARGET_BIN := gos.bin

GOS_ENTRY_ASM_FILES = $(wildcard $(GOS_ENTRY_DIR)/*.S)
GOS_ENTRY_C_FILES = $(wildcard $(GOS_ENTRY_DIR)/*.c)
GOS_LIB_C_FILES = $(wildcard $(GOS_LIB_DIR)/*.c)
GOS_VIRT_ASM_FILES = $(wildcard $(GOS_VIRT_DIR)/*.S)
GOS_VIRT_C_FILES = $(wildcard $(GOS_VIRT_DIR)/*.c)
GOS_USER_ASM_FILES = $(wildcard $(GOS_USER_DIR)/*.S)
GOS_USER_C_FILES = $(wildcard $(GOS_USER_DIR)/*.c)
GOS_FDT_C_FILES = $(wildcard $(GOS_FDT_DIR)/*.c)

GOS_OBJ_FILES = $(GOS_ENTRY_ASM_FILES:$(GOS_ENTRY_DIR)/%.S=$(GOS_ENTRY_DIR)/%_s.o)
GOS_OBJ_FILES += $(GOS_ENTRY_C_FILES:$(GOS_ENTRY_DIR)/%.c=$(GOS_ENTRY_DIR)/%_c.o)
GOS_OBJ_FILES += $(GOS_LIB_C_FILES:$(GOS_LIB_DIR)/%.c=$(GOS_LIB_DIR)/%_c.o)
GOS_OBJ_FILES += $(GOS_VIRT_ASM_FILES:$(GOS_VIRT_DIR)/%.S=$(GOS_VIRT_DIR)/%_s.o)
GOS_OBJ_FILES += $(GOS_VIRT_C_FILES:$(GOS_VIRT_DIR)/%.c=$(GOS_VIRT_DIR)/%_c.o)
GOS_OBJ_FILES += $(GOS_USER_ASM_FILES:$(GOS_USER_DIR)/%.S=$(GOS_USER_DIR)/%_s.o)
GOS_OBJ_FILES += $(GOS_USER_C_FILES:$(GOS_USER_DIR)/%.c=$(GOS_USER_DIR)/%_c.o)
GOS_OBJ_FILES += $(GOS_FDT_C_FILES:$(GOS_FDT_DIR)/%.c=$(GOS_FDT_DIR)/%_c.o)

obj-y += drivers/
obj-y += core/
obj-y += mm/
obj-y += app/

gos: autoconf mysbi_bin myUser_bin myGuest_bin $(GOS_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	make -f $(TOPDIR)/Makefile.build obj=.
	$(LD) -T ./gos.lds -o $(BUILD_DIR)/$(GOS_TARGET) $(GOS_OBJ_FILES) built-in.o -Map $(BUILD_DIR)/gos.map
	$(RISCV_COPY) $(BUILD_DIR)/$(GOS_TARGET) -O binary $(BUILD_DIR)/$(GOS_TARGET_BIN)

$(GOS_ENTRY_DIR)/%_s.o: $(GOS_ENTRY_DIR)/%.S
	$(CC) $(COPS) -I$(TOPDIR)/include -c $< -o $@

$(GOS_ENTRY_DIR)/%_c.o: $(GOS_ENTRY_DIR)/%.c
	$(CC) $(COPS) -I$(TOPDIR)/include -I$(GOS_CORE_DIR)/include -I$(APP_DIR) $(DEBUG)  -c $< -o $@

$(GOS_LIB_DIR)/%_c.o: $(GOS_LIB_DIR)/%.c
	$(CC) $(COPS) -I$(TOPDIR)/include -I$(GOS_CORE_DIR)/include -c $< -o $@

$(GOS_VIRT_DIR)/%_s.o: $(GOS_VIRT_DIR)/%.S
	$(CC) $(COPS) -I$(TOPDIR)/include -c $< -o $@

$(GOS_VIRT_DIR)/%_c.o: $(GOS_VIRT_DIR)/%.c
	$(CC) $(COPS) -I$(TOPDIR)/include -I$(GOS_CORE_DIR)/include $(DEBUG) -c $< -o $@

$(GOS_USER_DIR)/%_s.o: $(GOS_USER_DIR)/%.S
	$(CC) $(COPS) -I$(TOPDIR)/include -c $< -o $@

$(GOS_USER_DIR)/%_c.o: $(GOS_USER_DIR)/%.c
	$(CC) $(COPS) -I$(TOPDIR)/include -I$(GOS_CORE_DIR)/include -c $< -o $@

$(GOS_FDT_DIR)/%_c.o: $(GOS_FDT_DIR)/%.c
	$(CC) $(COPS) -I$(TOPDIR)/include -c $< -o $@

mysbi_bin: autoconf
	make -C mysbi

myUser_bin: autoconf
	make -C myUser

myGuest_bin: autoconf
	make -C myGuest

mysbi-clean:
	make -C mysbi clean

myUser-clean:
	make -C myUser clean

myGuest-clean:
	make -C myGuest clean

clean: mysbi-clean myGuest-clean myUser-clean
	rm -rf $(shell find -name "*.o")
	rm -rf $(BUILD_DIR)
	rm -rf out
	rm -rf include/config
	rm -rf include/gos
dtb:
	dtc -O dtb -I dts -o input.dtb input.dts

-include include/config/auto.conf

ifeq ($(CONFIG_SELECT_QEMU), y)
ifeq ($(CONFIG_SELECT_PLIC), y)
run:
		./qemu-system-riscv64 -nographic \
		-machine virt -smp 2 \
		-cpu rv64,sv39=on -m 8G \
		-bios out/Image.bin
else ifeq ($(CONFIG_SELECT_AIA), y)
run:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on -m 8G \
        -bios out/Image.bin
endif #CONFIG_SELECT_PLIC

else ifeq ($(CONFIG_SELECT_VCS), y)
ifeq ($(CONFIG_SELECT_PLIC), y)
run:
	./riscv64-nemu-interpreter -b out/Image.bin
else ifeq ($(CONFIG_SELECT_AIA), y)
run:
	@echo "Do not support aia in VCS..."
endif #CONFIG_SELECT_PLIC

endif #CONFIG_SELECT_QEMU

format:
	find . -name *.c |xargs ./Lindent
	find . -name *.h |xargs ./Lindent
	find . -name *.c~ |xargs rm
	find . -name *.h~ |xargs rm
