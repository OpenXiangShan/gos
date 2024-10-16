# Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2 or later, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

GNU = riscv64-unknown-linux-gnu-

TOPDIR := $(shell pwd)

CC := $(GNU)gcc
LD := $(GNU)ld
NM := $(GNU)nm
RISCV_COPY := $(GNU)objcopy
RISCV_COPY_FLAGS := --set-section-flags .bss=alloc,contents --set-section-flags .sbss=alloc,contents -O binary
RISCV_DUMP := $(GNU)objdump

COPS := -g -O0 -Wall -nostdlib -mcmodel=medany -mabi=lp64d -march=rv64gcv_zba_zbb_zbc_zbs -fno-PIE -fomit-frame-pointer -Wno-builtin-declaration-mismatch -fno-zero-initialized-in-bss

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
	@echo "Packing..."
	@./pack.sh

fpga:
	./bin2fpgadata -i out/Image.bin

defconfig: scripts/kconfig/conf FORCE
	scripts/kconfig/conf -D configs/$@ Kconfig

%_defconfig: scripts/kconfig/conf FORCE
	scripts/kconfig/conf -D configs/$@ Kconfig

autoconf: scripts/kconfig/conf FORCE
	echo "Running autoconf..."
	@mkdir -p include/config
	@mkdir -p include/linux
	@$< -s Kconfig
	@rm -rf include/gos-auto
	@mv include/linux include/gos-auto
	@echo "#define BUILD_TIME \"$(shell date '+%Y-%m-%d %H:%M:%S')\"" >> include/gos-auto/autoconf.h
	@echo "#define BUILD_USER \"$(shell id -un)\"" >> include/gos-auto/autoconf.h
	@echo "#define BUILD_GIT_INFO \"$(shell git log --oneline -1)\"" >> include/gos-auto/autoconf.h

menuconfig: scripts/kconfig/mconf
	$< Kconfig

KBUILD_CFLAGS +=CONFIG_MEM_START=$(CONFIG_MEM_START)
export KBUILD_CFLAGS

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

GOS_DIR = gos
GOS_C_FILES = $(wildcard $(GOS_DIR)/*.c)
GOS_ASM_FILES = $(wildcard $(GOS_DIR)/*.S)
GOS_OBJ_FILES = $(GOS_C_FILES:$(GOS_DIR)/%.c=$(GOS_DIR)/%_c.o)
GOS_OBJ_FILES += $(GOS_ASM_FILES:$(GOS_DIR)/%.S=$(GOS_DIR)/%_s.o)

gos: gos-tmp $(GOS_OBJ_FILES) $(GOS_DIR)/tmp_kallsyms_s.o
	@$(LD) -T ./gos.lds -o $(BUILD_DIR)/$(GOS_TARGET) built-in.o $(GOS_OBJ_FILES) $(GOS_DIR)/tmp_kallsyms_s.o -Map $(BUILD_DIR)/gos.map --no-warn-rwx-segments
	@$(RISCV_COPY) $(BUILD_DIR)/$(GOS_TARGET) -O binary $(BUILD_DIR)/$(GOS_TARGET_BIN)

gos-tmp: mysbi_bin myUser_bin myGuest_bin
	@mkdir -p $(BUILD_DIR)
	@make -f $(TOPDIR)/Makefile.build obj=. --no-print-directory
	@$(LD) -T ./gos.lds -o $(BUILD_DIR)/.tmp_gos.elf built-in.o -Map $(BUILD_DIR)/.tmp_gos.map --no-warn-rwx-segments
	@$(NM) $(BUILD_DIR)/.tmp_gos.elf > $(BUILD_DIR)/.tmp_gos_label
	@scripts/kallsyms --all-symbols $(BUILD_DIR)/.tmp_gos_label > $(GOS_DIR)/tmp_kallsyms.S
	@rm $(BUILD_DIR)/.tmp_gos_label
	@rm $(BUILD_DIR)/.tmp_gos.elf
	@rm $(BUILD_DIR)/.tmp_gos.map

$(GOS_DIR)/%_c.o: $(GOS_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(TOPDIR)/include/gos -I$(TOPDIR)/include -c $< -o $@

$(GOS_DIR)/%_s.o: $(GOS_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(TOPDIR)/include/gos -I$(TOPDIR)/include -c $< -o $@

$(GOS_DIR)/tmp_kallsyms_s.o: $(GOS_DIR)/tmp_kallsyms.S gos-tmp
	@echo "CC $@"
	@$(CC) $(COPS) -I$(TOPDIR)/include/gos -I$(TOPDIR)/include -c $< -o $@

mysbi_bin:
	@make -C mysbi --no-print-directory

myUser_bin:
ifdef CONFIG_MYUSER
	@make -C myUser --no-print-directory
endif

myGuest_bin:
ifdef CONFIG_MYGUEST
	@make -C myGuest --no-print-directory
endif

mysbi-clean:
	@make -C mysbi clean --no-print-directory

myUser-clean:
	@make -C myUser clean --no-print-directory

myGuest-clean:
	@make -C myGuest clean --no-print-directory

clean: mysbi-clean myGuest-clean myUser-clean
	@rm -rf $(shell find -name "*.o")
	@rm -rf $(BUILD_DIR)
	@rm -rf out
	@rm -rf include/config
	@rm -rf include/gos-auto
	@rm -f $(DTS_DIR)/*.dtb
	@rm $(GOS_DIR)/tmp_kallsyms.S

%.dtb: $(DTS_DIR)/$(patsubst %.dtb,%.dts,$@)
	dtc -O dtb -I dts -o $(DTS_DIR)/$@ $(DTS_DIR)/$(patsubst %.dtb,%.dts,$@)
	cp $(DTS_DIR)/$@ input.dtb

ifeq ($(CONFIG_SELECT_QEMU), y)
ifeq ($(CONFIG_SELECT_PLIC), y)
run:
	./qemu-system-riscv64 -nographic \
	-machine virt -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on,svpbmt=on,svinval=on,x-zicond=on -m 8G \
	-device my_dmaengine \
	-bios out/Image.bin
run-debug:
	./qemu-system-riscv64 -nographic \
	-machine virt -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on,svpbmt=on,svinval=on,x-zicond=on -m 8G \
	-device my_dmaengine \
	-bios out/Image.bin \
	-S -s
else ifeq ($(CONFIG_SELECT_AIA), y)
run:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on,svpbmt=on,svinval=on,x-zicond=on,v=on -m 8G \
	-device my_dmaengine \
	-bios out/Image.bin
run-debug:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on,sv48=on,sv57=on,svnapot=on,svpbmt=on,svinval=on,x-zicond=on,v=on -m 8G \
	-device my_dmaengine \
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
