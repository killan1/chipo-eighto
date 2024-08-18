TARGET_DIR=target
DEBUG_DIR=$(TARGET_DIR)/debug
RELEASE_DIR=$(TARGET_DIR)/release

BASE_CFLAGS=--std=c99
DEBUG_CFLAGS=$(BASE_CFLAGS) -g3 -Wall -Wextra -Wpedantic -fsanitize=address,undefined
RELEASE_CFLAGS=-O3

ifeq ($(CHIP_BACKEND),super-chip)
	CHIP = super-chip.c
else
	CHIP = chip.c
endif

VPATH = src
LIBS = -lraylib -lm
BUILD_CC = $(CC) $(CLFAGS) -o $@ -c $<

TARGET=$(BUILD_DIR)/bin/chipo8o

debug:
	mkdir	-p $(DEBUG_DIR)/bin
	$(MAKE) target BUILD_DIR=$(DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

release:
	mkdir	-p $(RELEASE_DIR)/bin
	$(MAKE) target BUILD_DIR=$(RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

target: $(TARGET)

all: debug release

OBJECTS = \
					$(BUILD_DIR)/chipo-eighto.o \
					$(BUILD_DIR)/chip.o \
					$(BUILD_DIR)/media.o \
					$(BUILD_DIR)/utils.o \
					$(BUILD_DIR)/sys.o \
					$(BUILD_DIR)/args.o \
					$(BUILD_DIR)/config.o

$(BUILD_DIR)/bin/chipo8o: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)
$(BUILD_DIR)/chipo-eighto.o: chipo-eighto.c
	$(BUILD_CC)
$(BUILD_DIR)/chip.o: chip.c
	$(BUILD_CC)
$(BUILD_DIR)/media.o: media.c
	$(BUILD_CC)
$(BUILD_DIR)/utils.o: utils.c
	$(BUILD_CC)
$(BUILD_DIR)/sys.o: sys.c
	$(BUILD_CC)
$(BUILD_DIR)/args.o: args.c
	$(BUILD_CC)
$(BUILD_DIR)/config.o: config.c
	$(BUILD_CC)

clean: clean-debug clean-release

clean-debug:
	$(MAKE) do-clean BUILD_DIR=$(DEBUG_DIR)

clean-release:
	$(MAKE) do-clean BUILD_DIR=$(RELEASE_DIR)

do-clean:
	-rm -f $(OBJECTS)
