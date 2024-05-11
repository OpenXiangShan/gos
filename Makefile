GNU = riscv64-unknown-linux-gnu-

TOPDIR := $(shell pwd)

CC := $(GNU)gcc
LD := $(GNU)ld
RISCV_COPY := $(GNU)objcopy
RISCV_COPY_FLAGS := --set-section-flags .bss=alloc,contents --set-section-flags .sbss=alloc,contents -O binary
RISCV_DUMP := $(GNU)objdump

#DEBUG := -DUSE_QEMU
DEBUG := -DUSE_FPGA
#DEBUG := -DUSE_ST_VCS
#DEBUG := -DUSE_ST_CMN600

#DEBUG += -DUSE_AIA
#DEBUG += -DIOMMU_PTWALK_TEST
DEBUG += -DENABLE_MMU
DEBUG += -DENABLE_MULTI_TASK
#DEBUG += -DNO_SHELL

COPS := -g -O0 -Wall -nostdlib -mcmodel=medany -mabi=lp64d -march=rv64imafdc -fno-PIE -fomit-frame-pointer -Wno-builtin-declaration-mismatch

export TOPDIR
export CC
export LD
export COPS
export DEBUG

BUILD_DIR = build
MYSBI_DIR = mysbi
BSP_DIR   = bsp
MYGUEST_DIR = myGuest
MYUSER_DIR = myUser

default: clean dtb mysbi myUser myGuest gos pack.sh
	./pack.sh

fpga:
	./bin2fpgadata -i out/Image.bin

#build user
USER_TARGET := myUser.elf
USER_TARGET_BIN := myUser.bin

MYUSER_ENTRY_DIR = $(MYUSER_DIR)/entry
MYUSER_CORE_DIR = $(MYUSER_DIR)/core
MYUSER_LIB_DIR = $(MYUSER_DIR)/lib
MYUSER_COMMAND_DIR = $(MYUSER_DIR)/command

