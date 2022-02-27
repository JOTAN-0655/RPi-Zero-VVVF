#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "my_math.h"
#include "vvvf_calculate.h"
#include "settings.h"
#include "my_switchingangle.h"

#include "rpi_lib/gpio.h"

// function caliculation
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

double get_saw_value(double x)
{
	return -get_saw_value_simple(x);
}

double get_sin_value(double x, double amplitude)
{
	return sin(x) * amplitude;
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

char get_P_with_switchingangle(
	double alpha1,
	double alpha2,
	double alpha3,
	double alpha4,
	double alpha5,
	double alpha6,
	double alpha7,
	int flag,
	double time, double sin_angle_frequency, double initial_phase)
{
	double theta = (initial_phase + time * sin_angle_frequency) - (double)((int)((initial_phase + time * sin_angle_frequency) * M_1_2PI) * M_2PI);

	int PWM_OUT = (((((theta <= alpha2) && (theta >= alpha1)) || ((theta <= alpha4) && (theta >= alpha3)) || ((theta <= alpha6) && (theta >= alpha5)) || ((theta <= M_PI - alpha1) && (theta >= M_PI - alpha2)) || ((theta <= M_PI - alpha3) && (theta >= M_PI - alpha4)) || ((theta <= M_PI - alpha5) && (theta >= M_PI - alpha6))) && ((theta <= M_PI) && (theta >= 0))) || (((theta <= M_PI - alpha7) && (theta >= alpha7)) && ((theta <= M_PI) && (theta >= 0)))) || ((!(((theta <= alpha2 + M_PI) && (theta >= alpha1 + M_PI)) || ((theta <= alpha4 + M_PI) && (theta >= alpha3 + M_PI)) || ((theta <= alpha6 + M_PI) && (theta >= alpha5 + M_PI)) || ((theta <= M_2PI - alpha1) && (theta >= M_2PI - alpha2)) || ((theta <= M_2PI - alpha3) && (theta >= M_2PI - alpha4)) || ((theta <= M_2PI - alpha5) && (theta >= M_2PI - alpha6))) && ((theta <= M_2PI) && (theta >= M_PI))) && !((theta <= M_2PI - alpha7) && (theta >= M_PI + alpha7)) && (theta <= M_2PI) && (theta >= M_PI)) ? 1 : -1;

	int gate = flag == 'A' ? -PWM_OUT + 1 : PWM_OUT + 1;
	return (char)gate << 1;
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
	if (mode == Async)
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
		return 3 + (2 * ((int)mode - (int)P_3));

	return get_Pulse_Num((Pulse_Mode)((int)mode - (int)P_61));
}

// sin value definitions
double sin_angle_freq = 0;
double sin_time = 0;

// saw value definitions
double saw_angle_freq = 1050;
double saw_time = 0;
int pre_saw_random_freq = 0;

bool disconnect = false;

int random_freq_move_count = 0;

void reset_all_variables()
{
	sin_angle_freq = 0;
	sin_time = 0;

	// saw value definitions
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
	if (pulse_mode == Async)
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

	if (pulse_mode == CHMP_15)
		return get_P_with_switchingangle(
			_7Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][5] * M_PI_180,
			_7Alpha[(int)(1000 * amplitude) + 1][6] * M_PI_180,
			_7Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Old_15)
		return get_P_with_switchingangle(
			_7Alpha_Old[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][5] * M_PI_180,
			_7Alpha_Old[(int)(1000 * amplitude) + 1][6] * M_PI_180,
			_7OldAlpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_15)
		return get_P_with_switchingangle(
			_7WideAlpha[(int)(1000 * amplitude) - 999][0] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][1] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][2] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][3] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][4] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][5] * M_PI_180,
			_7WideAlpha[(int)(1000 * amplitude) - 999][6] * M_PI_180,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_13)
		return get_P_with_switchingangle(
			_6Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_6Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_6Alpha[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_6Alpha[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_6Alpha[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			_6Alpha[(int)(1000 * amplitude) + 1][5] * M_PI_180,
			M_PI_2,
			_6Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Old_13)
		return get_P_with_switchingangle(
			_6Alpha_Old[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_6Alpha_Old[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_6Alpha_Old[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_6Alpha_Old[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_6Alpha_Old[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			_6Alpha_Old[(int)(1000 * amplitude) + 1][5] * M_PI_180,
			M_PI_2,
			_6OldAlpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_13)
		return get_P_with_switchingangle(
			_6WideAlpha[(int)(1000 * amplitude) - 999][0] * M_PI_180,
			_6WideAlpha[(int)(1000 * amplitude) - 999][1] * M_PI_180,
			_6WideAlpha[(int)(1000 * amplitude) - 999][2] * M_PI_180,
			_6WideAlpha[(int)(1000 * amplitude) - 999][3] * M_PI_180,
			_6WideAlpha[(int)(1000 * amplitude) - 999][4] * M_PI_180,
			_6WideAlpha[(int)(1000 * amplitude) - 999][5] * M_PI_180,
			M_PI_2,
			'A', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_11)
		return get_P_with_switchingangle(
			_5Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_5Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_5Alpha[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_5Alpha[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_5Alpha[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			_5Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Old_11)
		return get_P_with_switchingangle(
			_5Alpha_Old[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_5Alpha_Old[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_5Alpha_Old[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_5Alpha_Old[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_5Alpha_Old[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			_5OldAlpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_11)
		return get_P_with_switchingangle(
			_5WideAlpha[(int)(1000 * amplitude) - 999][0] * M_PI_180,
			_5WideAlpha[(int)(1000 * amplitude) - 999][1] * M_PI_180,
			_5WideAlpha[(int)(1000 * amplitude) - 999][2] * M_PI_180,
			_5WideAlpha[(int)(1000 * amplitude) - 999][3] * M_PI_180,
			_5WideAlpha[(int)(1000 * amplitude) - 999][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_9)
		return get_P_with_switchingangle(
			_4Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_4Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_4Alpha[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_4Alpha[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_4Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_9)
		return get_P_with_switchingangle(
			_4WideAlpha[(int)(1000 * amplitude) - 799][0] * M_PI_180,
			_4WideAlpha[(int)(1000 * amplitude) - 799][1] * M_PI_180,
			_4WideAlpha[(int)(1000 * amplitude) - 799][2] * M_PI_180,
			_4WideAlpha[(int)(1000 * amplitude) - 799][3] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_7)
		return get_P_with_switchingangle(
			_3Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_3Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_3Alpha[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_3Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_7)
		return get_P_with_switchingangle(
			_3WideAlpha[(int)(1000 * amplitude) - 799][0] * M_PI_180,
			_3WideAlpha[(int)(1000 * amplitude) - 799][1] * M_PI_180,
			_3WideAlpha[(int)(1000 * amplitude) - 799][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_5)
		return get_P_with_switchingangle(
			_2Alpha[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_2Alpha[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_2Alpha_Polary[(int)(1000 * amplitude) + 1], sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_5)
		return get_P_with_switchingangle(
			_2WideAlpha[(int)(1000 * amplitude) - 799][0] * M_PI_180,
			_2WideAlpha[(int)(1000 * amplitude) - 799][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == CHMP_Wide_3)
		return get_P_with_switchingangle(
			_WideAlpha[(int)(500 * amplitude) + 1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == SHEP_3)
		return get_P_with_switchingangle(
			_1Alpha_SHE[(int)(1000 * amplitude) + 1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == SHEP_5)
		return get_P_with_switchingangle(
			_2Alpha_SHE[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_2Alpha_SHE[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == SHEP_7)
		return get_P_with_switchingangle(
			_3Alpha_SHE[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_3Alpha_SHE[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_3Alpha_SHE[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', sin_time, sin_angle_freq, initial_phase);
	if (pulse_mode == SHEP_11)
		return get_P_with_switchingangle(
			_5Alpha_SHE[(int)(1000 * amplitude) + 1][0] * M_PI_180,
			_5Alpha_SHE[(int)(1000 * amplitude) + 1][1] * M_PI_180,
			_5Alpha_SHE[(int)(1000 * amplitude) + 1][2] * M_PI_180,
			_5Alpha_SHE[(int)(1000 * amplitude) + 1][3] * M_PI_180,
			_5Alpha_SHE[(int)(1000 * amplitude) + 1][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			'A', sin_time, sin_angle_freq, initial_phase);

	if (pulse_mode == Async)
	{
		saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
		saw_angle_freq = expect_saw_angle_freq;

		double sin_value = get_sin_value(sin_time * sin_angle_freq + initial_phase, amplitude);

		double saw_value = get_saw_value(saw_time * saw_angle_freq + 0);
		if ((int)pulse_mode > (int)P_61)
			saw_value = -saw_value;

		char pwm_value = get_pwm_value(sin_value, saw_value) << 1;

		return pwm_value;
	}

	else
	{
		int pulse_num = get_Pulse_Num(pulse_mode);
		expect_saw_angle_freq = sin_angle_freq * pulse_num;
		saw_time = sin_time;

		saw_angle_freq = expect_saw_angle_freq;

		double sin_value = get_sin_value(sin_time * sin_angle_freq + initial_phase, amplitude);

		double saw_value = get_saw_value(pulse_num * (saw_time * saw_angle_freq + initial_phase));
		if ((int)pulse_mode > (int)P_61)
			saw_value = -saw_value;

		char pwm_value = get_pwm_value(sin_value, saw_value) << 1;

		return pwm_value;
	}
}