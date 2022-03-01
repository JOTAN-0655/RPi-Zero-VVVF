#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "vvvf_calculate.h"
#include "vvvf_raspberrypi.h"
#include "vvvf_struct.h"

#include "my_math.h"
#include "settings.h"

#include "rpi_lib/gpio.h"
#include "rpi_lib/delay.h"
#include "rpi_lib/rpi_address.h"



char total_modes = 2;
void (* calc_func)(Control_Values * , Carrier_Settings *);
void set_Calculation_Func(int mode)
{
	switch(mode){
		case 0:
			calc_func = calculate_keikyu_n1000_siemens_gto_2_level;
			break;
		default:
			calc_func = calculate_E231;
			break;
	}
}



int pin_run()
{
	int return_val = 0;
	char update_pin = 1;
	unsigned int button_press_count = 0;
	double control_freq = 0;
	bool brake = false;
	bool mascon_off = false;
	bool free_run = false;
	bool disconnect = false;

	led_high();
	debug_pin_2_low();

	Calculate_Values current_status;
	current_status.saw_angle_freq = 0;
	current_status.saw_time = 0;
	current_status.sin_angle_freq = 0;
	current_status.sin_time = 0;

	while (1)
	{
		unsigned long long start_system_time = get_systime();
		unsigned long long end_targer_system_time = start_system_time + 17;
		current_status.sin_time += 0.000017;
		current_status.saw_time += 0.000017;

		char stat_U=0,stat_V=0,stat_W=0;
		Control_Values control_value = {brake , !mascon_off , free_run , control_freq , current_status.sin_angle_freq * M_1_2PI};
		Carrier_Settings cs = {P_1,0,1,-1, 0};

		debug_pin_2_toggle();
		(* calc_func)(&control_value , &cs); // sets carrier settings;
		debug_pin_2_toggle();

		for (int i = 0; i < 3; i++)
		{
			debug_pin_2_toggle();
			if (!update_pin)
				continue;
			double initial_phase = (double)2.094395393195 * (double)i; //(double)2.094395102393195 * (double)i;
			char require_stat=0;
			if(cs.level == 2)
				require_stat = calculate_two_level(&current_status ,&cs ,initial_phase);
			else
				require_stat = calculate_three_level(&current_status ,&cs ,initial_phase);


			if (i == 0)
				stat_U = require_stat;
			else if (i == 1)
				stat_V = require_stat;
			else
				stat_W = require_stat;
			
		}

		
		debug_pin_2_low();

		char dead_time_U = stat_U,dead_time_V = stat_V,dead_time_W = stat_W;
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

		debug_pin_2_high();

		set_phase(dead_time_U,dead_time_V,dead_time_W);
		

		//sine angle freq change etc..
		double sin_new_angle_freq = current_status.sin_angle_freq;
		char mascon_status = get_Mascon_status();
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

		if (sin_new_angle_freq > 942.4777960769379)
			sin_new_angle_freq = 942.4777960769379;
		if (control_freq <= 0)
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
			current_status.sin_time = 0;
			current_status.saw_time = 0;
			current_status.sin_angle_freq = 0;
			control_freq = 0;
			all_off();
			disconnect = true;
			ignore_pin_change = true;
			
		}
		else if (!free_run)
		{
			disconnect = false;
			ignore_pin_change = false;

			current_status.sin_time = current_status.sin_angle_freq / sin_new_angle_freq * current_status.sin_time;
			current_status.sin_angle_freq = sin_new_angle_freq;
		}
		control_freq = current_status.sin_angle_freq * M_1_2PI;

		if (digitalRead(button_R) == 0 && current_status.sin_time == 0)
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
		else if (digitalRead(button_L) == 0 && current_status.sin_time == 0)
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

		if(disconnect)
			set_phase(3,3,3);
		else
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
	debug_pin_2_low();

	initialize_vvvf_pin();

	int mode = 0;
	set_Calculation_Func(mode);
	while (1)
	{
		int change = pin_run();
		if (change == 0)
			break;
		delay_ms(100);
		mode += change;
		if (mode < 0)
			mode = total_modes;
		else if (mode > total_modes)
			mode = 0;
		set_Calculation_Func(mode);
	}

	while (1)
	{

		led_toggle();
		delay_ms(100);
	}
	return (0);
}
