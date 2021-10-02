#include "rpi_lib/gpio.h"

void led_toggle();
void led_high();
void led_low();

void all_off();
void initialize_vvvf_pin();
int get_Mascon_status();

// stat = 1 => pin_H = 1,pin_L=0
// stat = 0 => pin_H = 0,pin_L=0
// stat = -1 => pin_H = 0,pin_L=1
int get_phase_stat(char phase);
void set_phase_stat(char phase,int stat);
char get_pin_H(char phase);
char get_pin_L(char phase);
void set_phase(char phase,int stat);

Wave_Values get_Value_mode(int mode,bool brake,double initial_phase);
int pin_run(int mode);
int main();