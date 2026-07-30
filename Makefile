DEVKITARM ?=
PREFIX := $(if $(DEVKITARM),$(DEVKITARM)/bin/,)arm-none-eabi-

GBA_CC := $(PREFIX)gcc
GBA_AS := $(PREFIX)as
GBA_OBJCOPY := $(PREFIX)objcopy

TARGET := game
ASSET_DIR := assets
ASSET_BUILD_DIR := build/assets
ASSET_PNGS := $(wildcard $(ASSET_DIR)/*.png)
ASSET_SOURCES := $(patsubst $(ASSET_DIR)/%.png,$(ASSET_BUILD_DIR)/%.c,$(ASSET_PNGS))
ASSET_HEADERS := $(ASSET_SOURCES:.c=.h)
ASSET_OBJECTS := $(ASSET_SOURCES:.c=.o)

C_SOURCES := lamadil.c input.c audio.c sin_table.c newtest.c
OBJECTS := crt0.o $(C_SOURCES:.c=.o) $(ASSET_OBJECTS)

CFLAGS := -O3 -std=c11 -mcpu=arm7tdmi -mthumb -mthumb-interwork \
	-ffreestanding -fno-builtin -fomit-frame-pointer -fno-common \
	-Wall -Wextra -Wpedantic -I$(ASSET_BUILD_DIR)
LDFLAGS := -mcpu=arm7tdmi -mthumb-interwork -nostartfiles \
	-Wl,-T,lnkscript -Wl,-Map,$(TARGET).map

.PHONY: all clean run host-test
.SECONDARY: $(ASSET_SOURCES) $(ASSET_HEADERS)

all: $(TARGET).gba

$(TARGET).gba: $(TARGET).elf ht.pl
	$(GBA_OBJCOPY) -O binary $< $@
	perl ht.pl -cl $@

$(TARGET).elf: $(OBJECTS) lnkscript
	$(GBA_CC) $(LDFLAGS) -o $@ $(OBJECTS)

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

newtest.o: assets/assets.h $(ASSET_HEADERS)

run: $(TARGET).gba
	mgba $(TARGET).gba

host-test:
	$(CC) -std=c11 -O2 -Wall -Wextra -Wpedantic \
		-I. lamadil.c sin_table.c tests/render_test.c \
		-o render_test
	./render_test

clean:
	rm -f crt0.o $(C_SOURCES:.c=.o) $(TARGET).elf $(TARGET).gba \
		$(TARGET).map render_test
	rm -rf build
