#include "vvvf_calculate.h"
#include "vvvf_wave.h"

#include "rpi_lib/gpio.h"
#include "my_math.h"
#include "settings.h"
#include "my_switchingangle.h"

// function caliculation
double get_saw_value(double x)
{
	double result;
	double fixed_x = x - (double)((int)(x * M_1_2PI) * M_2PI);
	if (0 <= fixed_x && fixed_x < M_PI_2)
		result = M_2_PI * fixed_x;
	else if (M_PI_2 <= fixed_x && fixed_x < 3.0 * M_PI_2)
		result = -M_2_PI * fixed_x + 2;
	else
		result = M_2_PI * fixed_x - 4;
	return -result;
}

double get_sin_value(double x, double amplitude)
{
	return sin(x) * amplitude;
}

char get_pwm_value(double sin_value, double saw_value)
{
	if (sin_value - saw_value > 0)
		return 1;
	else
		return 0;
}

char get_Wide_P_3(double x, double voltage, bool saw_oppose)
{
	double sin = get_sin_value(x, 1);
	double saw = get_saw_value(x);
	if (saw_oppose)
		saw = -saw;
	double pwm = ((sin - saw > 0) ? 1 : -1) * voltage;
	double nega_saw = (saw > 0) ? saw - 1 : saw + 1;
	char gate = get_pwm_value(pwm, nega_saw) << 1;
	return gate;
}

