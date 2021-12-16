#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "my_math.h"

#include "rpi_lib/gpio.h"
#include "rpi_lib/delay.h"
#include "rpi_lib/rpi_address.h"

//#define ENABLE_MASCON_OFF

//PIN DEFINE
#define PIN_U_HIGH_2 12
#define PIN_U_HIGH_1 13
#define PIN_U_LOW_1 0
#define PIN_U_LOW_2 21

#define PIN_V_HIGH_2 16
#define PIN_V_HIGH_1 6
#define PIN_V_LOW_1 11
#define PIN_V_LOW_2 26

#define PIN_W_HIGH_2 20
#define PIN_W_HIGH_1 5
#define PIN_W_LOW_1 9
#define PIN_W_LOW_2 19

#define mascon_1 4
#define mascon_2 17
#define mascon_3 27
#define mascon_4 22

#define button_R 1
#define button_SEL 7
#define button_L 8

#define debug_PIN_2 24
#define debug_PIN 25
#define LED_PIN 47


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
	digitalWrite(debug_PIN, HIGH);
	debug_pin_stat = 1;
}
void debug_pin_low()
{
	digitalWrite(debug_PIN, LOW);
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
	digitalWrite(debug_PIN_2, HIGH);
	debug_pin_2_stat = 1;
}
void debug_pin_2_low()
{
	digitalWrite(debug_PIN_2, LOW);
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

int get_Mascon_status()
{
	int bit1 = digitalRead(mascon_1);
	int bit2 = digitalRead(mascon_2);
	int bit3 = digitalRead(mascon_3);
	int bit4 = digitalRead(mascon_4);

	int status = bit1 | bit2 << 1 | bit3 << 2 | bit4 << 3;
	return status;
}

char ignore_pin_change = 0;
int pre_phase_0_stat = 0, pre_phase_1_stat = 0, pre_phase_2_stat = 0;
int get_phase_stat(char phase)
{
	if (phase == 0)
		return pre_phase_0_stat;
	else if (phase == 1)
		return pre_phase_1_stat;
	else
		return pre_phase_2_stat;
}
void set_phase_stat(char phase, int stat)
{
	if (phase == 0)
		pre_phase_0_stat = stat;
	else if (phase == 1)
		pre_phase_1_stat = stat;
	else
		pre_phase_2_stat = stat;
}

char get_pin_H_2(char phase)
{
	if(phase == 0) return PIN_U_HIGH_2;
	else if(phase == 1) return PIN_V_HIGH_2;
	else return PIN_W_HIGH_2;
}

char get_pin_L_2(char phase)
{
	if(phase == 0) return PIN_U_LOW_2;
	else if(phase == 1) return PIN_V_LOW_2;
	else return PIN_W_LOW_2;
}

char get_pin_H_1(char phase)
{
	if(phase == 0) return PIN_U_HIGH_1;
	else if(phase == 1) return PIN_V_HIGH_1;
	else return PIN_W_HIGH_1;
}

char get_pin_L_1(char phase)
{
	if(phase == 0) return PIN_U_LOW_1;
	else if(phase == 1) return PIN_V_LOW_1;
	else return PIN_W_LOW_1;
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
Gpio_Set_Data get_phase_set(int stat)
{
	char H_1,H_2,L_1,L_2;
	if (stat == 0)
	{
		H_2 = LOW;
		H_1 = LOW;
		L_1 = HIGH;
		L_2 = HIGH;
	}
	else if(stat == 1){
		H_2 = LOW;
		H_1 = HIGH;
		L_1 = HIGH;
		L_2 = LOW;
	}
	else if (stat == 2)
	{
		H_2 = HIGH;
		H_1 = HIGH;
		L_1 = LOW;
		L_2 = LOW;
	}
	else
	{
		H_2 = LOW;
		H_1 = LOW;
		L_1 = LOW;
		L_2 = LOW;
	}
	Gpio_Set_Data set_data = { H_2,H_1,L_1,L_2 };
	return set_data;
}

void set_phase(int stat_U,int stat_V,int stat_W)
{
	if (ignore_pin_change == 1)
		return;

	unsigned int set_to_high = 0, set_to_low = 0;

	for(int phase = 0; phase < 3;phase++){
		int p_s = stat_U;
		if(phase == 1) p_s = stat_V;
		else if(phase == 2) p_s = stat_W;

		Gpio_Set_Data gpio = get_phase_set(p_s);
		if(gpio.H_2) set_to_high |= 1 << get_pin_H_2(phase);
		else set_to_low |= 1 << get_pin_H_2(phase);

		if(gpio.H_1) set_to_high |= 1 << get_pin_H_1(phase);
		else set_to_low |= 1 << get_pin_H_1(phase);

		if(gpio.L_1) set_to_high |= 1 << get_pin_L_1(phase);
		else set_to_low |= 1 << get_pin_L_1(phase);

		if(gpio.L_2) set_to_high |= 1 << get_pin_L_2(phase);
		else set_to_low |= 1 << get_pin_L_2(phase);
	}

	digitalWrite_special(set_to_high, HIGH);
	digitalWrite_special(set_to_low, LOW);
}

char total_modes = 23;
Wave_Values get_Value_mode(int mode, Control_Values cv)
{
	if (mode == 0)
		return calculate_207(cv);
	else if (mode == 1)
		return calculate_toyo_GTO(cv);
	else if (mode == 2)
		return calculate_doremi(cv);
	else if (mode == 3)
		return calculate_E209(cv);
	else if (mode == 4)
		return calculate_mitsubishi_gto(cv);
	else if (mode == 5)
		return calculate_tokyu_9000_hitachi_gto(cv);
	else if (mode == 6)
		return calculate_keio_8000_gto(cv);

	else if (mode == 7)
		return calculate_E231(cv);
	else if (mode == 8)
		return calculate_9820_mitsubishi(cv);
	else if (mode == 9)
		return calculate_9820_hitachi(cv);
	else if (mode == 10)
		return calculate_E233(cv);
	else if (mode == 11)
		return calculate_E235(cv);
	else if (mode == 12)
		return calculate_toyo_IGBT(cv);
	else if (mode == 13)
		return calculate_toubu_50050(cv);
	else if (mode == 14)
		return calculate_207_1000_update(cv);
	else if (mode == 15)
		return calculate_225_5100_mitsubishi(cv);
	else if (mode == 16)
		return calculate_321_hitachi(cv);
	else if (mode == 17)
		return calculate_toei_6300_3(cv);
	else if (mode == 18)
		return calculate_keihan_13000_toyo_IGBT(cv);
	else if (mode == 19)
		return calculate_tokyuu_5000(cv);
	else if (mode == 20)
		return calculate_tokyuu_1000_1500_IGBT(cv);
	else if (mode == 21)
		return calculate_E233_3000(cv);

	/*
	else if(mode == 21)
		return calculate_jre_209_mitsubishi_gto(cv);
	*/

	else if (mode == 22)
		return calculate_Famima(cv);
	else if (mode == 23)
		return calculate_real_doremi(cv);

	else
		return calculate_silent(cv);
}

char update_pin = 1;
unsigned int button_press_count = 0;

int mascon_off_div = 1000;

int pin_run(int mode)
{
	int return_val = 0;

	double wave_stat = 0;
	bool brake = false;
	bool mascon_off = false;
	bool free_run = false;

	unsigned long long start_system_time = 0, end_targer_system_time = 0;

	led_high();

	clearEventDetect(button_R);
	clearEventDetect(button_L);
	clearEventDetect(button_SEL);

	debug_pin_2_low();

	while (1)
	{
		start_system_time = get_systime();
		end_targer_system_time = start_system_time + 17;
		sin_time += 0.000017;
		saw_time += 0.000017;

		debug_pin_2_high();

		int stat_U = 0, stat_V = 0, stat_W = 0;
		for (int i = 0; i < 3; i++)
		{
			if (!update_pin)
				continue;
			double initial_phase = (double)2.094395393195 * (double)i; //(double)2.094395102393195 * (double)i;
			Control_Values cv = {brake,!mascon_off, free_run ,initial_phase,wave_stat};
			Wave_Values wv = get_Value_mode(mode, cv);
			int require_stat = wv.pwm_value;
			if (i == 0)
				stat_U = require_stat;
			else if (i == 1)
				stat_V = require_stat;
			else
				stat_W = require_stat;
		}

		int dead_time_U = stat_U,dead_time_V = stat_V,dead_time_W = stat_W;
		if (get_phase_stat(0) != stat_U){
			dead_time_U = 3;
			set_phase_stat(0,stat_U);
		}
		if (get_phase_stat(1) != stat_V){
			dead_time_V = 3;
			set_phase_stat(1,stat_V);
		}
		if (get_phase_stat(2) != stat_W){
			dead_time_W = 3;
			set_phase_stat(2,stat_W);
		}

		set_phase(dead_time_U,dead_time_V,dead_time_W);

		//sine angle freq change etc..
		double sin_new_angle_freq = sin_angle_freq;
		char mascon_status = (char)get_Mascon_status();
		if (mascon_status - 4 > 0)
		{
			sin_new_angle_freq += 0.00026179938779915 * (mascon_status - 4);
			brake = false;
			mascon_off = false;
		}
		else if (mascon_status - 4 < 0)
		{
			sin_new_angle_freq -= 0.00026179938779915 * (4 - mascon_status);
			brake = true;
			mascon_off = false;
		}
#ifdef ENABLE_MASCON_OFF
		else
		{
			if (sin_new_angle_freq > 0)
				mascon_off = true;
			else
				mascon_off = false;
		}
#endif

		if (sin_new_angle_freq > 942.4777960769379)
			sin_new_angle_freq = 942.4777960769379;
		if (wave_stat <= 0)
		{
			all_off();
			disconnect = true;
			ignore_pin_change = true;
		}
		else{
			disconnect = false;
			ignore_pin_change = false;
		}
			
		if (sin_new_angle_freq <= 0)
		{
			sin_time = 0;
			saw_time = 0;
			sin_angle_freq = 0;
			wave_stat = 0;
			all_off();
			disconnect = true;
			ignore_pin_change = true;
			
		}
		else if (!free_run)
		{
			disconnect = false;
			ignore_pin_change = false;
			sin_time *= sin_angle_freq;
			sin_time /= sin_new_angle_freq;
			sin_angle_freq = sin_new_angle_freq;
		}


#ifndef ENABLE_MASCON_OFF
		wave_stat = sin_angle_freq * M_1_2PI;
#endif

#ifdef ENABLE_MASCON_OFF
		if (!mascon_off)
        {
            if(free_run){
				wave_stat += 1 / (double)mascon_off_div;
           		if (sin_angle_freq * M_1_2PI < wave_stat){
					free_run = false;
					wave_stat = sin_angle_freq * M_1_2PI;
				}
				else
					free_run = true;
			}else{
				wave_stat = sin_angle_freq * M_1_2PI;
			}
				
        else
        {
			free_run = true;
            wave_stat -= 1 / (double)mascon_off_div;
            if (wave_stat < 0) wave_stat = 0;
        }
#endif

		if (digitalRead(button_R) == 0 && sin_angle_freq == 0)
		{
			if (button_press_count == 1000)
			{
				button_press_count = 0;
				return_val = 1;
				break;
			}
			else
				button_press_count++;
		}
		else if (digitalRead(button_L) == 0 && sin_angle_freq == 0)
		{
			if (button_press_count == 1000)
			{
				button_press_count = 0;
				return_val = -1;
				break;
			}
			else
				button_press_count++;
		}
		else if (digitalRead(button_SEL) == 0)
		{
			if (button_press_count == 1000)
			{
				button_press_count = 0;
				break;
			}
			else
				button_press_count++;
		}
		else
		{
			button_press_count = 0;
		}
		
		debug_pin_2_low();

		set_phase(stat_U,stat_V,stat_W);
		
		while (end_targer_system_time > get_systime());
		debug_pin_toggle();
	}
	led_low();
	debug_pin_low();

	all_off();
	return return_val;
}

int main(void)
{
	InitializeGpio();
	led_low();
	debug_pin_low();

	initialize_vvvf_pin();
	reset_all_variables();

	int mode = 0;
	while (1)
	{
		int change = pin_run(mode);
		if (change == 0)
			break;
		delay_ms(500);
		mode += change;
		if (mode < 0)
			mode = total_modes;
		else if (mode > total_modes)
			mode = 0;
	}

	while (1)
	{

		led_toggle();
		delay_ms(100);
	}
	return (0);
}
