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
CONFIGS_DIR = $(TOPDIR)/configs
DTS_DIR = $(CONFIGS_DIR)/dts

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
	rm -rf include/gos-auto
	mv include/linux include/gos-auto
	echo "#define BUILD_TIME \"$(shell date '+%Y-%m-%d %H:%M:%S')\"" >> include/gos-auto/autoconf.h
	echo "#define BUILD_USER \"$(shell id -un)\"" >> include/gos-auto/autoconf.h

menuconfig: scripts/kconfig/mconf
	$< Kconfig

# build gos
GOS_CORE_DIR = $(TOPDIR)/core
export GOS_CORE_DIR

GOS_TARGET := gos.elf
GOS_TARGET_BIN := gos.bin

-include include/config/auto.conf

obj-y += entry/
obj-y += lib/
obj-y += fdt/
obj-y += drivers/
obj-y += core/
obj-y += mm/
obj-y += app/
obj-$(CONFIG_VIRT) += virt/
obj-$(CONFIG_USER) += user/

gos: autoconf mysbi_bin myUser_bin myGuest_bin $(GOS_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	make -f $(TOPDIR)/Makefile.build obj=.
	$(LD) -T ./gos.lds -o $(BUILD_DIR)/$(GOS_TARGET) $(GOS_OBJ_FILES) built-in.o -Map $(BUILD_DIR)/gos.map --no-warn-rwx-segments
	$(RISCV_COPY) $(BUILD_DIR)/$(GOS_TARGET) -O binary $(BUILD_DIR)/$(GOS_TARGET_BIN)

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
	rm -rf include/gos-auto
	rm $(DTS_DIR)/*.dtb

%.dtb: $(DTS_DIR)/$(patsubst %.dtb,%.dts,$@)
	dtc -O dtb -I dts -o $(DTS_DIR)/$@ $(DTS_DIR)/$(patsubst %.dtb,%.dts,$@)
	cp $(DTS_DIR)/$@ input.dtb

ifeq ($(CONFIG_SELECT_QEMU), y)
ifeq ($(CONFIG_SELECT_PLIC), y)
run:
	./qemu-system-riscv64 -nographic \
	-machine virt -smp 2 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on -m 8G \
	-bios out/Image.bin
run-debug:
	./qemu-system-riscv64 -nographic \
	-machine virt -smp 2 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on -m 8G \
	-bios out/Image.bin \
	-S -s
else ifeq ($(CONFIG_SELECT_AIA), y)
run:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on -m 8G \
        -bios out/Image.bin
run-debug:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on -m 8G \
        -bios out/Image.bin \
	-S -s
endif #CONFIG_SELECT_PLIC

else ifeq ($(CONFIG_SELECT_VCS), y)
ifeq ($(CONFIG_SELECT_PLIC), y)
run:
	./riscv64-nemu-interpreter -b out/Image.bin
run-debug:
	@echo "Do not support debug in NEMU..."
endif #CONFIG_SELECT_PLIC

endif #CONFIG_SELECT_QEMU

format:
	find . -name *.c |xargs ./Lindent
	find . -name *.h |xargs ./Lindent
	find . -name *.c~ |xargs rm
	find . -name *.h~ |xargs rm
