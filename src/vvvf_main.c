#include "vvvf_wave.h"
#include "vvvf_main.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rpi_lib/gpio.h"
#include "rpi_lib/delay.h"
#include "rpi_lib/uart.h"
#include "rpi_lib/rpi_address.h"


//PIN DEFINE
#define PIN_U_HIGH 5
#define PIN_U_LOW 19

#define PIN_V_HIGH 6
#define PIN_V_LOW 26

#define PIN_W_HIGH 13
#define PIN_W_LOW 21

#define mascon_1 4
#define mascon_2 17
#define mascon_3 27
#define mascon_4 22

#define button_R 1
#define button_SEL 7
#define button_L 8

#define LED_PIN 47

//#define DELAY_SEARCH

char led_toggle_v = 0;
void led_toggle(){
	if(led_toggle_v==0) led_high();
	else led_low();
}
void led_high(){
	digitalWrite(LED_PIN,LOW);
	led_toggle_v = 1;
}
void led_low(){
	digitalWrite(LED_PIN,HIGH);
	led_toggle_v = 0;
}

void all_off(){
	digitalWrite(PIN_U_HIGH,LOW);
	digitalWrite(PIN_V_HIGH,LOW);
	digitalWrite(PIN_W_HIGH,LOW);
	digitalWrite(PIN_U_LOW,LOW);
	digitalWrite(PIN_V_LOW,LOW);
	digitalWrite(PIN_W_LOW,LOW);
}
void initialize_vvvf_pin(){
	pinMode(PIN_U_HIGH, OUTPUT);
	pinMode(PIN_V_HIGH, OUTPUT);
	pinMode(PIN_W_HIGH, OUTPUT);
	pinMode(PIN_U_LOW, OUTPUT);
	pinMode(PIN_V_LOW, OUTPUT);
	pinMode(PIN_W_LOW, OUTPUT);
	
	pinMode(mascon_1, INPUT);
	pinMode(mascon_2, INPUT);
	pinMode(mascon_3, INPUT);
	pinMode(mascon_4, INPUT);

	pinMode(button_R, INPUT);
	pinMode(button_SEL, INPUT);
	pinMode(button_L, INPUT);

	all_off();
}

int get_Mascon_status(){
	int bit1 = digitalRead(mascon_1);
	int bit2 = digitalRead(mascon_2);
	int bit3 = digitalRead(mascon_3);
	int bit4 = digitalRead(mascon_4);

	int status = bit1 | bit2<<1 | bit3<<2 | bit4<<3;
	return status;
}

char ignore_pin_change = 0;
int pre_phase_0_stat = 0,pre_phase_1_stat = 0,pre_phase_2_stat = 0;
int get_phase_stat(char phase){
	if(phase == 0) return pre_phase_0_stat;
	else if(phase == 1) return pre_phase_1_stat;
	else return pre_phase_2_stat;
}
void set_phase_stat(char phase,int stat){
	if(phase == 0) pre_phase_0_stat = stat;
	else if(phase == 1) pre_phase_1_stat = stat;
	else pre_phase_2_stat = stat;
}

char get_pin_H(char phase){
	switch(phase){
		case 0:
			return PIN_U_HIGH;
		case 1:
			return PIN_V_HIGH;
		default:
			return PIN_W_HIGH;
	}
}

char get_pin_L(char phase){
	switch(phase){
		case 0:
			return PIN_U_LOW;
		case 1:
			return PIN_V_LOW;
		default:
			return PIN_W_LOW;
	}
}
// stat = 1 => pin_H = 1,pin_L=0
// stat = 0 => pin_H = 0,pin_L=0
// stat = -1 => pin_H = 0,pin_L=1
void set_phase(char phase,int stat){
	if(ignore_pin_change == 1) return;
	if(get_phase_stat(phase) == stat) return;
	char pin_H = get_pin_H(phase);
	char pin_L = get_pin_L(phase);
	set_phase_stat(phase,stat);
	if(stat == 0){
		digitalWrite(pin_H, LOW);
		digitalWrite(pin_L, LOW);
	}else if(stat == 1){
		digitalWrite(pin_H, HIGH);
		digitalWrite(pin_L, LOW);
	}else if(stat == -1){
		digitalWrite(pin_H, LOW);
		digitalWrite(pin_L, HIGH);
	}else{
		digitalWrite(pin_H, LOW);
		digitalWrite(pin_L, LOW);
	}
}

