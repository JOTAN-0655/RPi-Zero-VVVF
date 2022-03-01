#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "vvvf_calculate.h"
#include "vvvf_raspberrypi.h"

#include "my_math.h"
#include "settings.h"

#include "rpi_lib/gpio.h"
#include "rpi_lib/delay.h"
#include "rpi_lib/rpi_address.h"

// PUBLIC AREA
char ignore_pin_change = 0;
//

char led_toggle_v = 0;
void led_toggle()
{
	if (led_toggle_v == 0)
		led_high();
	else
		led_low();
}
void led_high()
{
	digitalWrite(LED_PIN, LOW);
	led_toggle_v = 1;
}
void led_low()
{
	digitalWrite(LED_PIN, HIGH);
	led_toggle_v = 0;
}


char debug_pin_stat = 0;
void debug_pin_toggle()
{
	if (debug_pin_stat == 0)
		debug_pin_high();
	else
		debug_pin_low();
}
void debug_pin_high()
{
#ifndef DISABLE_DEBUG_PIN
	digitalWrite_special(1 << debug_PIN,HIGH); // 1 << debug_PIN
#endif
	debug_pin_stat = 1;
}
void debug_pin_low()
{
	digitalWrite_special(1 << debug_PIN,LOW); // 1 << debug_PIN
	debug_pin_stat = 0;
}

char debug_pin_2_stat = 0;
void debug_pin_2_toggle()
{
	if (debug_pin_2_stat == 0)
		debug_pin_2_high();
	else
		debug_pin_2_low();
}
void debug_pin_2_high()
{
#ifndef DISABLE_DEBUG_PIN
	digitalWrite_special(1 << debug_PIN_2,HIGH); // 1 << debug_PIN_2
#endif
	debug_pin_2_stat = 1;
}
void debug_pin_2_low()
{
	digitalWrite_special(1 << debug_PIN_2,LOW); // 1 << debug_PIN_2
	debug_pin_2_stat = 0;
}

void all_off()
{
	set_phase(3,3,3);
}
void initialize_vvvf_pin()
{
	pinMode(PIN_U_HIGH_2, OUTPUT);
	pinMode(PIN_U_HIGH_1, OUTPUT);
	pinMode(PIN_U_LOW_1, OUTPUT);
	pinMode(PIN_U_LOW_2, OUTPUT);
	
	pinMode(PIN_V_HIGH_2, OUTPUT);
	pinMode(PIN_V_HIGH_1, OUTPUT);
	pinMode(PIN_V_LOW_1, OUTPUT);
	pinMode(PIN_V_LOW_2, OUTPUT);

	pinMode(PIN_W_HIGH_2, OUTPUT);
	pinMode(PIN_W_HIGH_1, OUTPUT);
	pinMode(PIN_W_LOW_1, OUTPUT);
	pinMode(PIN_W_LOW_2, OUTPUT);

	pinMode(mascon_1, INPUT);
	pinMode(mascon_2, INPUT);
	pinMode(mascon_3, INPUT);
	pinMode(mascon_4, INPUT);

	pinMode(button_R, INPUT);
	pinMode(button_SEL, INPUT);
	pinMode(button_L, INPUT);

	pinMode(LED_PIN, OUTPUT);
	pinMode(debug_PIN, OUTPUT);
	pinMode(debug_PIN_2, OUTPUT);

	all_off();
}

char get_Mascon_status()
{
	int bit1 = digitalRead(mascon_1);
	int bit2 = digitalRead(mascon_2);
	int bit3 = digitalRead(mascon_3);
	int bit4 = digitalRead(mascon_4);

	char status = (char)bit1 | (char)(bit2 << 1) | (char)(bit3 << 2) | (char)(bit4 << 3);
	return status;
}

