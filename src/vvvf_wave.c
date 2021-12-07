#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "my_math.h"

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

int get_pwm_value(double sin_value, double saw_value)
{
	if (disconnect)
		return 0;
	if (sin_value - saw_value > 0)
		return 1;
	else
		return 0;
}

Wave_Values get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage, bool saw_oppose)
{
	double sin = get_sin_value(time, angle_frequency, initial_phase, 1);
	double saw = get_saw_value(time, angle_frequency, initial_phase);
	if (saw_oppose)
		saw = -saw;
	double pwm = ((sin - saw > 0) ? 1 : -1) * voltage;
	double nega_saw = (saw > 0) ? saw - 1 : saw + 1;
	int gate = get_pwm_value(pwm, nega_saw) * 2;
	Wave_Values wv;
	wv.sin_value = pwm;
	wv.saw_value = nega_saw;
	wv.pwm_value = gate;
	return wv;
}

Wave_Values get_P_with_saw(double time, double sin_angle_frequency, double initial_phase, double voltage, double carrier_mul, bool saw_oppose)
{
	double carrier_saw = -get_saw_value(time, carrier_mul * sin_angle_frequency, carrier_mul * initial_phase);
	double saw = -get_saw_value(time, sin_angle_frequency, initial_phase);
	if (saw_oppose)
		saw = -saw;
	double pwm = (saw > 0) ? voltage : -voltage;
	int gate = get_pwm_value(pwm, carrier_saw) * 2;
	Wave_Values wv;
	wv.sin_value = saw;
	wv.saw_value = carrier_saw;
	wv.pwm_value = gate;
	return wv;
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

Wave_Values calculate_three_level(Pulse_Mode pulse_mode, double expect_saw_angle_freq, double initial_phase, double amplitude, bool dipolar)
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

	double changed_saw = (dipolar == 1) ? 2 : 0.5 * saw_value;
	int pwm_value = get_pwm_value(sin_value, changed_saw + 0.5) + get_pwm_value(sin_value, changed_saw - 0.5);

	Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
	return wv;
}

Wave_Values calculate_common(Pulse_Mode pulse_mode, double expect_saw_angle_freq, double initial_phase, double amplitude)
{

	if (pulse_mode == P_Wide_3)
		return get_Wide_P_3(sin_time, sin_angle_freq, initial_phase, amplitude, false);
	if (pulse_mode == SP_Wide_3)
		return get_Wide_P_3(sin_time, sin_angle_freq, initial_phase, amplitude, true);
	if (pulse_mode == P_5)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	if (pulse_mode == SP_5)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);
	if (pulse_mode == P_7)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	if (pulse_mode == SP_7)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);
	if (pulse_mode == P_11)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), false);
	if (pulse_mode == SP_11)
		return get_P_with_saw(sin_time, sin_angle_freq, initial_phase, amplitude, get_Pulse_Num(pulse_mode), true);

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

	int pwm_value = get_pwm_value(sin_value, saw_value) * 2;

	Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
	return wv;
}

