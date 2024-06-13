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
	$(CC) $(COPS) -I$(MYSBI_DIR)/include -I$(TOPDIR)/include/gos $(DEBUG) -c $< -o $@

clean:
	rm -rf $(shell find -name "*.o")