char get_P_with_saw(double x, double voltage, double carrier_mul, bool saw_oppose)
{
	double carrier_saw = -get_saw_value(carrier_mul * x );
	double saw = -get_saw_value(x);
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
	double x)
{
	double theta = x - (double)((int)(x * M_1_2PI) * M_2PI);

	int PWM_OUT = (((((theta <= alpha2) && (theta >= alpha1)) || ((theta <= alpha4) && (theta >= alpha3)) || ((theta <= alpha6) && (theta >= alpha5)) || ((theta <= M_PI - alpha1) && (theta >= M_PI - alpha2)) || ((theta <= M_PI - alpha3) && (theta >= M_PI - alpha4)) || ((theta <= M_PI - alpha5) && (theta >= M_PI - alpha6))) && ((theta <= M_PI) && (theta >= 0))) || (((theta <= M_PI - alpha7) && (theta >= alpha7)) && ((theta <= M_PI) && (theta >= 0)))) || ((!(((theta <= alpha2 + M_PI) && (theta >= alpha1 + M_PI)) || ((theta <= alpha4 + M_PI) && (theta >= alpha3 + M_PI)) || ((theta <= alpha6 + M_PI) && (theta >= alpha5 + M_PI)) || ((theta <= M_2PI - alpha1) && (theta >= M_2PI - alpha2)) || ((theta <= M_2PI - alpha3) && (theta >= M_2PI - alpha4)) || ((theta <= M_2PI - alpha5) && (theta >= M_2PI - alpha6))) && ((theta <= M_2PI) && (theta >= M_PI))) && !((theta <= M_2PI - alpha7) && (theta >= M_PI + alpha7)) && (theta <= M_2PI) && (theta >= M_PI)) ? 1 : -1;

	int gate = flag == 'A' ? -PWM_OUT + 1 : PWM_OUT + 1;
	return (char)gate;
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
int pre_saw_random_freq = 0;
int random_freq_move_count = 0;
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

char calculate_three_level(Calculate_Values *cv, Carrier_Settings *cs, double initial_phase)
{
	double _saw_time , _saw_angle_freq , _sine_time  = cv->sin_time, _sine_angle_freq = cv-> sin_angle_freq;

	if (cs->pulse_mode == Async){
		_saw_time = cv->saw_angle_freq / cs->carrier_angle_freq * cv->saw_time;
		_saw_angle_freq = cv-> saw_angle_freq;
	}
	else
	{
		_saw_time = cv-> sin_time;
		_saw_angle_freq = cv->sin_angle_freq * get_Pulse_Num(cs->pulse_mode);
	}

	cv-> saw_time = _saw_time;
	cv-> saw_angle_freq = _saw_angle_freq;

	double sin_value = get_sin_value(_sine_time * _sine_angle_freq + initial_phase, cs->amplitude);
	double saw_value = get_saw_value(_saw_time * _saw_angle_freq + initial_phase);

	if ((int)cs->pulse_mode > (int)P_61)
		saw_value = -saw_value;

	double changed_saw = (cs->dipolar != -1) ? cs->dipolar : 0.5 * saw_value;
	int pwm_value = get_pwm_value(sin_value, changed_saw + 0.5) + get_pwm_value(sin_value, changed_saw - 0.5);

	return (char)pwm_value;
}

char calculate_two_level(Calculate_Values* cv, Carrier_Settings* cs, double initial_phase)
{
	
	double _saw_time , _saw_angle_freq , _sine_time  = cv->sin_time, _sine_angle_freq = cv->sin_angle_freq;
	double _sine_x = _sine_angle_freq * _sine_time + initial_phase;

	double amplitude = cs->amplitude;
	Pulse_Mode pulse_mode = cs->pulse_mode;

	switch ((int)pulse_mode)
	{
	case P_Wide_3:
		return get_Wide_P_3(_sine_x, amplitude, false);
	case SP_Wide_3:
		return get_Wide_P_3(_sine_x, amplitude, true);
	case P_5:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_5:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), true);
	case P_7:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_7:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), true);
	case P_11:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), false);
	case SP_11:
		return get_P_with_saw(_sine_x, amplitude, get_Pulse_Num(pulse_mode), true);
	}

	if (pulse_mode == CHMP_15){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_7Alpha[array_l][0] * M_PI_180,
			_7Alpha[array_l][1] * M_PI_180,
			_7Alpha[array_l][2] * M_PI_180,
			_7Alpha[array_l][3] * M_PI_180,
			_7Alpha[array_l][4] * M_PI_180,
			_7Alpha[array_l][5] * M_PI_180,
			_7Alpha[array_l][6] * M_PI_180,
			_7Alpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Old_15){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_7Alpha_Old[array_l][0] * M_PI_180,
			_7Alpha_Old[array_l][1] * M_PI_180,
			_7Alpha_Old[array_l][2] * M_PI_180,
			_7Alpha_Old[array_l][3] * M_PI_180,
			_7Alpha_Old[array_l][4] * M_PI_180,
			_7Alpha_Old[array_l][5] * M_PI_180,
			_7Alpha_Old[array_l][6] * M_PI_180,
			_7OldAlpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_15){
		int array_l = (int)(1000 * amplitude) - 999;
		return get_P_with_switchingangle(
			_7WideAlpha[array_l][0] * M_PI_180,
			_7WideAlpha[array_l][1] * M_PI_180,
			_7WideAlpha[array_l][2] * M_PI_180,
			_7WideAlpha[array_l][3] * M_PI_180,
			_7WideAlpha[array_l][4] * M_PI_180,
			_7WideAlpha[array_l][5] * M_PI_180,
			_7WideAlpha[array_l][6] * M_PI_180,
			'B', _sine_x);
	}
		
	if (pulse_mode == CHMP_13){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_6Alpha[array_l][0] * M_PI_180,
			_6Alpha[array_l][1] * M_PI_180,
			_6Alpha[array_l][2] * M_PI_180,
			_6Alpha[array_l][3] * M_PI_180,
			_6Alpha[array_l][4] * M_PI_180,
			_6Alpha[array_l][5] * M_PI_180,
			M_PI_2,
			_6Alpha_Polary[array_l], _sine_x);
	}
	if (pulse_mode == CHMP_Old_13){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_6Alpha_Old[array_l][0] * M_PI_180,
			_6Alpha_Old[array_l][1] * M_PI_180,
			_6Alpha_Old[array_l][2] * M_PI_180,
			_6Alpha_Old[array_l][3] * M_PI_180,
			_6Alpha_Old[array_l][4] * M_PI_180,
			_6Alpha_Old[array_l][5] * M_PI_180,
			M_PI_2,
			_6OldAlpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_13){
		int array_l = (int)(1000 * amplitude) - 999;
		return get_P_with_switchingangle(
			_6WideAlpha[array_l][0] * M_PI_180,
			_6WideAlpha[array_l][1] * M_PI_180,
			_6WideAlpha[array_l][2] * M_PI_180,
			_6WideAlpha[array_l][3] * M_PI_180,
			_6WideAlpha[array_l][4] * M_PI_180,
			_6WideAlpha[array_l][5] * M_PI_180,
			M_PI_2,
			'A', _sine_x);
	}
		
	if (pulse_mode == CHMP_11){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_5Alpha[array_l][0] * M_PI_180,
			_5Alpha[array_l][1] * M_PI_180,
			_5Alpha[array_l][2] * M_PI_180,
			_5Alpha[array_l][3] * M_PI_180,
			_5Alpha[array_l][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			_5Alpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Old_11){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_5Alpha_Old[array_l][0] * M_PI_180,
			_5Alpha_Old[array_l][1] * M_PI_180,
			_5Alpha_Old[array_l][2] * M_PI_180,
			_5Alpha_Old[array_l][3] * M_PI_180,
			_5Alpha_Old[array_l][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			_5OldAlpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_11){
		int array_l = (int)(1000 * amplitude) - 999;
		return get_P_with_switchingangle(
			_5WideAlpha[array_l][0] * M_PI_180,
			_5WideAlpha[array_l][1] * M_PI_180,
			_5WideAlpha[array_l][2] * M_PI_180,
			_5WideAlpha[array_l][3] * M_PI_180,
			_5WideAlpha[array_l][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			'B', _sine_x);
	}
		
	if (pulse_mode == CHMP_9){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_4Alpha[array_l][0] * M_PI_180,
			_4Alpha[array_l][1] * M_PI_180,
			_4Alpha[array_l][2] * M_PI_180,
			_4Alpha[array_l][3] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_4Alpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_9){
		int array_l = (int)(1000 * amplitude) - 799;
		return get_P_with_switchingangle(
			_4WideAlpha[array_l][0] * M_PI_180,
			_4WideAlpha[array_l][1] * M_PI_180,
			_4WideAlpha[array_l][2] * M_PI_180,
			_4WideAlpha[array_l][3] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', _sine_x);
	}
		
	if (pulse_mode == CHMP_7){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_3Alpha[array_l][0] * M_PI_180,
			_3Alpha[array_l][1] * M_PI_180,
			_3Alpha[array_l][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_3Alpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_7){
		int array_l = (int)(1000 * amplitude) - 799;
		return get_P_with_switchingangle(
			_3WideAlpha[array_l][0] * M_PI_180,
			_3WideAlpha[array_l][1] * M_PI_180,
			_3WideAlpha[array_l][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', _sine_x);
	}
		
	if (pulse_mode == CHMP_5){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_2Alpha[array_l][0] * M_PI_180,
			_2Alpha[array_l][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			_2Alpha_Polary[array_l], _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_5){
		int array_l = (int)(1000 * amplitude) - 799;
		return get_P_with_switchingangle(
			_2WideAlpha[array_l][0] * M_PI_180,
			_2WideAlpha[array_l][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', _sine_x);
	}
		
	if (pulse_mode == CHMP_Wide_3){
		return get_P_with_switchingangle(
			_WideAlpha[(int)(500 * amplitude) + 1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', _sine_x);
	}
		
	if (pulse_mode == SHEP_3){
		return get_P_with_switchingangle(
			_1Alpha_SHE[(int)(1000 * amplitude) + 1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', _sine_x);
	}
		
	if (pulse_mode == SHEP_5){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_2Alpha_SHE[array_l][0] * M_PI_180,
			_2Alpha_SHE[array_l][1] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'A', _sine_x);
	}
		
	if (pulse_mode == SHEP_7){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_3Alpha_SHE[array_l][0] * M_PI_180,
			_3Alpha_SHE[array_l][1] * M_PI_180,
			_3Alpha_SHE[array_l][2] * M_PI_180,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			M_PI_2,
			'B', _sine_x);
	}
		
	if (pulse_mode == SHEP_11){
		int array_l = (int)(1000 * amplitude) + 1;
		return get_P_with_switchingangle(
			_5Alpha_SHE[array_l][0] * M_PI_180,
			_5Alpha_SHE[array_l][1] * M_PI_180,
			_5Alpha_SHE[array_l][2] * M_PI_180,
			_5Alpha_SHE[array_l][3] * M_PI_180,
			_5Alpha_SHE[array_l][4] * M_PI_180,
			M_PI_2,
			M_PI_2,
			'A', _sine_x);
	}

		

	if (pulse_mode == Async || pulse_mode == Async_THI)
	{
		_saw_time = cv->saw_angle_freq / cs->carrier_angle_freq * cv->saw_time;
		_saw_angle_freq = cs->carrier_angle_freq;

		cv->saw_time = _saw_time;
		cv->saw_angle_freq = _saw_angle_freq;

		double sin_value;
		if (pulse_mode == Async_THI)
			sin_value = get_sin_value(_sine_x, amplitude) + 0.2 * get_sin_value(3 * _sine_x, amplitude);
		else
			sin_value = get_sin_value(_sine_x, amplitude);

		double saw_value = get_saw_value(_saw_time * _saw_angle_freq + 0);
		char pwm_value = get_pwm_value(sin_value, saw_value) << 1;

		return pwm_value;
	}

	else
	{
		_saw_time = _sine_time;
		_saw_angle_freq = _sine_angle_freq;

		cv->saw_time = _saw_time;
		cv->saw_angle_freq = _saw_angle_freq;

		int pulse = get_Pulse_Num(pulse_mode);

		double sin_value = get_sin_value(_sine_x, amplitude);
		double saw_value = get_saw_value( pulse * (_saw_time * _saw_angle_freq + initial_phase));
		if ((int)pulse_mode > (int)P_61)
			saw_value = -saw_value;

		char pwm_value = get_pwm_value(sin_value, saw_value) << 1;

		return pwm_value;
	}
}