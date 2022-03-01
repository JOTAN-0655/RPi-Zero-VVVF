#ifndef VVVF_RASPBERRYPI_H
#define VVVF_RASPBERRYPI_H

// PUBLIC ERIA
extern char ignore_pin_change;
//


void led_toggle();
void led_high();
void led_low();

void debug_pin_toggle();
void debug_pin_high();
void debug_pin_low();

void debug_pin_2_toggle();
void debug_pin_2_high();
void debug_pin_2_low();

void all_off();
void initialize_vvvf_pin();
char get_Mascon_status();

char get_phase_stat(char phase);
void set_phase_stat(char phase,char stat);

typedef struct {
    char H_2;
    char H_1;
    char L_1;
    char L_2;
} Gpio_Set_Data;

char get_pin_H_2(char phase);
char get_pin_L_2(char phase);
char get_pin_H_1(char phase);
char get_pin_L_1(char phase);

Gpio_Set_Data get_phase_set(char stat);
void set_phase(char stat_U,char stat_V,char stat_W);

#endif