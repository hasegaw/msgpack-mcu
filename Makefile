CC ?= gcc
RM := rm -rf
MKDIR := mkdir -p

CFLAGS  = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
# CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
# CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmissing-declarations
CFLAGS += -Wno-unused-parameter
# CFLAGS += -Wfloat-equal
CFLAGS += -pedantic-errors
CFLAGS += -g -O0
# CFLAGS += -O3 -flto
CFLAGS += -MMD -MP
LDFLGAS := 

BUILD_DIR  = _build
SOURCE_DIR = src
TEST_DIR   = test

INCLUDES  = -Iinclude
INCLUDES += -Itest

SOURCES  = $(SOURCE_DIR)/umsgpack.c
TEST_SOURCES  = $(TEST_DIR)/umsgpack_test.c

OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))

# OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))
DEPENDS = $(OBJECTS:.o=.d)

TARGET = umsgpack_test

.PHONY: test clean

test: $(OBJECTS)
	$(CC) $(LDFLGAS) -o $(BUILD_DIR)/$(TARGET) $(OBJECTS)

$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%.c
	@[ -d $(BUILD_DIR) ] || $(MKDIR) $(BUILD_DIR)
	@echo { $@ } [ $< ] 
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(BUILD_DIR)/%.o:$(TEST_DIR)/%.c
	@[ -d $(BUILD_DIR) ] || $(MKDIR) $(BUILD_DIR)
	@echo { $@ } [ $< ] 
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	$(RM) $(BUILD_DIR)

-include $(DEPENDS)
