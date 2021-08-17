
ARMGNU ?= arm-none-eabi
#ARMGNU ?= arm-linux-gnueabi

AOPS = --warn --fatal-warnings -mcpu=arm1176jzf-s -march=armv6 -mfpu=vfp
COPS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mhard-float -mfpu=vfp -lm -lc 

all : kernel.img

clean :
	rm -f build/*.o
	rm -f build/*.bin
	rm -f build/*.srec
	rm -f build/*.elf
	rm -f build/*.list
	rm -f build/*.img

start.o : src/start.s
	$(ARMGNU)-as $(AOPS) src/start.s -o build/start.o

vvvf_main.o : src/vvvf_main.c
	$(ARMGNU)-gcc $(COPS) -c src/vvvf_main.c -o build/vvvf_main.o
	
vvvf_wave.o : src/vvvf_wave.c
	$(ARMGNU)-gcc $(COPS) -c src/vvvf_wave.c -o build/vvvf_wave.o

gpio.o : src/rpi_lib/gpio.c
	$(ARMGNU)-gcc $(COPS) -c src/rpi_lib/gpio.c -o build/gpio.o
delay.o : src/rpi_lib/delay.c
	$(ARMGNU)-gcc $(COPS) -c src/rpi_lib/delay.c -o build/delay.o
uart.o : src/rpi_lib/uart.c
	$(ARMGNU)-gcc $(COPS) -c src/rpi_lib/uart.c -o build/uart.o
	

LIB_IMPORT = -lm -lc -L/usr/lib/arm-none-eabi/newlib/hard -L/usr/lib/gcc/arm-none-eabi/7.3.1

build/vvvf_main.elf : memmap start.o vvvf_main.o vvvf_wave.o gpio.o delay.o uart.o
	$(ARMGNU)-ld build/start.o build/vvvf_main.o build/vvvf_wave.o build/gpio.o build/delay.o build/uart.o -T memmap $(LIB_IMPORT) -o build/vvvf_main.elf
	$(ARMGNU)-objdump -D build/vvvf_main.elf > build/vvvf_main.list

kernel.img : build/vvvf_main.elf
	$(ARMGNU)-objcopy --srec-forceS3 build/vvvf_main.elf -O srec build/vvvf_main.srec
	$(ARMGNU)-objcopy build/vvvf_main.elf -O binary build/kernel.img

