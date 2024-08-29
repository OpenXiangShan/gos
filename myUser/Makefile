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

#build user

BUILD_DIR = $(TOPDIR)/build
MYUSER_DIR := $(shell pwd)

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
	@mkdir -p $(BUILD_DIR)
	@$(LD) -T $(MYUSER_DIR)/myUser.lds -o $(BUILD_DIR)/$(USER_TARGET) $(MYUSER_OBJ_FILES) -Map $(BUILD_DIR)/myUser.map --no-warn-rwx-segments
	@$(RISCV_COPY) $(BUILD_DIR)/$(USER_TARGET) -O binary $(BUILD_DIR)/$(USER_TARGET_BIN)

$(MYUSER_ENTRY_DIR)/%_s.o: $(MYUSER_ENTRY_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_ENTRY_DIR)/%_c.o: $(MYUSER_ENTRY_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_CORE_DIR)/%_s.o: $(MYUSER_CORE_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_CORE_DIR)/%_c.o: $(MYUSER_CORE_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_LIB_DIR)/%_c.o: $(MYUSER_LIB_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

$(MYUSER_COMMAND_DIR)/%_c.o: $(MYUSER_COMMAND_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYUSER_DIR)/include -I$(TOPDIR)/include/uapi -c $< -o $@

clean:
	@rm -rf $(shell find -name "*.o")
