SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

TARGET = $(BIN_DIR)/firmware
CPU = -mcpu=cortex-m0

SRC += $(wildcard $(SRC_DIR)/driver/*.cpp)
SRC += $(wildcard $(SRC_DIR)/helpers/*.cpp)
SRC += $(wildcard $(SRC_DIR)/svc/*.cpp)
SRC += $(wildcard $(SRC_DIR)/ui/*.cpp)
SRC += $(wildcard $(SRC_DIR)/apps/*.cpp)
SRC += $(wildcard $(SRC_DIR)/*.cpp)

OBJS =
OBJS += $(OBJ_DIR)/start.o
OBJS += $(OBJ_DIR)/init.o
OBJS += $(OBJ_DIR)/printf.o

OBJS += $(filter %.o,$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o))

BSP_DEFINITIONS := $(wildcard hardware/*/*.def)
BSP_HEADERS := $(patsubst hardware/%,inc/%,$(BSP_DEFINITIONS))
BSP_HEADERS := $(patsubst %.def,%.h,$(BSP_HEADERS))

AS = arm-none-eabi-g++
CC = arm-none-eabi-gcc
CPP = arm-none-eabi-g++
LD = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# GIT_HASH := $(shell git rev-parse --short HEAD)

ASFLAGS = $(CPU)
CFLAGS_COMMON = $(CPU) -Os -MMD
CFLAGS_COMMON += -Wall -Wextra -Wno-error -fpermissive
CFLAGS_COMMON += -fshort-enums -fno-exceptions -ffunction-sections -fdata-sections -fno-builtin -fno-delete-null-pointer-checks
CFLAGS_COMMON += -DPRINTF_USER_DEFINED_PUTCHAR
CFLAGS_COMMON += -DPRINTF_DISABLE_SUPPORT_LONG_LONG
CFLAGS_COMMON += -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL
CFLAGS_COMMON += -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T
CFLAGS_COMMON += -DPRINTF_DISABLE_SUPPORT_FLOAT
# CFLAGS_COMMON += -DGIT_HASH=\"$(GIT_HASH)\"

CFLAGS = $(CFLAGS_COMMON) -std=c2x
CPPFLAGS = $(CFLAGS_COMMON) -std=c++20
LDFLAGS = $(CPU) -nostartfiles -Wl,-T,firmware.ld

INC =
INC += -I ./src
INC += -I ./src/external/CMSIS_5/CMSIS/Core/Include/
INC += -I ./src/external/CMSIS_5/Device/ARM/ARMCM0/Include

DEPS = $(OBJS:.o=.d)

.PHONY: all clean

all: $(TARGET)
	$(OBJCOPY) -O binary $< $<.bin
	-python fw-pack.py $<.bin $(GIT_HASH) $<.packed.bin
	-python3 fw-pack.py $<.bin $(GIT_HASH) $<.packed.bin
	$(SIZE) $<

version.o: .FORCE

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(LD) $(LDFLAGS) $^ -o $@

inc/dp32g030/%.h: hardware/dp32g030/%.def

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BSP_HEADERS) $(OBJ_DIR)
	mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) $(INC) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(BSP_HEADERS) $(OBJ_DIR)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S | $(OBJ_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

.FORCE:

-include $(DEPS)

clean:
	rm -f $(TARGET).bin $(TARGET).packed.bin $(TARGET) $(OBJS) $(DEPS)
