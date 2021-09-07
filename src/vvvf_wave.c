#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "rpi_lib/delay.h"
#include "vvvf_wave.h"
#include "vvvf_main.h"

#include "rpi_lib/uart.h"

#define M_2PI 6.28318530717958

//function caliculation

unsigned const sin_table_size = 2000;
double sin_table[2000];
void generate_sin_table(){
#ifdef USE_FAST_CALICULATE
	unsigned division = sin_table_size;
	for(unsigned i = 0; i < division;i++){
		double cal_radian = M_PI / ((double)sin_table_size-1) * i;
		double val = sin(cal_radian);
		sin_table[i] = val;	
	}
#endif
}

void test_sin_table(){
	for(int i = 0; i < 1999;i++){
		UartPut_I((int)(sin_table[i]*1000000),7);
		delay_ms(80);
	}
}

double from_sin_table(double radian){
	long cycles = 0;
	if(radian > M_PI) cycles = (long)(radian / M_PI);
	double pi_radian = radian - (double)cycles*M_PI;
	int loc = round((double)pi_radian * 636.3014624813976);
	double val = sin_table[loc];
	if(cycles % 2 != 0) val = -(double)val;
	return val;
}
double get_saw_value_simple(double x)
{	
    double fixed_x = x - (double)((int)(x / M_2PI) * M_2PI);
    if (0 <= fixed_x && fixed_x < M_PI_2) return M_2_PI * fixed_x;
    else if (M_PI_2 <= fixed_x && fixed_x < 3.0 * M_PI_2) return -M_2_PI * fixed_x + 2;
    else return M_2_PI * fixed_x - 4;
}

double get_saw_value(double time, double angle_frequency, double initial_phase)
{
#ifndef USE_FAST_CALICULATE
    return asin(sin(time * angle_frequency + initial_phase)) / M_PI * 2 + 1;
#else
	return get_saw_value_simple(time*angle_frequency+initial_phase) + 1;
#endif
}

double get_sin_value(double time, double angle_frequency, double initial_phase, double amplitude)
{
#ifndef USE_FAST_CALICULATE
    return (sin(time * angle_frequency + initial_phase) + 1) * amplitude;
#else
	return (from_sin_table(time * angle_frequency + initial_phase) + 1) * amplitude;
#endif
}


double get_pwm_value(double sin_value, double saw_value)
{
    double diff = round(fabs(sin_value)*1000000)/1000000 - round(fabs(saw_value)*1000000)/1000000;
	if(diff > 0) return 1;
	else return -1;
	/*
    if (diff > 0)
    {
		if (sin_value < 0) return -1;
		else return 1;
    }
    else return 0;
	*/
}

double mod_d(double a,double b){
	long div = (long)(a / b);
	return a - (double)(div * b);
}

double get_Amplitude(double freq){
	double max_freq=60,rate=0.4,init=0.6;
	if(freq > max_freq) return 1.0;
	if(freq <= 0.1) return 0.0;

	return rate / max_freq * freq + init;
}


//sin value definitions
double sin_angle_freq = 0;
double sin_time = 0;

//saw value definitions
double saw_angle_freq = 1050;
double saw_time = 0;

int saw_in_sync_mode = -1;
int random_freq_move_count = 0;

void reset_all_variables(){
	sin_angle_freq = 0;
	sin_time = 0;

	//saw value definitions
	saw_angle_freq = 1050;
	saw_time = 0;
	saw_in_sync_mode = -1;
	
	random_freq_move_count=0;
}

Wave_Values caliculate_E231(bool brake,double initial_phase)
{
    double sin_freq = sin_angle_freq / 2.0 / M_PI;

    double amplitude = get_Amplitude(sin_freq);

    
    double sin_value,saw_value, pwm_value,expect_saw_angle_freq;
    if (sin_freq > 67) saw_in_sync_mode = 1;
    else if (sin_freq > 60) saw_in_sync_mode = 3;
    else if (49 <= sin_freq && sin_freq <= 60)
    {
		double expect_saw_freq = 710 + (1750 - 710) / 11 * (sin_freq - 49);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		saw_in_sync_mode = -1;
    }
    else if (23 <= sin_freq && sin_freq < 50)
    {
		double expect_saw_freq = 1045 + (710 - 1045) / 26 * (sin_freq - 23);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		saw_in_sync_mode = -1;
    }
    else
    {
		expect_saw_angle_freq = 1045 * M_2PI;
		saw_in_sync_mode = -1;
    }
	
	if(saw_in_sync_mode == -1) saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else{
		expect_saw_angle_freq = sin_angle_freq * saw_in_sync_mode;
		saw_time = sin_time;
	}
	saw_angle_freq = expect_saw_angle_freq;
	
	sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
	saw_value = get_saw_value(saw_time, saw_angle_freq,initial_phase * ((saw_in_sync_mode>0) ? saw_in_sync_mode : 1));
    pwm_value = get_pwm_value(sin_value, saw_value);

    Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
    return wv;


}

