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

#build guest

BUILD_DIR = $(TOPDIR)/build
#INCLUDE_DIR = $(TOPDIR)/include
CORE_INCLUDE_DIR = $(TOPDIR)/core
USERINCLUDE	:= \
		-I$(CORE_INCLUDE_DIR)/include \
		-I$(TOPDIR)/include

MYGUEST_DIR := $(shell pwd)
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
	@mkdir -p $(BUILD_DIR)
	@$(LD) -T $(MYGUEST_DIR)/myGuest.lds --defsym $(KBUILD_MYGUEST_CFLAGS) -o $(BUILD_DIR)/$(GUEST_TARGET) $(MYGUEST_OBJ_FILES) -Map $(BUILD_DIR)/myGuest.map --no-warn-rwx-segments
	@$(RISCV_COPY) $(BUILD_DIR)/$(GUEST_TARGET) -O binary $(BUILD_DIR)/$(GUEST_TARGET_BIN)

$(MYGUEST_ENTRY_DIR)/%_s.o: $(MYGUEST_ENTRY_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include $(USERINCLUDE) -c $< -o $@

$(MYGUEST_ENTRY_DIR)/%_c.o: $(MYGUEST_ENTRY_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

$(MYGUEST_CORE_DIR)/%_c.o: $(MYGUEST_CORE_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

$(MYGUEST_DRIVERS_DIR)/%_c.o: $(MYGUEST_DRIVERS_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

$(MYGUEST_LIB_DIR)/%_c.o: $(MYGUEST_LIB_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

$(MYGUEST_MM_DIR)/%_c.o: $(MYGUEST_MM_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

$(MYGUEST_COMMAND_DIR)/%_c.o: $(MYGUEST_COMMAND_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYGUEST_DIR)/include -I$(TOPDIR)/include/generic $(USERINCLUDE) -c $< -o $@

clean:
	@rm -rf $(shell find -name "*.o")
