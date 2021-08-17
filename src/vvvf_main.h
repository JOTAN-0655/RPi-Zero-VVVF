void led_toggle();
void led_high();
void led_low();

void all_off();
void initialize_vvvf_pin();
// stat = 1 => pin_H = 1,pin_L=0
// stat = 0 => pin_H = 0,pin_L=0
// stat = -1 => pin_H = 0,pin_L=1
void set_phase(char pin_H,char pin_L,int stat);
char get_pin_num_H(char n);
char get_pin_num_L(char n);

Wave_Values get_Value_mode(int mode,char brake,double initial_phase);
int pin_run(int mode);
int main();