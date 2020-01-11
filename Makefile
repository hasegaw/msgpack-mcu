#
# MessagePack for MCUs
# ====================
#
#  The MIT License (MIT)
#
#  Copyright (c) 2015-2016 ryochack
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

CC ?= gcc
RM := rm -rf
MKDIR := mkdir -p

CFLAGS  = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
# CFLAGS += -Werror
CFLAGS += -Wpointer-arith
# CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
# CFLAGS += -Wstrict-prototypes
# CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
# CFLAGS += -Wmissing-prototypes
# CFLAGS += -Wmissing-declarations
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-function
# CFLAGS += -Wfloat-equal
CFLAGS += -Wno-unreachable-code
CFLAGS += -Wno-old-style-definition
CFLAGS += -Wno-overflow
CFLAGS += -Wimplicit-function-declaration
CFLAGS += -pedantic-errors
CFLAGS += -g -O0
# CFLAGS += -O3 -flto
CFLAGS += -MMD -MP
LDFLGAS := 

DEFINES  = -DUMSGPACK_FUNC_INT16
DEFINES += -DUMSGPACK_FUNC_INT32
DEFINES += -DUMSGPACK_FUNC_INT64
DEFINES += -DUMSGPACK_LITTLE_ENDIAN

BUILD_DIR  = _build
SOURCE_DIR = $(CURDIR)
TEST_DIR   = test

INCLUDES  = -I$(CURDIR)
INCLUDES += -Itest

SOURCES  = $(SOURCE_DIR)/umsgpack.c
TEST_SOURCES  = $(TEST_DIR)/umsgpack_test.c

UNITTEST_FRAMEWORK := minunit
UNITTEST_FRAMEWORK_GIT_URL := https://github.com/siu/minunit.git
UNITTEST_FRAMEWORK_GIT_CONFIG := $(TEST_DIR)/$(UNITTEST_FRAMEWORK)/.git/config
UNITTEST_FRAMEWORK_PATCH := patches/minunit.patch

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
	patch -p0 < $(UNITTEST_FRAMEWORK_PATCH)

$(BUILD_DIR):
	@[ -d $(BUILD_DIR) ] || $(MKDIR) $(BUILD_DIR)

$(BUILD_DIR)/%.o:$(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ -c $<

$(BUILD_DIR)/%.o:$(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ -c $<

clean:
	$(RM) $(BUILD_DIR)

-include $(DEPENDS)