Wave_Values caliculate_207(bool brake,double initial_phase)
{
    double sin_freq = sin_angle_freq / M_2PI;

    double amplitude = get_Amplitude(sin_freq);
    

    double saw_value, pwm_value,expect_saw_angle_freq,sin_value;

    if (60 <= sin_freq) saw_in_sync_mode = 1;
    else if (53 <= sin_freq) saw_in_sync_mode = 3;
    else if (44 <= sin_freq) saw_in_sync_mode = 5;
    else if (31 <= sin_freq) saw_in_sync_mode = 10;
    else if (14 <= sin_freq) saw_in_sync_mode = 15;
    else if( sin_freq < 14 && !brake)
    {
		expect_saw_angle_freq = M_2PI * 365;
		saw_in_sync_mode = -1;
    }else if( 8 < sin_freq && sin_freq < 14 && brake)
    {

		expect_saw_angle_freq = sin_angle_freq * 27;
		saw_in_sync_mode = 27;
		saw_time = sin_time;
    }
    else
    {
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
    }

	if(saw_in_sync_mode == -1) saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else{
		expect_saw_angle_freq = sin_angle_freq * saw_in_sync_mode;
		saw_time = sin_time;
	}
	saw_angle_freq = expect_saw_angle_freq;
		
	sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
	saw_value = get_saw_value(saw_time, saw_angle_freq,initial_phase * ((saw_in_sync_mode>0) ? saw_in_sync_mode : 1));
    pwm_value = get_pwm_value(sin_value, saw_value);

    Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
    return wv;
}


Wave_Values caliculate_doremi(bool brake,double initial_phase){
	double sin_freq = sin_angle_freq / M_2PI;

    double amplitude = get_Amplitude(sin_freq);

    double sin_value, saw_value = 0, pwm_value,expect_saw_angle_freq = 0;
    if (80 <= sin_freq) saw_in_sync_mode = 1;
    else if (57 <= sin_freq) saw_in_sync_mode = 3;
    else if (50 <= sin_freq) saw_in_sync_mode = 3;
    else if (43 <= sin_freq) saw_in_sync_mode = 5;
    else if (35 <= sin_freq) saw_in_sync_mode = 7;
    else if (30 <= sin_freq) saw_in_sync_mode = 12;
    else if (27 <= sin_freq) saw_in_sync_mode = 15;
    else if (24.5 <= sin_freq) saw_in_sync_mode = 18;
    else
    {
		if (!brake)
		{
			if (5.6 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 400;
				saw_in_sync_mode = -1;
			}
			else if (5 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 350;
				saw_in_sync_mode = -1;
			}
			else if (4.3 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 311;
				saw_in_sync_mode = -1;
			}
			else if (3.4 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 294;
				saw_in_sync_mode = -1;
			}
			else if (2.7 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 262;
				saw_in_sync_mode = -1;
			}
			else if (2.0 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 233;
				saw_in_sync_mode = -1;
			}
			else if (1.5 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 223;
				saw_in_sync_mode = -1;
			}
			else if (0.5 <= sin_freq)
			{
				expect_saw_angle_freq = 2 * M_PI * 196;
				saw_in_sync_mode = -1;
			}
			else
			{
				expect_saw_angle_freq = 2 * M_PI * 175;
				saw_in_sync_mode = -1;
			}
		}
		else
		{
			if (sin_freq > 4)
			{
				expect_saw_angle_freq = 2 * M_PI * 400;
				saw_in_sync_mode = -1;
			}
			else
			{
				Wave_Values wv;
				wv.sin_value = 0;
				wv.saw_value = 0;
				wv.pwm_value = 0;
				return wv;
			}
		}
	}

	if(saw_in_sync_mode == -1) saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else{
		expect_saw_angle_freq = sin_angle_freq * saw_in_sync_mode;
		saw_time = sin_time;
	}
	saw_angle_freq = expect_saw_angle_freq;

	sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
	saw_value = get_saw_value(saw_time, saw_angle_freq, initial_phase * ((saw_in_sync_mode>0) ? saw_in_sync_mode : 1));
    pwm_value = get_pwm_value(sin_value, saw_value);

    Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
    return wv;


}