Wave_Values calculate_E231(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 65);
	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (cv.wave_stat > 67)
		pulse_mode = P_1;
	else if (cv.wave_stat > 60)
	{
		pulse_mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 8.0 * (cv.wave_stat - 60);
	}
	else if (49 <= cv.wave_stat && cv.wave_stat <= 60)
	{
		double expect_saw_freq = 710 + (1750 - 710) / 11 * (cv.wave_stat - 49);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}
	else if (23 <= cv.wave_stat && cv.wave_stat < 50)
	{
		double expect_saw_freq = 1045 + (710 - 1045) / 26 * (cv.wave_stat - 23);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 1045 * M_2PI;
		pulse_mode = Not_In_Sync;
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_207(Control_Values cv)
{
	double amplitude = 0;

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;

	if (!cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 60);
		if (60 <= cv.wave_stat)
			pulse_mode = P_1;
		else if (53 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 53 * M_2PI))
		{
			if (cv.free_run)
				amplitude = get_Amplitude(cv.wave_stat, 50);
			if (amplitude > 0.97)
				amplitude = 0.97;
			pulse_mode = P_3;
		}
		else if (44 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 44 * M_2PI))
			pulse_mode = P_5;
		else if (31 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 31 * M_2PI))
			pulse_mode = P_9;
		else if (14 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 14 * M_2PI))
			pulse_mode = P_15;
		else
		{
			expect_saw_angle_freq = M_2PI * 365;
			pulse_mode = Not_In_Sync;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 80);
		if (80 <= cv.wave_stat)
			pulse_mode = P_1;
		else if (65 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 65 * M_2PI))
		{
			if (cv.free_run)
				amplitude = get_Amplitude(cv.wave_stat, 50);
			if (amplitude > 0.97)
				amplitude = 0.97;
			pulse_mode = P_3;
		}
		else if (50 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 50 * M_2PI))
			pulse_mode = P_5;
		else if (30 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 30 * M_2PI))
			pulse_mode = P_9;
		else if (14 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 14 * M_2PI))
			pulse_mode = P_15;
		else if (8 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 8 * M_2PI))
			pulse_mode = P_27;
		else
		{
			Wave_Values wv;
			wv.sin_value = 0;
			wv.saw_value = 0;
			wv.pwm_value = 0;
			return wv;
		}
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_doremi(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 80);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (80 <= cv.wave_stat)
		pulse_mode = P_1;
	else if (57 <= cv.wave_stat)
		pulse_mode = P_Wide_3;
	else if (50 <= cv.wave_stat)
		pulse_mode = P_3;
	else if (43 <= cv.wave_stat)
		pulse_mode = P_5;
	else if (35 <= cv.wave_stat)
		pulse_mode = P_7;
	else if (30 <= cv.wave_stat)
		pulse_mode = P_12;
	else if (27 <= cv.wave_stat)
		pulse_mode = P_15;
	else if (24.5 <= cv.wave_stat)
		pulse_mode = P_18;
	else
	{
		if (!cv.brake)
		{
			double expect_saw_freq = 400;
			pulse_mode = Not_In_Sync;
			if (5.6 <= cv.wave_stat)
				expect_saw_freq = 400;
			else if (5 <= cv.wave_stat)
				expect_saw_freq = 350;
			else if (4.3 <= cv.wave_stat)
				expect_saw_freq = 311;
			else if (3.4 <= cv.wave_stat)
				expect_saw_freq = 294;
			else if (2.7 <= cv.wave_stat)
				expect_saw_freq = 262;
			else if (2.0 <= cv.wave_stat)
				expect_saw_freq = 233;
			else if (1.5 <= cv.wave_stat)
				expect_saw_freq = 223;
			else if (0.5 <= cv.wave_stat)
				expect_saw_freq = 196;
			else
				expect_saw_freq = 175;

			expect_saw_angle_freq = M_2PI * expect_saw_freq;
		}
		else
		{
			if (cv.wave_stat > 4)
			{
				expect_saw_angle_freq = 2 * M_PI * 400;
				pulse_mode = Not_In_Sync;
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

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_E235(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 54);

	double sin_value = get_sin_value(sin_time, sin_angle_freq, cv.initial_phase, amplitude);
	double saw_value;
	if (cv.wave_stat > 54)
	{

		saw_angle_freq = sin_angle_freq * 15;
		saw_time = sin_time;

		saw_value = get_saw_value(saw_time, saw_angle_freq, 0);
	}
	else
	{

		if (random_freq_move_count == 0)
		{
			//saw_freq = 740;
			int random_v = my_random();
			int diff_freq = mod_i(random_v, 30);
			if (random_v & 0x01)
				diff_freq = -diff_freq;

			double base_freq = (double)550 + 3.148148148148148 * (cv.wave_stat); //170.0/54.0*(cv.wave_stat);

			double silent_random_freq = base_freq + diff_freq;

			double expect_saw_angle_freq = 2 * M_PI * silent_random_freq;
			saw_time = saw_angle_freq / expect_saw_angle_freq * saw_time;
			saw_angle_freq = expect_saw_angle_freq;
		}
		saw_value = get_saw_value(saw_time, saw_angle_freq, 0);

		random_freq_move_count++;
		if (random_freq_move_count == 100)
			random_freq_move_count = 0;
	}

	int pwm_value = get_pwm_value(sin_value, saw_value) * 2;

	Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
	return wv;
}

Wave_Values calculate_E209(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 53);

	double expect_saw_angle_freq;
	Pulse_Mode pulse_mode;
	if (53 <= cv.wave_stat)
		pulse_mode = P_1;
	else if (46 <= cv.wave_stat)
		pulse_mode = P_3;
	else if (30 <= cv.wave_stat)
		pulse_mode = P_9;
	else if (19 <= cv.wave_stat)
		pulse_mode = P_21;
	else if (9 <= cv.wave_stat)
		pulse_mode = P_33;
	else if (2 <= cv.wave_stat && !cv.brake)
		pulse_mode = P_57;
	else if (cv.wave_stat < 2 && !cv.brake)
	{
		pulse_mode = Not_In_Sync;
		expect_saw_angle_freq = M_2PI * 114;
	}
	else if (8 < cv.wave_stat && cv.wave_stat < 18 && cv.brake)
		pulse_mode = P_33;
	else
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_9820_mitsubishi(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 55);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (55 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (50 <= cv.wave_stat)
		pulse_Mode = P_3;
	else if (13 <= cv.wave_stat)
	{
		expect_saw_angle_freq = 700 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (cv.brake && cv.wave_stat < 8.5)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	else if (cv.wave_stat > 2)
	{
		double expect_saw_freq = 250 + (700 - 250) / 11 * (cv.wave_stat - 2);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 250 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_9820_hitachi(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 65);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (67 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (60 <= cv.wave_stat)
	{
		pulse_Mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 8.0 * (cv.wave_stat - 60);
	}
	else if (49 <= cv.wave_stat)
	{
		double expect_saw_freq = 780 + (1820 - 780) / 11 * (cv.wave_stat - 49);
		expect_saw_angle_freq = expect_saw_freq * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 780 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_E233(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 50);
	if (cv.free_run && cv.mascon_on == false && amplitude < 0.85)
	{
		amplitude = 0.0;
	}

	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 73.5);
		if (73.5 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (62.5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 62.5 * M_2PI))
			pulse_Mode = P_3;
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(750, 100) * M_2PI;
		}
	}
	else
	{
		if (50 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (45 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 45 * M_2PI))
			pulse_Mode = P_3;
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(750, 100) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_silent(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 60);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (50 <= cv.wave_stat)
		pulse_Mode = P_27;
	else
	{
		pulse_Mode = Not_In_Sync;
		expect_saw_angle_freq = get_random_freq(550, 100) * M_2PI;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_mitsubishi_gto(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 63);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (63 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (60 <= cv.wave_stat)
		pulse_Mode = P_Wide_3;
	else if (57 <= cv.wave_stat)
		pulse_Mode = P_3;
	else if (44 <= cv.wave_stat)
		pulse_Mode = P_5;
	else if (36 <= cv.wave_stat)
		pulse_Mode = P_7;
	else if (16 <= cv.wave_stat)
	{
		expect_saw_angle_freq = 400 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (cv.brake && cv.wave_stat < 7.4)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	else if (cv.wave_stat >= 2)
	{
		double expect_saw_freq = 216 + (400 - 216) / 14 * (cv.wave_stat - 2);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 216 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_toyo_IGBT(Control_Values cv)
{
	if (cv.brake)
	{
		double amplitude = get_Amplitude(cv.wave_stat, 98);

		if (96 <= cv.wave_stat && cv.wave_stat <= 98)
		{
			amplitude = 5 + (get_Amplitude(96, 98) - 5) / 2.0 * (98 - cv.wave_stat);
		}

		double expect_saw_angle_freq = sin_angle_freq * 10;
		Pulse_Mode pulse_Mode = P_1;
		if (98 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (33 <= cv.wave_stat)
			pulse_Mode = P_9;
		else
		{
			expect_saw_angle_freq = 1045 * M_2PI;
			pulse_Mode = Not_In_Sync;
		}

		return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
	}
	else
	{
		double amplitude = get_Amplitude(cv.wave_stat, 55);

		if (53 <= cv.wave_stat && cv.wave_stat <= 55)
		{
			amplitude = 5 + (get_Amplitude(53, 55) - 5) / 2.0 * (55 - cv.wave_stat);
		}

		double expect_saw_angle_freq = sin_angle_freq * 10;
		Pulse_Mode pulse_Mode = P_1;
		if (55 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (34 <= cv.wave_stat)
			pulse_Mode = P_9;
		else
		{
			expect_saw_angle_freq = 1045 * M_2PI;
			pulse_Mode = Not_In_Sync;
		}

		return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
	}
}

Wave_Values calculate_Famima(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 60);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (60 <= cv.wave_stat)
		pulse_mode = P_1;
	else if (56 <= cv.wave_stat)
		pulse_mode = P_3;
	else
	{
		double expect_saw_freq = 0;
		if (48 <= cv.wave_stat)
			expect_saw_freq = 622;
		else if (44 <= cv.wave_stat)
			expect_saw_freq = 466;
		else if (40 <= cv.wave_stat)
			expect_saw_freq = 698;
		else if (36 <= cv.wave_stat)
			expect_saw_freq = 783;
		else if (32 <= cv.wave_stat)
			expect_saw_freq = 698;
		else if (28 <= cv.wave_stat)
			expect_saw_freq = 466;
		else if (20 <= cv.wave_stat)
			expect_saw_freq = 932;
		else if (16 <= cv.wave_stat)
			expect_saw_freq = 587;
		else if (12 <= cv.wave_stat)
			expect_saw_freq = 622;
		else if (8 <= cv.wave_stat)
			expect_saw_freq = 466;
		else if (4 <= cv.wave_stat)
			expect_saw_freq = 622;
		else
			expect_saw_freq = 783;

		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_real_doremi(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 80);

	double expect_saw_angle_freq = 0;
	Wave_Values wv;
	Pulse_Mode pulse_mode;
	if (80 <= cv.wave_stat)
		pulse_mode = P_1;
	else if (57 <= cv.wave_stat)
		pulse_mode = P_Wide_3;
	else if (50 <= cv.wave_stat)
		pulse_mode = P_3;
	else if (43 <= cv.wave_stat)
		pulse_mode = P_5;
	else if (35 <= cv.wave_stat)
		pulse_mode = P_7;
	else if (30 <= cv.wave_stat)
		pulse_mode = P_12;
	else if (27 <= cv.wave_stat)
		pulse_mode = P_15;
	else if (24.5 <= cv.wave_stat)
		pulse_mode = P_18;
	else
	{
		if (!cv.brake)
		{
			double expect_saw_freq = 0;
			if (5.6 <= cv.wave_stat)
				expect_saw_freq = 587;
			else if (5 <= cv.wave_stat)
				expect_saw_freq = 523;
			else if (4.3 <= cv.wave_stat)
				expect_saw_freq = 493;
			else if (3.4 <= cv.wave_stat)
				expect_saw_freq = 440;
			else if (2.7 <= cv.wave_stat)
				expect_saw_freq = 391;
			else if (2.0 <= cv.wave_stat)
				expect_saw_freq = 349;
			else if (1.5 <= cv.wave_stat)
				expect_saw_freq = 329;
			else if (0.5 <= cv.wave_stat)
				expect_saw_freq = 293;
			else
				expect_saw_freq = 261;
			expect_saw_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Not_In_Sync;
		}
		else
		{
			if (cv.wave_stat > 4)
			{
				expect_saw_angle_freq = M_2PI * 400;
				pulse_mode = Not_In_Sync;
			}
			else
			{
				wv.sin_value = 0;
				wv.saw_value = 0;
				wv.pwm_value = 0;
				return wv;
			}
		}
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_toubu_50050(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 61);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (61 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (58 <= cv.wave_stat)
	{
		pulse_Mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 4.0 * (cv.wave_stat - 58);
	}
	else if (49 <= cv.wave_stat)
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)680 + 1140 / 9.0 * (cv.wave_stat - 49); //170.0/54.0*(cv.wave_stat);
		expect_saw_angle_freq = M_2PI * base_freq;
	}
	else if (46 <= cv.wave_stat)
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)730 - 50.0 / 49.0 * (cv.wave_stat); //170.0/54.0*(cv.wave_stat);
		expect_saw_angle_freq = M_2PI * base_freq;
	}
	else if (cv.brake && cv.wave_stat <= 4)
	{
		pulse_Mode = Not_In_Sync;
		expect_saw_angle_freq = M_2PI * 200;
	}
	else
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)730 - 50.0 / 49.0 * (cv.wave_stat); //170.0/54.0*(cv.wave_stat);
		expect_saw_angle_freq = get_random_freq((int)base_freq, 100) * M_2PI;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_207_1000_update(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 60);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;

	if (!cv.brake)
	{
		if (60 <= cv.wave_stat)
			pulse_mode = P_1;
		else if (59 <= cv.wave_stat)
		{
			amplitude = 0.9 + 0.1 / 2.0 * (cv.wave_stat - 59);
			pulse_mode = P_Wide_3;
		}
		else if (55 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 55 * M_2PI))
			pulse_mode = P_3;
		else if (47 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 47 * M_2PI))
			pulse_mode = P_5;
		else if (36 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 36 * M_2PI))
			pulse_mode = P_9;
		else if (23 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 23 * M_2PI))
			pulse_mode = P_15;
		else
		{
			pulse_mode = Not_In_Sync;
			double base_freq = 550 + 3.272727272727273 * cv.wave_stat;
			expect_saw_angle_freq = get_random_freq((int)base_freq, 100) * M_2PI;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 80);
		if (80 <= cv.wave_stat)
			pulse_mode = P_1;
		else if (72 <= cv.wave_stat)
		{
			amplitude = 0.8 + 0.2 / 8.0 * (cv.wave_stat - 72);
			pulse_mode = P_Wide_3;
		}
		else if (57 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 57 * M_2PI))
			pulse_mode = P_3;
		else if (44 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 44 * M_2PI))
			pulse_mode = P_5;
		else if (29 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 29 * M_2PI))
			pulse_mode = P_9;
		else if (14 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 14 * M_2PI))
			pulse_mode = P_15;
		else
		{
			pulse_mode = Not_In_Sync;
			double base_freq = 550 + 3.272727272727273 * cv.wave_stat;
			expect_saw_angle_freq = get_random_freq((int)base_freq, 100) * M_2PI;
		}
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_225_5100_mitsubishi(Control_Values cv)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (!cv.brake)
	{
		if (56 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (48 <= cv.wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 1.1 + (2 - 1.1) / 8.0 * (cv.wave_stat - 48);
		}
		else
		{
			amplitude = get_Amplitude(cv.wave_stat, 48);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(1050, 100) * M_2PI;
		}
	}
	else
	{
		if (77 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (59 <= cv.wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 0.8 + (2 - 0.8) / 18.0 * (cv.wave_stat - 59);
		}
		else
		{
			amplitude = get_Amplitude(cv.wave_stat, 74);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(1050, 100) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_321_hitachi(Control_Values cv)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.brake)
	{
		if (72 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (56 <= cv.wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 2 + (get_Amplitude(56, 72) - 2) / 16.0 * (72 - cv.wave_stat);
		}
		else
		{
			amplitude = get_Amplitude(cv.wave_stat, 72);
			pulse_Mode = Not_In_Sync;
			double base_freq = 1050;
			if (4 >= cv.wave_stat)
				base_freq = 510 + ((cv.wave_stat > 1) ? ((623 - 510) / 3.0 * (cv.wave_stat - 1)) : 0);
			expect_saw_angle_freq = get_random_freq((int)base_freq, 100) * M_2PI;
		}
	}
	else
	{
		if (55 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (40 <= cv.wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 2 + (get_Amplitude(40, 55) - 2) / 15.0 * (55 - cv.wave_stat);
		}
		else
		{
			amplitude = get_Amplitude(cv.wave_stat, 55);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq((int)1050, 100) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_toyo_GTO(Control_Values cv)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 77);

		if (77 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (74 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 4.0 * (cv.wave_stat - 74);
		}
		else if (69 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8;
		}

		else if (60 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 60 * M_2PI))
			pulse_Mode = P_3;
		else if (43 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 43 * M_2PI))
			pulse_Mode = P_5;
		else if (25 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 25 * M_2PI))
			pulse_Mode = P_9;
		else if (6 <= cv.wave_stat && cv.wave_stat <= 9)
		{
			expect_saw_angle_freq = M_2PI * (260 + (365 - 260) / 25.0 * (cv.wave_stat - 3));
			pulse_Mode = Not_In_Sync;
		}
		else
		{
			if (cv.wave_stat < 5)
			{
				Wave_Values wv;
				wv.sin_value = 0;
				wv.saw_value = 0;
				wv.pwm_value = 0;
				return wv;
			}
			pulse_Mode = Not_In_Sync;
			int base_freq = 260;
			if (cv.wave_stat > 3)
				base_freq = (int)(260 + (365 - 260) / 25.0 * (cv.wave_stat - 3));
			expect_saw_angle_freq = get_random_freq(base_freq, 100) * M_2PI;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 60);

		if (60 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (56 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 5.0 * (cv.wave_stat - 56);
		}

		else if (51 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 51 * M_2PI))
			pulse_Mode = P_3;
		else if (43 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 43 * M_2PI))
			pulse_Mode = P_5;
		else if (27 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 27 * M_2PI))
			pulse_Mode = P_9;
		else if (6 <= cv.wave_stat && cv.wave_stat <= 9)
		{
			expect_saw_angle_freq = M_2PI * (260 + (365 - 260) / 23.0 * (cv.wave_stat - 3));
			pulse_Mode = Not_In_Sync;
		}
		else
		{
			pulse_Mode = Not_In_Sync;

			int base_freq = 260;
			if (cv.wave_stat > 3)
				base_freq = (int)(260 + (365 - 260) / 23.0 * (cv.wave_stat - 3));
			expect_saw_angle_freq = get_random_freq(base_freq, 100) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_tokyu_9000_hitachi_gto(Control_Values cv)
{
	double amplitude = 0;
	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode = Not_In_Sync;
	if (cv.brake) //settings for braking vvvf pattern
	{
		amplitude = get_Amplitude(cv.wave_stat, 60);
		expect_saw_angle_freq = cv.wave_stat;
		if (60 <= cv.wave_stat)
		{
			pulse_mode = P_1;
		}
		else if (54 <= cv.wave_stat)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 6.0 * (cv.wave_stat - 54);
		}
		else if (50 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 50 * M_2PI))
			pulse_mode = P_3;
		else if (41 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 41 * M_2PI))
			pulse_mode = P_5;
		else if (27 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 27 * M_2PI))
			pulse_mode = P_9;
		else if (14.5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 14.5 * M_2PI))
			pulse_mode = P_15;
		else if (8 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 8 * M_2PI))
			pulse_mode = P_27;
		else if (7 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 7 * M_2PI))
			pulse_mode = P_45;
		else if (5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 5 * M_2PI))
		{
			expect_saw_angle_freq = M_2PI * 200;
			pulse_mode = Not_In_Sync;
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
	else //settings for accelerating vvvf pattern
	{
		amplitude = get_Amplitude(cv.wave_stat, 45);
		expect_saw_angle_freq = cv.wave_stat;
		if (45 <= cv.wave_stat)
		{
			pulse_mode = P_1;
		}
		else if (43 <= cv.wave_stat)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 2.0 * (cv.wave_stat - 43);
		}
		else if (37 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 37 * M_2PI))
			pulse_mode = P_3;
		else if (32 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 32 * M_2PI))
			pulse_mode = P_5;
		else if (25 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 25 * M_2PI))
			pulse_mode = P_9;
		else if (14 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 14 * M_2PI))
			pulse_mode = P_15;
		else if (7 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 7 * M_2PI))
			pulse_mode = P_27;
		else if (5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 5 * M_2PI))
			pulse_mode = P_45;
		else
		{
			expect_saw_angle_freq = M_2PI * 200;
			pulse_mode = Not_In_Sync;
		}
	}
	return calculate_common(pulse_mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_toei_6300_3(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 60);
	if (cv.mascon_on == false && amplitude < 0.65)
		amplitude = 0.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (60 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (52.5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 52.5 * M_2PI))
		pulse_Mode = P_3;
	else if (15 <= cv.wave_stat)
	{
		expect_saw_angle_freq = get_random_freq(1000, 100) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (5 <= cv.wave_stat)
	{
		double expect_saw_freq = 220 + (1000 - 220) / 10 * (cv.wave_stat - 5);
		expect_saw_angle_freq = get_random_freq((int)expect_saw_freq, 100) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (cv.wave_stat <= 3 && cv.brake)
	{
		double expect_saw_freq = 205 + (220 - 205) / 3.0 * (cv.wave_stat);
		expect_saw_angle_freq = get_random_freq((int)expect_saw_freq, 100) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = get_random_freq(220, 100) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_keihan_13000_toyo_IGBT(Control_Values cv)
{
	double amplitude = get_Amplitude(cv.wave_stat, 55);

	if (53 <= cv.wave_stat && cv.wave_stat <= 55)
	{
		amplitude = 5 + (get_Amplitude(53, 55) - 5) / 2.0 * (55 - cv.wave_stat);
	}

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (55 <= cv.wave_stat)
		pulse_Mode = P_1;
	else if (34 <= cv.wave_stat)
		pulse_Mode = P_9;
	else
	{
		expect_saw_angle_freq = 525 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_tokyuu_5000(Control_Values cv)
{

	double amplitude = get_Amplitude(cv.wave_stat, 58);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 65);
		if (65 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (61 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 5.0 * (cv.wave_stat - 61);
		}
		else if (50 <= cv.wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)700 + 1100 / 11.0 * (cv.wave_stat - 50); //170.0/54.0*(cv.wave_stat);
			expect_saw_angle_freq = M_2PI * base_freq;
		}
		else if (23 <= cv.wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)740 - 40.0 / (50 - 23) * (cv.wave_stat - 23); //170.0/54.0*(cv.wave_stat);
			expect_saw_angle_freq = base_freq * M_2PI;
		}
		else if (cv.brake && cv.wave_stat <= 4)
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 200;
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 740;
		}
	}
	else
	{
		if (58 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (55 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 4.0 * (cv.wave_stat - 55);
		}
		else if (42 <= cv.wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)700 + 1100 / 13.0 * (cv.wave_stat - 42); //170.0/54.0*(cv.wave_stat);
			expect_saw_angle_freq = M_2PI * base_freq;
		}
		else if (23 <= cv.wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)740 - 40.0 / (46 - 23) * (cv.wave_stat - 23); //170.0/54.0*(cv.wave_stat);
			expect_saw_angle_freq = base_freq * M_2PI;
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 740;
		}
	}

	if (!cv.mascon_on && cv.free_run && cv.wave_stat < 23)
		amplitude = 0;

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_keio_8000_gto(Control_Values cv)
{

	double amplitude;
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 68.2);
		if (68.2 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (63.5 <= cv.wave_stat && !cv.free_run)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 6.0 * (cv.wave_stat - 63.5);
		}
		else if ((cv.free_run && sin_angle_freq > 63.5 * M_2PI) && cv.wave_stat >= 30)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 34.0 * (cv.wave_stat - 30);
		}
		else if (54.5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 54.5 * M_2PI))
			pulse_Mode = P_3;
		else if (41.2 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 41.2 * M_2PI))
			pulse_Mode = P_7;
		else if (32.3 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 32.3 * M_2PI))
			pulse_Mode = P_11;
		else if (21.0 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 21.0 * M_2PI))
			pulse_Mode = P_15;
		else if (7.8 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 7.8 * M_2PI))
			pulse_Mode = P_21;
		else
		{
			Wave_Values wv;
			wv.sin_value = 0;
			wv.saw_value = 0;
			wv.pwm_value = 0;
			return wv;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 50);
		if (50 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (48.7 <= cv.wave_stat && !cv.free_run)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 2.0 * (cv.wave_stat - 48.7);
		}
		else if ((cv.free_run && sin_angle_freq > 48.7 * M_2PI) && cv.wave_stat >= 30)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 20.0 * (cv.wave_stat - 30);
		}
		else if (41.2 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 41.2 * M_2PI))
			pulse_Mode = P_3;
		else if (32.4 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 38.4 * M_2PI))
			pulse_Mode = P_7;
		else if (29.5 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 29.5 * M_2PI))
			pulse_Mode = P_11;
		else if (25.8 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 25.8 * M_2PI))
			pulse_Mode = P_15;
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 400;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_tokyuu_1000_1500_IGBT(Control_Values cv)
{

	double amplitude;
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.free_run && !cv.mascon_on)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 51);
		if (51 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (50.8 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 0.2 * (cv.wave_stat - 50.8);
		}
		else if (38.3 <= cv.wave_stat && !(cv.free_run && sin_angle_freq > 50 * M_2PI))
		{
			pulse_Mode = SP_15;
			amplitude = 2 + (get_Amplitude(38.3, 50) - 2) / (50 - 38.3) * (50 - cv.wave_stat);
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * get_pattern_random((int)(400 + 180 / 38.3 * cv.wave_stat), 600, 7000);
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 47.3);
		if (47.3 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (44.4 <= cv.wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 4.0 * (cv.wave_stat - 44.4);
		}
		else if (35.8 <= cv.wave_stat && !(cv.free_run && sin_angle_freq > 44.4 * M_2PI))
		{
			pulse_Mode = SP_15;
			amplitude = 1.3 + (get_Amplitude(35.8, 50) - 1.3) / (44.4 - 35.8) * (44.4 - cv.wave_stat);
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * get_pattern_random((int)(400 + 180 / 34.0 * cv.wave_stat), 600, 7000);
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_E233_3000(Control_Values cv)
{

	double amplitude;
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (!cv.mascon_on && cv.free_run && cv.wave_stat < 18)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 69.5);
		if (69.5 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (64.8 <= cv.wave_stat && cv.mascon_on)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 6.0 * (cv.wave_stat - 64.8);
		}
		else if (51.3 <= cv.wave_stat && cv.mascon_on)
		{
			amplitude = 1.3 + (get_Amplitude(51.3, 69.5) - 1.3) / (64.8 - 51.3) * (64.8 - cv.wave_stat);
			pulse_Mode = SP_9;
		}
		else if (cv.wave_stat <= 4)
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 200;
		}
		else
		{
			double base_freq = 200 + (900 - 200) / 47.3 * (cv.wave_stat - 4);
			if (base_freq > 900)
				base_freq = 900;
			pulse_Mode = Not_In_Sync;
			if (cv.wave_stat <= 9)
			{
				double random_range = 99 / 5.0 * (cv.wave_stat - 4) + 1;
				expect_saw_angle_freq = M_2PI * get_random_freq((int)base_freq, (int)random_range);
			}
			else
			{
				expect_saw_angle_freq = M_2PI * get_random_freq((int)base_freq, 100);
			}
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 51);
		if (51 <= cv.wave_stat)
			pulse_Mode = P_1;
		else if (46.8 <= cv.wave_stat && cv.mascon_on)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 6.0 * (cv.wave_stat - 46.8);
		}
		else if (42 <= cv.wave_stat && cv.mascon_on)
		{
			amplitude = 1.3 + (get_Amplitude(42, 51) - 1.3) / (46.8 - 42) * (46.8 - cv.wave_stat);
			pulse_Mode = SP_9;
		}
		else if (19.7 <= cv.wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = 525 + (910 - 510) / (41.6 - 19.7) * (cv.wave_stat - 19.7);
			if (base_freq > 910)
				base_freq = 910;
			expect_saw_angle_freq = M_2PI * get_random_freq((int)base_freq, 100);
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * get_random_freq(525, 100);
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude);
}

