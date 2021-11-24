# raspi-zero-vvvf
Making VVVF inverter with raspberry pi zero.
This code is for baremetal on raspberry pi zero.
This `main` repository is for 2 and 3 level vvvf.

# terms of use
## disclaimer
PLEASE DO EVERYTHING BY YOUR OWN RESPONSIBILITY.<br>
MAY CAUSE YOU A BIG DAMAGE WITH ELECTRONIC.<br>
THIS VVVF CODE IS NOT MADE BY ENGINEER OR ANY PROFESSIONAL.<br>

## credit
Main author of this program is JOTAN-0655 except raspberry pi lib.<br>
Please label this github url and references url if you upload on youtube or other websites.

# references
https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf<br>
https://github.com/licux/BareMetalInvaders (I borrowed some cord from here.)

# requirements
You need to install cross compiler.<br>
```
sudo apt install gcc-arm-none-eabi
```

# build
Normally, just run <br>
`sh build.sh`


# install to RPi zero
Since you have finished your code build, you will find `kernel.img` inside of build folder.<br>
What you need to do is

## Download the needed files
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

# VVVF pin out
This number is BCM GPIO number.
- PIN_U_HIGH_2 12
- PIN_U_HIGH_1 13
- PIN_U_LOW_1 0
- PIN_U_LOW_2 21

- PIN_V_HIGH_2 16
- PIN_V_HIGH_1 6
- PIN_V_LOW_1 11
- PIN_V_LOW_2 26

- PIN_W_HIGH_2 20
- PIN_W_HIGH_1 5
- PIN_W_LOW_1 9
- PIN_W_LOW_2 19

# Function pin out
This number is BCM GPIO number

## Mascon (Speed controller)
### pin out
 - mascon_1 4
 - mascon_2 17
 - mascon_3 27
 - mascon_4 22

Inside the program, it will generate a integer by using mascon_1 ~ mason_4.<br>
This is the how integer will be.
`mascon_status_value = input(mascon_1) | input(mascon_2)<<1 | input(mascon_3)<<2 | input(mascon_4)<<3`<br>

When mascon_status_value equals to 4 , it will not change its vvvf speed.
If it is less than 4 (3,2,1,0), vvvf frequency will decrease. 0 is most strong frequency decrease.
If it is more than 4 (5,6,7,8), vvvf frequency will increase. 8 is most strong frequency increase.

## Control button
### pin out
 - button_R 1
 - button_SEL 7
 - button_L 8

When button_SEL is pressed, it will stop vvvf system.And sets all of vvvf pin to low.
When button_R/L is pressed, it will change vvvf sound.