char total_modes = 9;
Wave_Values get_Value_mode(int mode,bool brake,double initial_phase){
	if(mode == 0) return calculate_207(brake,initial_phase);
	else if(mode == 1) return calculate_E231(brake,initial_phase);
	else if(mode == 2) return calculate_doremi(brake,initial_phase);
	else if(mode == 3) return calculate_E235(brake,initial_phase);
	else if(mode == 4) return calculate_E209(brake,initial_phase);
	else if(mode == 5) return calculate_9820_mitsubishi(brake,initial_phase);
	else if(mode == 6) return calculate_9820_hitachi(brake,initial_phase);
	else if(mode == 7) return calculate_E233(brake,initial_phase);
	else if(mode == 8) return calculate_mitsubishi_gto(brake,initial_phase);
	else if(mode == 9) return calculate_toyo_IGBT(brake,initial_phase);
	else return calculate_silent(brake,initial_phase);
}

char count = 0;
char do_frequency_change = 1,update_pin = 1;
unsigned int button_press_count = 0;
int pin_run(int mode){
	int return_val = 0;
	bool brake = false;

	unsigned long long start_system_time = 0,end_targer_system_time = 0;

	led_high();

	clearEventDetect(button_R);
	clearEventDetect(button_L);
	clearEventDetect(button_SEL);

	while (1)
    {
		start_system_time = get_systime();
		end_targer_system_time = start_system_time + 25;
		sin_time += 0.000025;
		saw_time += 0.000025;


		int stat_U = 0,stat_V=0,stat_W=0;
		for(int i = 0; i < 3; i++)
		{
			if(!update_pin) continue;
			double initial_phase = (double)2.094395393195 * (double)i;//(double)2.094395102393195 * (double)i;
			Wave_Values wv = get_Value_mode(mode,brake,initial_phase);
			int require_stat = (int)wv.pwm_value;
			if (i==0) stat_U = require_stat;
			else if(i==1) stat_V = require_stat;
			else stat_W = require_stat;
		}

		if(get_phase_stat(0) != stat_U) set_phase(0,0);
		if(get_phase_stat(1) != stat_V) set_phase(1,0);
		if(get_phase_stat(2) != stat_W) set_phase(2,0);
		delay_us(1);
		set_phase(0,stat_U);
		set_phase(1,stat_V);
		set_phase(2,stat_W);

		count++;
		if (count % 8 == 0 && do_frequency_change)
		{
			count=0;
			double sin_new_angle_freq = sin_angle_freq;

			if(get_Mascon_status()-4 > 0){
				sin_new_angle_freq += M_PI / 1000.0 * (get_Mascon_status() - 4) ;
				brake = false;
			}else if(get_Mascon_status() - 4 < 0){
				sin_new_angle_freq -= M_PI / 1000.0 * (4-get_Mascon_status());
				brake = true;
			}
			
			if(sin_new_angle_freq > 300.0 * M_PI) sin_new_angle_freq = 300.0 * M_PI;
			
			if(sin_new_angle_freq <= 0){
				sin_time = 0;
				saw_time = 0;
				sin_angle_freq = 0;
				disconnect = true;
				all_off();
			}else{
				disconnect = false;
				sin_time *= sin_angle_freq / sin_new_angle_freq;
				sin_angle_freq = sin_new_angle_freq;
			}
			
			
		}

		if(digitalRead(button_R)==0  && sin_angle_freq == 0){
			if(button_press_count==1000){
				button_press_count=0;
				return_val = 1;
				break;
			}else button_press_count++;
			
		}else if(digitalRead(button_L)==0  && sin_angle_freq == 0){
			if(button_press_count==1000){
				button_press_count=0;
				return_val = -1;
				break;
			}else button_press_count++;
		}else if(digitalRead(button_SEL)==0){
			if(button_press_count==1000){
				button_press_count=0;
				break;
			}else button_press_count++;
		}else{
			button_press_count=0;
		}
		
		
		while(end_targer_system_time > get_systime());
		
    }
	led_low();
    all_off();
	return return_val;
}

int main ( void )
{
	InitializeGpio();
	UartInit();
	pinMode(LED_PIN,OUTPUT);
	led_high();
	initialize_vvvf_pin();
	generate_sin_table();
	reset_all_variables();
	srand(0);
	led_low();

	int mode = 0;
	while(1){
		int change = pin_run(mode);
		if(change==0) break;
		delay_ms(500);
		mode+=change;
		if(mode < 0) mode = total_modes;
		else if(mode > total_modes) mode = 0;
	}
	
	
	while(1) {

		led_toggle();
		delay_ms(100);

	}
    return(0);
}