USER_ENTRY_C_FILES = $(wildcard $(MYUSER_ENTRY_DIR)/*.c)
USER_ENTRY_ASM_FILES = $(wildcard $(MYUSER_ENTRY_DIR)/*.S)
USER_CORE_C_FILES = $(wildcard $(MYUSER_CORE_DIR)/*.c)
USER_CORE_ASM_FILES = $(wildcard $(MYUSER_CORE_DIR)/*.S)
USER_LIB_C_FILES = $(wildcard $(MYUSER_LIB_DIR)/*.c)
USER_COMMAND_C_FILES = $(wildcard $(MYUSER_COMMAND_DIR)/*.c)

MYUSER_OBJ_FILES = $(USER_ENTRY_ASM_FILES:$(MYUSER_ENTRY_DIR)/%.S=$(MYUSER_ENTRY_DIR)/%_s.o)
MYUSER_OBJ_FILES += $(USER_ENTRY_C_FILES:$(MYUSER_ENTRY_DIR)/%.c=$(MYUSER_ENTRY_DIR)/%_c.o)
MYUSER_OBJ_FILES += $(USER_CORE_ASM_FILES:$(MYUSER_CORE_DIR)/%.S=$(MYUSER_CORE_DIR)/%_s.o)
MYUSER_OBJ_FILES += $(USER_CORE_C_FILES:$(MYUSER_CORE_DIR)/%.c=$(MYUSER_CORE_DIR)/%_c.o)
MYUSER_OBJ_FILES += $(USER_LIB_C_FILES:$(MYUSER_LIB_DIR)/%.c=$(MYUSER_LIB_DIR)/%_c.o)
MYUSER_OBJ_FILES += $(USER_COMMAND_C_FILES:$(MYUSER_COMMAND_DIR)/%.c=$(MYUSER_COMMAND_DIR)/%_c.o)

myUser: $(MYUSER_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(LD) -T $(MYUSER_DIR)/myUser.lds -o $(BUILD_DIR)/$(USER_TARGET) $(MYUSER_OBJ_FILES) -Map $(BUILD_DIR)/myUser.map
	$(RISCV_COPY) $(BUILD_DIR)/$(USER_TARGET) -O binary $(BUILD_DIR)/$(USER_TARGET_BIN)

$(MYUSER_ENTRY_DIR)/%_s.o: $(MYUSER_ENTRY_DIR)/%.S
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_ENTRY_DIR)/%_c.o: $(MYUSER_ENTRY_DIR)/%.c
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_CORE_DIR)/%_s.o: $(MYUSER_CORE_DIR)/%.S
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_CORE_DIR)/%_c.o: $(MYUSER_CORE_DIR)/%.c
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_LIB_DIR)/%_c.o: $(MYUSER_LIB_DIR)/%.c
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_COMMAND_DIR)/%_c.o: $(MYUSER_COMMAND_DIR)/%.c
	$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

#build guest
GUEST_TARGET := myGuest.elf
GUEST_TARGET_BIN := myGuest.bin

MYGUEST_ENTRY_DIR = $(MYGUEST_DIR)/entry
MYGUEST_CORE_DIR = $(MYGUEST_DIR)/core
MYGUEST_DRIVERS_DIR = $(MYGUEST_DIR)/drivers
MYGUEST_LIB_DIR = $(MYGUEST_DIR)/lib
MYGUEST_MM_DIR = $(MYGUEST_DIR)/mm
MYGUEST_COMMAND_DIR = $(MYGUEST_DIR)/command

GUEST_ENTRY_C_FILES = $(wildcard $(MYGUEST_ENTRY_DIR)/*.c)
GUEST_ENTRY_ASM_FILES = $(wildcard $(MYGUEST_ENTRY_DIR)/*.S)
GUEST_CORE_C_FILES = $(wildcard $(MYGUEST_CORE_DIR)/*.c)
GUEST_DRIVERS_C_FILES = $(wildcard $(MYGUEST_DRIVERS_DIR)/*.c)
GUEST_LIB_C_FILES = $(wildcard $(MYGUEST_LIB_DIR)/*.c)
GUEST_MM_C_FILES = $(wildcard $(MYGUEST_MM_DIR)/*.c)
GUEST_COMMAND_C_FILES = $(wildcard $(MYGUEST_COMMAND_DIR)/*.c)

MYGUEST_OBJ_FILES = $(GUEST_ENTRY_ASM_FILES:$(MYGUEST_ENTRY_DIR)/%.S=$(MYGUEST_ENTRY_DIR)/%_s.o)
MYGUEST_OBJ_FILES += $(GUEST_ENTRY_C_FILES:$(MYGUEST_ENTRY_DIR)/%.c=$(MYGUEST_ENTRY_DIR)/%_c.o)
MYGUEST_OBJ_FILES += $(GUEST_LIB_C_FILES:$(MYGUEST_LIB_DIR)/%.c=$(MYGUEST_LIB_DIR)/%_c.o)
MYGUEST_OBJ_FILES += $(GUEST_CORE_C_FILES:$(MYGUEST_CORE_DIR)/%.c=$(MYGUEST_CORE_DIR)/%_c.o)
MYGUEST_OBJ_FILES += $(GUEST_DRIVERS_C_FILES:$(MYGUEST_DRIVERS_DIR)/%.c=$(MYGUEST_DRIVERS_DIR)/%_c.o)
MYGUEST_OBJ_FILES += $(GUEST_MM_C_FILES:$(MYGUEST_MM_DIR)/%.c=$(MYGUEST_MM_DIR)/%_c.o)
MYGUEST_OBJ_FILES += $(GUEST_COMMAND_C_FILES:$(MYGUEST_COMMAND_DIR)/%.c=$(MYGUEST_COMMAND_DIR)/%_c.o)

myGuest: $(MYGUEST_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(LD) -T $(MYGUEST_DIR)/myGuest.lds -o $(BUILD_DIR)/$(GUEST_TARGET) $(MYGUEST_OBJ_FILES) -Map $(BUILD_DIR)/myGuest.map
	$(RISCV_COPY) $(BUILD_DIR)/$(GUEST_TARGET) -O binary $(BUILD_DIR)/$(GUEST_TARGET_BIN)

$(MYGUEST_ENTRY_DIR)/%_s.o: $(MYGUEST_ENTRY_DIR)/%.S
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_ENTRY_DIR)/%_c.o: $(MYGUEST_ENTRY_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_CORE_DIR)/%_c.o: $(MYGUEST_CORE_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_DRIVERS_DIR)/%_c.o: $(MYGUEST_DRIVERS_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_LIB_DIR)/%_c.o: $(MYGUEST_LIB_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_MM_DIR)/%_c.o: $(MYGUEST_MM_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

$(MYGUEST_COMMAND_DIR)/%_c.o: $(MYGUEST_COMMAND_DIR)/%.c
	$(CC) $(COPS) -I$(MYGUEST_DIR)/include -c $< -o $@

# build sbi
SBI_TARGET := mysbi.elf
SBI_TARGET_BIN := mysbi.bin

MYSBI_ENTRY_DIR = $(MYSBI_DIR)/entry
MYSBI_SBI_DIR   = $(MYSBI_DIR)/sbi
MYSBI_LIB_DIR   = $(MYSBI_DIR)/lib

SBI_C_FILES = $(wildcard $(MYSBI_SBI_DIR)/*.c)
SBI_ASM_FILES = $(wildcard $(MYSBI_SBI_DIR)/*.S)
SBI_ENTRY_C_FILES = $(wildcard $(MYSBI_ENTRY_DIR)/*.c)
SBI_ENTRY_ASM_FILES = $(wildcard $(MYSBI_ENTRY_DIR)/*.S)
SBI_LIB_C_FILES = $(wildcard $(MYSBI_LIB_DIR)/*.c)
SBI_BSP_C_FILES = $(wildcard $(BSP_DIR)/*.c)

MYSBI_OBJ_FILES = $(SBI_ENTRY_ASM_FILES:$(MYSBI_ENTRY_DIR)/%.S=$(MYSBI_ENTRY_DIR)/%_s.o)
MYSBI_OBJ_FILES += $(SBI_ENTRY_C_FILES:$(MYSBI_ENTRY_DIR)/%.c=$(MYSBI_ENTRY_DIR)/%_c.o)
MYSBI_OBJ_FILES += $(SBI_C_FILES:$(MYSBI_SBI_DIR)/%.c=$(MYSBI_SBI_DIR)/%_c.o)
MYSBI_OBJ_FILES += $(SBI_ASM_FILES:$(MYSBI_SBI_DIR)/%.S=$(MYSBI_SBI_DIR)/%_s.o)
MYSBI_OBJ_FILES += $(SBI_BSP_C_FILES:$(BSP_DIR)/%.c=$(BSP_DIR)/%_c.o)
MYSBI_OBJ_FILES += $(ENTRY_C_FILES:$(MYSBI_ENTRY_DIR)/%.c=$(MYSBI_ENTRY_DIR)/%_c.o)
MYSBI_OBJ_FILES += $(SBI_LIB_C_FILES:$(MYSBI_LIB_DIR)/%.c=$(MYSBI_LIB_DIR)/%_c.o)

mysbi: $(MYSBI_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(LD) -T $(MYSBI_DIR)/mysbi.lds -o $(BUILD_DIR)/$(SBI_TARGET) $(MYSBI_OBJ_FILES) -Map $(BUILD_DIR)/mysbi.map
	$(RISCV_COPY) $(BUILD_DIR)/$(SBI_TARGET) -O binary $(BUILD_DIR)/$(SBI_TARGET_BIN)
	
$(MYSBI_ENTRY_DIR)/%_s.o: $(MYSBI_ENTRY_DIR)/%.S
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_ENTRY_DIR)/%_c.o: $(MYSBI_ENTRY_DIR)/%.c
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_SBI_DIR)/%_s.o: $(MYSBI_SBI_DIR)/%.S
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_SBI_DIR)/%_c.o: $(MYSBI_SBI_DIR)/%.c
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_LIB_DIR)/%_c.o: $(MYSBI_LIB_DIR)/%.c
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(BSP_DIR)/%_c.o: $(BSP_DIR)/%.c
	$(CC) $(COPS) -I$(MYSBI_DIR)/include $(DEBUG) -c $< -o $@

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

gos: $(GOS_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	make -C ./ -f $(TOPDIR)/Makefile.build
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

clean:
	rm -rf $(shell find -name "*.o")
	rm -rf $(BUILD_DIR)
	rm -rf out
dtb:
	mkdir -p build
	dtc -O dtb -I dts -o build/input.dtb input.dts
	
run:
	./qemu-system-riscv64 -nographic \
        -machine virt -smp 2 \
	-cpu rv64,sv39=on -m 8G \
        -bios out/Image.bin \

run-aia:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on -m 8G \
        -bios out/Image.bin \

run-debug:
	./qemu-system-riscv64 -nographic \
        -machine virt -smp 2 \
	-cpu rv64,sv39=on -m 8G \
        -bios out/Image.bin \
	-S -s

run-aia-debug:
	./qemu-system-riscv64 -nographic \
        -machine virt,aia=aplic-imsic,aia-guests=7 -smp 4 \
	-cpu rv64,sv39=on -m 8G \
        -bios out/Image.bin \
	-S -s

run-nemu:
	./riscv64-nemu-interpreter -b out/Image.bin

format:
	find . -name *.c |xargs ./Lindent
	find . -name *.h |xargs ./Lindent
	find . -name *.c~ |xargs rm
	find . -name *.h~ |xargs rm
