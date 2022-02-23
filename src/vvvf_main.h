#ifndef VVVF_MAIN_H

#define VVVF_MAIN_H

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

// stat = 1 => pin_H = 1,pin_L=0
// stat = 0 => pin_H = 0,pin_L=0
// stat = -1 => pin_H = 0,pin_L=1
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

void set_Calculation_Func(int mode);

extern int mascon_off_div;
extern double pin_run_wave_stat;
int pin_run();
int main();

#endif