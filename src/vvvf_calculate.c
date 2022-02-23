#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "my_math.h"
#include "vvvf_calculate.h"
#include "settings.h"

#include "rpi_lib/gpio.h"

//function caliculation
double get_saw_value_simple(double x)
{
	double fixed_x = x - (double)((int)(x * M_1_2PI) * M_2PI);
	if (0 <= fixed_x && fixed_x < M_PI_2)
		return M_2_PI * fixed_x;
	else if (M_PI_2 <= fixed_x && fixed_x < 3.0 * M_PI_2)
		return -M_2_PI * fixed_x + 2;
	else
		return M_2_PI * fixed_x - 4;
}

double get_saw_value(double time, double angle_frequency, double initial_phase)
{
	return -get_saw_value_simple(time * angle_frequency + initial_phase);
}

double get_sin_value(double time, double angle_frequency, double initial_phase, double amplitude)
{
	return sin(time * angle_frequency + initial_phase) * amplitude;
}

char get_pwm_value(double sin_value, double saw_value)
{
	if (disconnect)
		return 0;
	if (sin_value - saw_value > 0)
		return 1;
	else
		return 0;
}

char get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage, bool saw_oppose)
{
	double sin = get_sin_value(time, angle_frequency, initial_phase, 1);
	double saw = get_saw_value(time, angle_frequency, initial_phase);
	if (saw_oppose)
		saw = -saw;
	double pwm = ((sin - saw > 0) ? 1 : -1) * voltage;
	double nega_saw = (saw > 0) ? saw - 1 : saw + 1;
	char gate = get_pwm_value(pwm, nega_saw) << 1;
	return gate;
}

char get_P_with_saw(double time, double sin_angle_frequency, double initial_phase, double voltage, double carrier_mul, bool saw_oppose)
{
	double carrier_saw = -get_saw_value(time, carrier_mul * sin_angle_frequency, carrier_mul * initial_phase);
	double saw = -get_saw_value(time, sin_angle_frequency, initial_phase);
	if (saw_oppose)
		saw = -saw;
	double pwm = (saw > 0) ? voltage : -voltage;
	char gate = get_pwm_value(pwm, carrier_saw) << 1;
	return gate;
}

double get_Amplitude(double freq, double max_freq)
{

	double rate = 0.99, init = 0.01;
	if (freq > max_freq)
		return 1.0;
	if (freq <= 0.1)
		return 0.0;

	return rate / max_freq * freq + init;
}

int get_Pulse_Num(Pulse_Mode mode)
{
	if (mode == Not_In_Sync)
		return -1;
	if (mode == P_1)
		return 0;
	if (mode == P_Wide_3)
		return 0;
	if (mode == P_5)
		return 6;
	if (mode == P_7)
		return 9;
	if (mode == P_10)
		return 10;
	if (mode == P_11)
		return 15;
	if (mode == P_12)
		return 12;
	if (mode == P_18)
		return 18;
	if ((int)mode <= (int)P_61)
		return 3 + (2 * ((int)mode - 6));

	return get_Pulse_Num((Pulse_Mode)((int)mode - 35));
}

//sin value definitions
double sin_angle_freq = 0;
double sin_time = 0;

//saw value definitions
double saw_angle_freq = 1050;
double saw_time = 0;
int pre_saw_random_freq = 0;

bool disconnect = false;

int random_freq_move_count = 0;

void reset_all_variables()
{
	sin_angle_freq = 0;
	sin_time = 0;

	//saw value definitions
	saw_angle_freq = 1050;
	saw_time = 0;

	random_freq_move_count = 0;
}

// random range => -range ~ range
int get_random_freq(int base_freq, int range)
{
	int random_freq = 0;
	if (random_freq_move_count == 0 || pre_saw_random_freq == 0)
	{
		int random_v = my_random();
		int diff_freq = mod_i(random_v, range);
		if (random_v & 0x01)
			diff_freq = -diff_freq;
		int silent_random_freq = base_freq + diff_freq;
		random_freq = silent_random_freq;
		pre_saw_random_freq = silent_random_freq;
	}
	else
	{
		random_freq = pre_saw_random_freq;
	}
	random_freq_move_count++;
	if (random_freq_move_count >= 100)
		random_freq_move_count = 0;
	return random_freq;
}

double get_pattern_random(int lowest, int highest, int interval_count)
{
	double random_freq = 0;
	if (random_freq_move_count < interval_count * 0.5)
		random_freq = lowest + (highest - lowest) / (interval_count * 0.5) * random_freq_move_count;
	else
		random_freq = highest + (lowest - highest) / (interval_count * 0.5) * (random_freq_move_count - interval_count * 0.5);
	if (++random_freq_move_count > interval_count)
		random_freq_move_count = 0;
	return random_freq;
}

char calculate_three_level(Pulse_Mode pulse_mode, double expect_saw_angle_freq, double initial_phase, double amplitude, double dipolar)
{
	if (pulse_mode == Not_In_Sync)
		saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else
	{
		expect_saw_angle_freq = sin_angle_freq * get_Pulse_Num(pulse_mode);
		saw_time = sin_time;
	}
	saw_angle_freq = expect_saw_angle_freq;

	double sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);

	double saw_value = get_saw_value(saw_time, saw_angle_freq, 0);
	if ((int)pulse_mode > (int)P_61)
		saw_value = -saw_value;

	double changed_saw = (dipolar != -1) ? dipolar : 0.5 * saw_value;
	int pwm_value = get_pwm_value(sin_value, changed_saw + 0.5) + get_pwm_value(sin_value, changed_saw - 0.5);

	return (char)pwm_value;
}

char calculate_two_level(Pulse_Mode pulse_mode, double expect_saw_angle_freq, double initial_phase, double amplitude)
{

	switch ((int)pulse_mode)
	{
	case P_Wide_3:
		return get_Wide_P_3(sin_time, sin_angle_freq, initial_phase, amplitude, false);
	case SP_Wide_3:
		return get_Wide_P_3(sin_time, sin_angle_freq, initial_phase, amplitude, true);
	case P_5:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_5:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);
	case P_7:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_7:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);
	case P_11:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_11:
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);
	}

	if (pulse_mode == Not_In_Sync)
		saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
	else
	{
		expect_saw_angle_freq = sin_angle_freq * get_Pulse_Num(pulse_mode);
		saw_time = sin_time;
	}
	saw_angle_freq = expect_saw_angle_freq;

	double sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);

	double saw_value = get_saw_value(saw_time, saw_angle_freq, 0);
	if ((int)pulse_mode > (int)P_61)
		saw_value = -saw_value;

	char pwm_value = get_pwm_value(sin_value, saw_value) << 1;

	return pwm_value;
}