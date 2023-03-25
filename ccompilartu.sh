# Exit on error
set -e

arm-none-eabi-as   -o crt0.o crt0.s
arm-none-eabi-gcc -O3 -fomit-frame-pointer -marm -mcpu=arm7tdmi -pedantic -Wall -c -o lamadil.o newtest.c -mthumb
arm-none-eabi-gcc -o game.elf crt0.o lamadil.o -Tlnkscript -nostartfiles -lm -Xlinker -Map=memory.map 
arm-none-eabi-objcopy -O binary game.elf game.gba
rm -f game.elf crt0.o lamadil.o
perl ht.pl -cl game.gba

mgba game.gba