Wave_Values caliculate_E235(bool brake,double initial_phase)
{
    double sin_freq = sin_angle_freq / M_2PI;

    double amplitude = get_Amplitude(sin_freq);

    double sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
    double saw_value, pwm_value;
    if (sin_freq > 54)
    {
		
		saw_angle_freq = sin_angle_freq * 15;
		saw_time = sin_time;
		saw_in_sync_mode = 15;		
		
		saw_value = get_saw_value(saw_time, saw_angle_freq,initial_phase * ((saw_in_sync_mode>0) ? saw_in_sync_mode : 1));
		
    }
    else{
		
		if(random_freq_move_count==0){
			//saw_freq = 740;
			double random_v = random();
			double diff_freq = mod_d(random_v,100.0);
			if(mod_d(random_v,500.0) < 250) diff_freq = -diff_freq;
			
			double base_freq = (double)550 + 3.148148148148148*(sin_freq);//170.0/54.0*(sin_freq);
			
			double silent_random_freq = base_freq + diff_freq;
			
			double expect_saw_angle_freq = 2 * M_PI * silent_random_freq;
			saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
			saw_angle_freq = expect_saw_angle_freq;
		}
		saw_value = get_saw_value(saw_time, saw_angle_freq, 0);
		
		random_freq_move_count++;
		if(random_freq_move_count == 30) random_freq_move_count=0;
		saw_in_sync_mode = -1;
	}

    pwm_value = get_pwm_value(sin_value, saw_value);

    Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
    return wv;
}

Wave_Values caliculate_E209(bool brake, double initial_phase)
{
    double sin_freq = sin_angle_freq / M_2PI;
    double amplitude = get_Amplitude(sin_freq);

    double sin_value, saw_value, pwm_value, expect_saw_angle_freq;

    if (53 <= sin_freq) saw_in_sync_mode = 1;
    else if (46 <= sin_freq) saw_in_sync_mode = 3;
    else if (30 <= sin_freq) saw_in_sync_mode = 9;
    else if (19 <= sin_freq) saw_in_sync_mode = 21;
    else if (9 <= sin_freq) saw_in_sync_mode = 33;
    else if (2 <= sin_freq && !brake) saw_in_sync_mode = 57;
    else if (sin_freq < 2 && !brake)
    {
		saw_in_sync_mode = -1;
		expect_saw_angle_freq = M_2PI * 114;
    }
    else if (8 < sin_freq && sin_freq < 18 && brake) saw_in_sync_mode = 33;
    else
    {	
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
    }

    if (saw_in_sync_mode == -1) saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else{
		expect_saw_angle_freq = sin_angle_freq * saw_in_sync_mode;
		saw_time = sin_time;
	}
    saw_angle_freq = expect_saw_angle_freq;

	sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
    saw_value = get_saw_value(saw_time, saw_angle_freq, initial_phase * ((saw_in_sync_mode > 0) ? saw_in_sync_mode : 1));
    pwm_value = get_pwm_value(sin_value, saw_value);

	Wave_Values wv;
    wv.sin_value = sin_value;
    wv.saw_value = saw_value;
    wv.pwm_value = pwm_value;
    return wv;
}

Wave_Values caliculate_silent(bool brake,double initial_phase)
{
    double sin_freq = sin_angle_freq / M_2PI;

    double amplitude = get_Amplitude(sin_freq);

    double sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
    double saw_value, pwm_value;
    if (sin_freq > 45)
    {
		
		saw_angle_freq = sin_angle_freq * 20;
		saw_time = sin_time;
		saw_in_sync_mode = 20;		
		
		saw_value = get_saw_value(saw_time, saw_angle_freq,initial_phase * ((saw_in_sync_mode>0) ? saw_in_sync_mode : 1));
		
    }
    else{
		
		if(random_freq_move_count==0){
			//saw_freq = 740;
			
			double diff_freq = mod_d(random(),200.0);
			if(mod_d(random(),500.0) < 250) diff_freq = -diff_freq;
			double silent_random_freq = 550 + diff_freq;
			
			double expect_saw_angle_freq = 2 * M_PI * silent_random_freq;
			saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
			saw_angle_freq = expect_saw_angle_freq;
		}
		saw_value = get_saw_value(saw_time, saw_angle_freq, 0);
		
		random_freq_move_count++;
		if(random_freq_move_count == 30) random_freq_move_count=0;
		saw_in_sync_mode = -1;
	}

    pwm_value = get_pwm_value(sin_value, saw_value);

    Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
    return wv;
}