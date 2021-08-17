# raspi-zero-vvvf
Making VVVF inverter with raspberry pi zero.
This code is for baremetal on raspberry pi zero.

# requirements
You need to install cross compiler.<br>
```
sudo apt install gcc-arm-none-eabi
```

# build
To build these code, just run
```
sh build.sh
```

# install to RPi zero
Since you have finished your code build, you will find `kernel.img` inside of build folder.<br>
What you need to do is

## Download the needed files.
From https://github.com/raspberrypi/firmware/tree/master/boot , you have to get <br>
 - bootcode.bin
 - start.elf

## SD card
Now, you need to have some SD card which has more than 2GB.<br>
Also the SD card needs to be formated with FAT32.<br>
<br>

## Install
Now , it's time to install.<br>
Copy `bootcode.bin` , `start.elf` and `kernel.img` that you have built.<br>

## Sequel
If you want raspberry pi zero to work faster, make `config.txt` and just paste next code.<br>
```
force_turbo=1
arm_freq=1200
```

# pin layout
 - PIN_U_HIGH 5
 - PIN_U_LOW 19
 - PIN_V_HIGH 6
 - PIN_V_LOW 26
 - PIN_W_HIGH 13
 - PIN_W_LOW 21


# references
https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf<br>
https://github.com/licux/BareMetalInvaders
