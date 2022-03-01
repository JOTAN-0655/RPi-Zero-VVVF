#ifndef VVVF_CALCULATE_H
#define VVVF_CALCULATE_H

#include "rpi_lib/gpio.h"
#include "vvvf_struct.h"


// VARIABLE PUBLIC
extern int random_freq_move_count;
extern int pre_saw_random_freq;

double get_saw_value(double x);
double get_sin_value(double x, double amplitude);
char get_pwm_value(double sin_value, double saw_value);

double get_Amplitude(double freq, double max_freq);

// PULSE GEN
char get_P_with_saw(double x, double voltage, double carrier_mul, bool saw_oppose);
char get_Wide_P_3(double x, double voltage, bool saw_oppose);
char get_P_with_switchingangle(
	double alpha1,
	double alpha2,
	double alpha3,
	double alpha4,
	double alpha5,
	double alpha6,
	double alpha7,
	int flag,
	double x);

int get_Pulse_Num(Pulse_Mode mode);
int get_random_freq(int base_freq, int range);
double get_pattern_random(int lowest, int highest, int interval_count);

char calculate_two_level(Calculate_Values*, Carrier_Settings*, double initial_phase);
char calculate_three_level(Calculate_Values *, Carrier_Settings*, double initial_phase);
#endif