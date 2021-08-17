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

#define HANDLE_BIT_0 17
#define HANDLE_BIT_1 27
#define HANDLE_BIT_2 22

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
	all_off();
}

char ignore_pin_change = 0;
// stat = 1 => pin_H = 1,pin_L=0
// stat = 0 => pin_H = 0,pin_L=0
// stat = -1 => pin_H = 0,pin_L=1
void set_phase(char pin_H,char pin_L,int stat){
	if(ignore_pin_change == 1) return;
	digitalWrite(pin_H, LOW);
	digitalWrite(pin_L, LOW);
	delay_us(1);
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

char get_pin_num_H(char n){
	if(n == 0) return PIN_U_HIGH;
	else if(n == 1) return PIN_V_HIGH;
	else return PIN_W_HIGH;
}
char get_pin_num_L(char n){
	if(n == 0) return PIN_U_LOW;
	else if(n == 1) return PIN_V_LOW;
	else return PIN_W_LOW;
}

Wave_Values get_Value_mode(int mode,char brake,double initial_phase){
	Wave_Values wv;
	if(mode == 0)wv = caliculate_207(brake,initial_phase);
	else if(mode == 1) wv = caliculate_E231(brake,initial_phase);
	else if(mode == 2) wv = caliculate_doremi(brake,initial_phase);
	else if(mode == 3) wv = caliculate_E235(brake,initial_phase);
	else wv = caliculate_silent(brake,initial_phase);
	
	return wv;
}

long count = 0;
char do_frequency_change = 1,update_pin = 1;
int pin_run(int mode){
	int return_val = 1;
	reset_all_variables();
	char brake = 0;
	unsigned long long start_system_time = 0,end_targer_system_time = 0;
	led_high();
	while (1)
    {
		start_system_time = get_systime();
		end_targer_system_time = start_system_time + 32;
		sin_time += 0.000032;
		saw_time += 0.000032;

		for(int i = 0; i < 3; i++)
		{
			if(!update_pin) continue;
			double initial_phase = (double)2.094395102393195 * (double)i;
			Wave_Values wv = get_Value_mode(mode,brake,initial_phase);
			char H_PIN = get_pin_num_H(i);
			char L_PIN = get_pin_num_L(i);
			set_phase(H_PIN,L_PIN,(int)wv.pwm_value);
		}

		count++;
		if (count % 8 == 0 && do_frequency_change)
		{
			double sin_new_angle_freq = sin_angle_freq;
			if(!brake) sin_new_angle_freq += M_PI / 500.0;
			else sin_new_angle_freq -= M_PI / 500.0;
			double amp = sin_angle_freq / sin_new_angle_freq;
			sin_angle_freq = sin_new_angle_freq;
			sin_time = amp * sin_time;
		}
		
		if(sin_angle_freq / 6.283185307179586 > 150 && !brake && do_frequency_change){
			do_frequency_change = 0;
			count = 0;
		}else if(count / 31250 == 2 && !do_frequency_change){
			do_frequency_change = 1;
			brake = 1;
		}else if(sin_angle_freq / 6.283185307179586 < 0 && brake && do_frequency_change) break;
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
	srand(0);
	led_low();

	for(int x = 0; x < 4;x++){
		pin_run(x);
	}
	
	
	
	while(1) {

		led_toggle();
		delay_ms(500);

	}
    return(0);
}

