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

# build sbi

BUILD_DIR = $(TOPDIR)/build
MYSBI_DIR := $(shell pwd)

SBI_TARGET := mysbi.elf
SBI_TARGET_BIN := mysbi.bin

MYSBI_ENTRY_DIR = $(MYSBI_DIR)/entry
MYSBI_SBI_DIR   = $(MYSBI_DIR)/sbi
MYSBI_LIB_DIR   = $(MYSBI_DIR)/lib
BSP_DIR         = $(TOPDIR)/bsp

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
	@mkdir -p $(BUILD_DIR)
	$(LD) -T $(MYSBI_DIR)/mysbi.lds --defsym $(KBUILD_CFLAGS) -o $(BUILD_DIR)/$(SBI_TARGET) $(MYSBI_OBJ_FILES) -Map $(BUILD_DIR)/mysbi.map --no-warn-rwx-segments
	@$(RISCV_COPY) $(BUILD_DIR)/$(SBI_TARGET) -O binary $(BUILD_DIR)/$(SBI_TARGET_BIN)

$(MYSBI_ENTRY_DIR)/%_s.o: $(MYSBI_ENTRY_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_ENTRY_DIR)/%_c.o: $(MYSBI_ENTRY_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -I$(TOPDIR)/include/gos-auto -c $< -o $@

$(MYSBI_SBI_DIR)/%_s.o: $(MYSBI_SBI_DIR)/%.S
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(MYSBI_SBI_DIR)/%_c.o: $(MYSBI_SBI_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -I$(MYSBI_DIR)/../include/gos-auto -c $< -o $@

$(MYSBI_LIB_DIR)/%_c.o: $(MYSBI_LIB_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -c $< -o $@

$(BSP_DIR)/%_c.o: $(BSP_DIR)/%.c
	@echo "CC $@"
	@$(CC) $(COPS) -I$(MYSBI_DIR)/include -I$(TOPDIR)/include/gos $(DEBUG) -c $< -o $@

clean:
	@rm -rf $(shell find -name "*.o")
