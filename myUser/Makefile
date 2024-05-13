#build user

include $(TOPDIR)/include/config/auto.conf

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

ifeq ($(CONFIG_MYUSER), y)
myUser: $(MYUSER_OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(LD) -T $(MYUSER_DIR)/myUser.lds -o $(BUILD_DIR)/$(USER_TARGET) $(MYUSER_OBJ_FILES) -Map $(BUILD_DIR)/myUser.map
	$(RISCV_COPY) $(BUILD_DIR)/$(USER_TARGET) -O binary $(BUILD_DIR)/$(USER_TARGET_BIN)
else
myUser:
	cp $(TOPDIR)/myUser.bin $(TOPDIR)/build/myUser.bin
endif

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

clean:
	rm -rf $(shell find -name "*.o")
