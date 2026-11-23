DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
LIBGBA ?= $(DEVKITPRO)/libgba
PREFIX := $(DEVKITARM)/bin/arm-none-eabi-

GBA_CC := $(PREFIX)gcc
GBA_AS := $(PREFIX)as
GBA_OBJCOPY := $(PREFIX)objcopy
MMUTIL ?= $(DEVKITPRO)/tools/bin/mmutil
BIN2S ?= $(DEVKITPRO)/tools/bin/bin2s

TARGET := game
VIDEO_MODE ?= 4
ASSET_DIR := assets
ASSET_BUILD_DIR := build/assets
ASSET_PNGS := $(wildcard $(ASSET_DIR)/*.png)
ASSET_SOURCES := $(patsubst $(ASSET_DIR)/%.png,$(ASSET_BUILD_DIR)/%.c,$(ASSET_PNGS))
ASSET_HEADERS := $(ASSET_SOURCES:.c=.h)
ASSET_OBJECTS := $(ASSET_SOURCES:.c=.o)

AUDIO_DIR := $(ASSET_DIR)/audio
AUDIO_BUILD_DIR := build/audio
AUDIO_FILES := $(sort \
	$(wildcard $(AUDIO_DIR)/*.mod) \
	$(wildcard $(AUDIO_DIR)/*.s3m) \
	$(wildcard $(AUDIO_DIR)/*.xm) \
	$(wildcard $(AUDIO_DIR)/*.it) \
	$(wildcard $(AUDIO_DIR)/*.wav))
SOUNDBANK_BIN := $(AUDIO_BUILD_DIR)/soundbank.bin
SOUNDBANK_HEADER := $(AUDIO_BUILD_DIR)/soundbank.h
SOUNDBANK_ASM := $(AUDIO_BUILD_DIR)/soundbank_bin.s
SOUNDBANK_BIN_HEADER := $(AUDIO_BUILD_DIR)/soundbank_bin.h
SOUNDBANK_OBJECT := $(AUDIO_BUILD_DIR)/soundbank_bin.o

C_SOURCES := lamadil.c input.c audio.c sin_table.c newtest.c
OBJECTS := crt0.o $(C_SOURCES:.c=.o) $(ASSET_OBJECTS) $(SOUNDBANK_OBJECT)

CFLAGS := -O3 -std=c11 -mcpu=arm7tdmi -mthumb -mthumb-interwork \
	-ffreestanding -fno-builtin -fomit-frame-pointer -fno-common \
	-Wall -Wextra -Wpedantic -I$(ASSET_BUILD_DIR) \
	-I$(AUDIO_BUILD_DIR) -I$(LIBGBA)/include \
	-DLAMADIL_VIDEO_MODE=$(VIDEO_MODE)
LDFLAGS := -mcpu=arm7tdmi -mthumb-interwork -nostartfiles \
	-Wl,-T,lnkscript -Wl,-Map,$(TARGET).map -L$(LIBGBA)/lib
LIBS := -lmm

.PHONY: all clean run host-test mode4 mode5
.SECONDARY: $(ASSET_SOURCES) $(ASSET_HEADERS) $(SOUNDBANK_BIN) \
	$(SOUNDBANK_HEADER) $(SOUNDBANK_ASM) $(SOUNDBANK_BIN_HEADER)

all: $(TARGET).gba

mode4:
	$(MAKE) clean
	$(MAKE) VIDEO_MODE=4 all

mode5:
	$(MAKE) clean
	$(MAKE) VIDEO_MODE=5 all

$(TARGET).gba: $(TARGET).elf ht.pl
	$(GBA_OBJCOPY) -O binary $< $@
	perl ht.pl -cl $@

$(TARGET).elf: $(OBJECTS) lnkscript
	$(GBA_CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

crt0.o: crt0.s
	$(GBA_AS) -mcpu=arm7tdmi -o $@ $<

%.o: %.c lamadil.h
	$(GBA_CC) $(CFLAGS) -c -o $@ $<

$(ASSET_BUILD_DIR)/%.c: $(ASSET_DIR)/%.png
	mkdir -p $(ASSET_BUILD_DIR)
	grit $< -gb -gB16 -gu16 -p! -m! -ftc -o$@

$(ASSET_BUILD_DIR)/%.h: $(ASSET_BUILD_DIR)/%.c
	@test -f $@

$(ASSET_BUILD_DIR)/%.o: $(ASSET_BUILD_DIR)/%.c $(ASSET_BUILD_DIR)/%.h
	$(GBA_CC) $(CFLAGS) -c -o $@ $<

$(SOUNDBANK_BIN): $(AUDIO_FILES)
	mkdir -p $(AUDIO_BUILD_DIR)
	$(MMUTIL) $^ -o$@ -h$(SOUNDBANK_HEADER)

$(SOUNDBANK_HEADER): $(SOUNDBANK_BIN)
	@test -f $@

$(SOUNDBANK_ASM): $(SOUNDBANK_BIN)
	$(BIN2S) -H$(SOUNDBANK_BIN_HEADER) $< > $@
	@printf '\n' >> $@

$(SOUNDBANK_BIN_HEADER): $(SOUNDBANK_ASM)
	@test -f $@

$(SOUNDBANK_OBJECT): $(SOUNDBANK_ASM)
	$(GBA_CC) -mcpu=arm7tdmi -c -o $@ $<

newtest.o: assets/assets.h $(ASSET_HEADERS) $(SOUNDBANK_HEADER) \
	$(SOUNDBANK_BIN_HEADER)

run: $(TARGET).gba
	mgba $(TARGET).gba

host-test:
	$(CC) -std=c11 -O2 -Wall -Wextra -Wpedantic \
		-DLAMADIL_VIDEO_MODE=4 -I. \
		lamadil.c sin_table.c tests/render_test.c -o render_test_mode4
	./render_test_mode4
	$(CC) -std=c11 -O2 -Wall -Wextra -Wpedantic \
		-DLAMADIL_VIDEO_MODE=5 -I. \
		lamadil.c sin_table.c tests/render_test.c -o render_test_mode5
	./render_test_mode5

clean:
	rm -f crt0.o $(C_SOURCES:.c=.o) $(TARGET).elf $(TARGET).gba \
		$(TARGET).map render_test render_test_mode4 render_test_mode5
	rm -rf build