char pre_phase_0_stat = 0, pre_phase_1_stat = 0, pre_phase_2_stat = 0;
char get_phase_stat(char phase)
{
	switch(phase){
		case 0:
			return pre_phase_0_stat;
		case 1:
			return pre_phase_1_stat;
		default:
			return pre_phase_2_stat;
	}
}
void set_phase_stat(char phase, char stat)
{
	switch(phase){
		case 0:
			pre_phase_0_stat = stat;
			break;
		case 1:
			pre_phase_1_stat = stat;
			break;
		default:
			pre_phase_2_stat = stat;
			break;
	}
}

char get_pin_H_2(char phase)
{
	switch(phase){
		case 0:
			return PIN_U_HIGH_2;
		case 1:
			return PIN_V_HIGH_2;
		default:
			return PIN_W_HIGH_2;
	}
}
char get_pin_L_2(char phase)
{
	switch(phase){
		case 0:
			return PIN_U_LOW_2;
		case 1:
			return PIN_V_LOW_2;
		default:
			return PIN_W_LOW_2;
	}
}
char get_pin_H_1(char phase)
{
	switch(phase){
		case 0:
			return PIN_U_HIGH_1;
		case 1:
			return PIN_V_HIGH_1;
		default:
			return PIN_W_HIGH_1;
	}
}
char get_pin_L_1(char phase)
{
	switch(phase){
		case 0:
			return PIN_U_LOW_1;
		case 1:
			return PIN_V_LOW_1;
		default:
			return PIN_W_LOW_1;
	}
}

/**
 * @brief Get the phase set object
 * stat = 0 => PIN_H_1/2 = 0 , PIN_L_1/2 = 1
 * stat = 1 => PIN_H/L_2 = 0 , PIN_H/L_1 = 1
 * stat = 2 => PIN_H_1/2 = 1 , PIN_L_1/2 = 0
 * no above => PIN_H_1/2 = 0 , PIN_L_1/2 = 0
 * 
 * @param stat 
 * @return Gpio_Set_Data 
 */
Gpio_Set_Data get_phase_set(char stat)
{
	char H_1,H_2,L_1,L_2;
	switch(stat){
	case 0:
		H_2 = LOW;
		H_1 = LOW;
		L_1 = HIGH;
		L_2 = HIGH;
		break;
	case 1:
		H_2 = LOW;
		H_1 = HIGH;
		L_1 = HIGH;
		L_2 = LOW;
		break;
	case 2:
		H_2 = HIGH;
		H_1 = HIGH;
		L_1 = LOW;
		L_2 = LOW;
		break;
	default:
		H_2 = LOW;
		H_1 = LOW;
		L_1 = LOW;
		L_2 = LOW;
		break;
	}
	Gpio_Set_Data set_data = { H_2,H_1,L_1,L_2 };
	return set_data;
}
void set_phase(char stat_U,char stat_V,char stat_W)
{
	if (ignore_pin_change == 1)
		return;

	unsigned int set_to_high = 0, set_to_low = 0;

	for(char phase = 0; phase < 3;phase++){
		char p_s = stat_U;
		if(phase == 1) p_s = stat_V;
		else if(phase == 2) p_s = stat_W;

		Gpio_Set_Data gpio = get_phase_set(p_s);
		if(gpio.H_2) set_to_high |= 1 << get_pin_H_2(phase);
		else set_to_low |= 1 << get_pin_H_2(phase);

#ifdef ENABLE_3_LEVEL
		if(gpio.H_1) set_to_high |= 1 << get_pin_H_1(phase);
		else set_to_low |= 1 << get_pin_H_1(phase);

		if(gpio.L_1) set_to_high |= 1 << get_pin_L_1(phase);
		else set_to_low |= 1 << get_pin_L_1(phase);
#endif

		if(gpio.L_2) set_to_high |= 1 << get_pin_L_2(phase);
		else set_to_low |= 1 << get_pin_L_2(phase);
	}

	digitalWrite_special(set_to_high, HIGH);
	digitalWrite_special(set_to_low, LOW);
}