Wave_Values calculate_jre_209_mitsubishi_gto(Control_Values cv)
{
	double amplitude = 0;
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (cv.brake)
	{
		amplitude = get_Amplitude(cv.wave_stat, 68);
		if (59 <= cv.wave_stat)
		{
			pulse_Mode = P_1;
			amplitude = get_Amplitude(cv.wave_stat, 72) + get_Amplitude(cv.wave_stat - 59, 15);
		}
		else if (49 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 49 * M_2PI))
			pulse_Mode = P_3;
		else if (40 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 40 * M_2PI))
			pulse_Mode = P_9;
		else if (19 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 19 * M_2PI))
			pulse_Mode = P_21;
		else if (7 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 7 * M_2PI))
			pulse_Mode = P_33;
		else
		{
			Wave_Values wv;
			wv.sin_value = 0;
			wv.saw_value = 0;
			wv.pwm_value = 0;
			return wv;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv.wave_stat, 62);
		if (53 <= cv.wave_stat)
		{
			pulse_Mode = P_1;
			amplitude = get_Amplitude(cv.wave_stat, 66) + get_Amplitude(cv.wave_stat - 53, 15);
		}
		else if (46 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 46 * M_2PI))
			pulse_Mode = P_3;
		else if (30 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 30 * M_2PI))
			pulse_Mode = P_9;
		else if (19 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 19 * M_2PI))
			pulse_Mode = P_21;
		else if (9 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 9 * M_2PI))
			pulse_Mode = P_33;
		else if (2 <= cv.wave_stat || (cv.free_run && sin_angle_freq > 2 * M_2PI))
			pulse_Mode = P_57;
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 114;
		}
	}
	return calculate_three_level(pulse_Mode, expect_saw_angle_freq, cv.initial_phase, amplitude, false);
}