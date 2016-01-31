CC ?= gcc
RM := rm -rf
MKDIR := mkdir -p

CFLAGS  = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -Wpointer-arith
# CFLAGS += -Wcast-align
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

UNITTEST_FRAMEWORK := minunit
UNITTEST_FRAMEWORK_GIT_URL := https://github.com/siu/minunit.git
UNITTEST_FRAMEWORK_GIT_CONFIG := $(TEST_DIR)/$(UNITTEST_FRAMEWORK)/.git/config

OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(TEST_SOURCES:.c=.o)))

# OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))
DEPENDS = $(OBJECTS:.o=.d)

TARGET = umsgpack_test

.PHONY: test clean

all: $(UNITTEST_FRAMEWORK_GIT_CONFIG) $(BUILD_DIR) $(OBJECTS)
	$(CC) $(LDFLGAS) -o $(BUILD_DIR)/$(TARGET) $(OBJECTS)

test: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET)

$(UNITTEST_FRAMEWORK_GIT_CONFIG):
	git clone $(UNITTEST_FRAMEWORK_GIT_URL) $(TEST_DIR)/$(UNITTEST_FRAMEWORK)

$(BUILD_DIR):
	@[ -d $(BUILD_DIR) ] || $(MKDIR) $(BUILD_DIR)

$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(BUILD_DIR)/%.o:$(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	$(RM) $(BUILD_DIR)

-include $(DEPENDS)
