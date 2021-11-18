#include "vvvf_wave.h"
#include "vvvf_main.h"
#include "my_math.h"

#define M_2PI 6.28318530717958
#define M_PI 3.14159265358979
#define M_PI_2 1.57079632679489661923
#define M_2_PI 0.636619772367581343076

//function caliculation
double get_saw_value_simple(double x)
{
	double fixed_x = x - (double)((int)(x / M_2PI) * M_2PI);
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

double get_pwm_value(double sin_value, double saw_value)
{
	if (disconnect)
		return 0;
	if (sin_value - saw_value > 0)
		return 1;
	else
		return -1;
}

Wave_Values get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage, bool saw_oppose)
{
	double sin = get_sin_value(time, angle_frequency, initial_phase, 1);
	double saw = get_saw_value(time, angle_frequency, initial_phase);
	if (saw_oppose)
		saw = -saw;
	double pwm = ((sin - saw > 0) ? 1 : -1) * voltage;
	double nega_saw = (saw > 0) ? saw - 1 : saw + 1;
	double gate = get_pwm_value(pwm, nega_saw);
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
	double gate = get_pwm_value(pwm, carrier_saw);
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
		if (mod_i(random_v, 500) < 250)
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
	if (random_freq_move_count == 100)
		random_freq_move_count = 0;
	return random_freq;
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

	double pwm_value = get_pwm_value(sin_value, saw_value);

	Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
	return wv;
}

Wave_Values calculate_E231(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 65);
	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (wave_stat > 67)
		pulse_mode = P_1;
	else if (wave_stat > 60)
	{
		pulse_mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 8.0 * (wave_stat - 60);
	}
	else if (49 <= wave_stat && wave_stat <= 60)
	{
		double expect_saw_freq = 710 + (1750 - 710) / 11 * (wave_stat - 49);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}
	else if (23 <= wave_stat && wave_stat < 50)
	{
		double expect_saw_freq = 1045 + (710 - 1045) / 26 * (wave_stat - 23);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 1045 * M_2PI;
		pulse_mode = Not_In_Sync;
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_207(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 60);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (60 <= wave_stat)
		pulse_mode = P_1;
	else if (53 <= wave_stat)
		pulse_mode = P_3;
	else if (44 <= wave_stat)
		pulse_mode = P_5;
	else if (31 <= wave_stat)
		pulse_mode = P_9;
	else if (14 <= wave_stat)
		pulse_mode = P_15;
	else if (wave_stat < 14 && !brake)
	{
		expect_saw_angle_freq = M_2PI * 365;
		pulse_mode = Not_In_Sync;
	}
	else if (8 < wave_stat && wave_stat < 14 && brake)
	{

		expect_saw_angle_freq = sin_angle_freq * 27;
		pulse_mode = P_27;
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
	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_doremi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 80);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (80 <= wave_stat)
		pulse_mode = P_1;
	else if (57 <= wave_stat)
		pulse_mode = P_Wide_3;
	else if (50 <= wave_stat)
		pulse_mode = P_3;
	else if (43 <= wave_stat)
		pulse_mode = P_5;
	else if (35 <= wave_stat)
		pulse_mode = P_7;
	else if (30 <= wave_stat)
		pulse_mode = P_12;
	else if (27 <= wave_stat)
		pulse_mode = P_15;
	else if (24.5 <= wave_stat)
		pulse_mode = P_18;
	else
	{
		if (!brake)
		{
			double expect_saw_freq = 400;
			pulse_mode = Not_In_Sync;
			if (5.6 <= wave_stat)
				expect_saw_freq = 400;
			else if (5 <= wave_stat)
				expect_saw_freq = 350;
			else if (4.3 <= wave_stat)
				expect_saw_freq = 311;
			else if (3.4 <= wave_stat)
				expect_saw_freq = 294;
			else if (2.7 <= wave_stat)
				expect_saw_freq = 262;
			else if (2.0 <= wave_stat)
				expect_saw_freq = 233;
			else if (1.5 <= wave_stat)
				expect_saw_freq = 223;
			else if (0.5 <= wave_stat)
				expect_saw_freq = 196;
			else
				expect_saw_freq = 175;

			expect_saw_angle_freq = M_2PI * expect_saw_freq;
		}
		else
		{
			if (wave_stat > 4)
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

	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_E235(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 54);

	double sin_value = get_sin_value(sin_time, sin_angle_freq, initial_phase, amplitude);
	double saw_value, pwm_value;
	if (wave_stat > 54)
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
			if (mod_i(random_v, 500) < 250)
				diff_freq = -diff_freq;

			double base_freq = (double)550 + 3.148148148148148 * (wave_stat); //170.0/54.0*(wave_stat);

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

	pwm_value = get_pwm_value(sin_value, saw_value);

	Wave_Values wv;
	wv.sin_value = sin_value;
	wv.saw_value = saw_value;
	wv.pwm_value = pwm_value;
	return wv;
}

Wave_Values calculate_E209(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 53);

	double expect_saw_angle_freq;
	Pulse_Mode pulse_mode;
	if (53 <= wave_stat)
		pulse_mode = P_1;
	else if (46 <= wave_stat)
		pulse_mode = P_3;
	else if (30 <= wave_stat)
		pulse_mode = P_9;
	else if (19 <= wave_stat)
		pulse_mode = P_21;
	else if (9 <= wave_stat)
		pulse_mode = P_33;
	else if (2 <= wave_stat && !brake)
		pulse_mode = P_57;
	else if (wave_stat < 2 && !brake)
	{
		pulse_mode = Not_In_Sync;
		expect_saw_angle_freq = M_2PI * 114;
	}
	else if (8 < wave_stat && wave_stat < 18 && brake)
		pulse_mode = P_33;
	else
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_9820_mitsubishi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 55);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (55 <= wave_stat)
		pulse_Mode = P_1;
	else if (50 <= wave_stat)
		pulse_Mode = P_3;
	else if (13 <= wave_stat)
	{
		expect_saw_angle_freq = 700 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (brake && wave_stat < 8.5)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	else if (wave_stat > 2)
	{
		double expect_saw_freq = 250 + (700 - 250) / 11 * (wave_stat - 2);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 250 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_9820_hitachi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 65);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (67 <= wave_stat)
		pulse_Mode = P_1;
	else if (60 <= wave_stat)
	{
		pulse_Mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 8.0 * (wave_stat - 60);
	}
	else if (49 <= wave_stat)
	{
		double expect_saw_freq = 780 + (1820 - 780) / 11 * (wave_stat - 49);
		expect_saw_angle_freq = expect_saw_freq * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 780 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_E233(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 50);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (50 <= wave_stat)
		pulse_Mode = P_1;
	else if (45 <= wave_stat)
		pulse_Mode = P_3;
	else
	{
		pulse_Mode = Not_In_Sync;
		expect_saw_angle_freq = get_random_freq(750, 30) * M_2PI;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_silent(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 60);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (50 <= wave_stat)
		pulse_Mode = P_27;
	else
	{
		pulse_Mode = Not_In_Sync;
		expect_saw_angle_freq = get_random_freq(550, 30) * M_2PI;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_mitsubishi_gto(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 63);

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (63 <= wave_stat)
		pulse_Mode = P_1;
	else if (60 <= wave_stat)
		pulse_Mode = P_Wide_3;
	else if (57 <= wave_stat)
		pulse_Mode = P_3;
	else if (44 <= wave_stat)
		pulse_Mode = P_5;
	else if (36 <= wave_stat)
		pulse_Mode = P_7;
	else if (16 <= wave_stat)
	{
		expect_saw_angle_freq = 400 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (brake && wave_stat < 7.4)
	{
		Wave_Values wv;
		wv.sin_value = 0;
		wv.saw_value = 0;
		wv.pwm_value = 0;
		return wv;
	}
	else if (wave_stat >= 2)
	{
		double expect_saw_freq = 216 + (400 - 216) / 14 * (wave_stat - 2);
		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = 216 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_toyo_IGBT(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	if (brake)
	{
		double amplitude = get_Amplitude(wave_stat, 98);

		if (96 <= wave_stat && wave_stat <= 98)
		{
			amplitude = 5 + (get_Amplitude(96, 98) - 5) / 2.0 * (98 - wave_stat);
		}

		double expect_saw_angle_freq = sin_angle_freq * 10;
		Pulse_Mode pulse_Mode = P_1;
		if (98 <= wave_stat)
			pulse_Mode = P_1;
		else if (33 <= wave_stat)
			pulse_Mode = P_9;
		else
		{
			expect_saw_angle_freq = 1045 * M_2PI;
			pulse_Mode = Not_In_Sync;
		}

		return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
	}
	else
	{
		double amplitude = get_Amplitude(wave_stat, 55);

		if (53 <= wave_stat && wave_stat <= 55)
		{
			amplitude = 5 + (get_Amplitude(53, 55) - 5) / 2.0 * (55 - wave_stat);
		}

		double expect_saw_angle_freq = sin_angle_freq * 10;
		Pulse_Mode pulse_Mode = P_1;
		if (55 <= wave_stat)
			pulse_Mode = P_1;
		else if (34 <= wave_stat)
			pulse_Mode = P_9;
		else
		{
			expect_saw_angle_freq = 1045 * M_2PI;
			pulse_Mode = Not_In_Sync;
		}

		return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
	}
}

Wave_Values calculate_Famima(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 60);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;
	if (60 <= wave_stat)
		pulse_mode = P_1;
	else if (56 <= wave_stat)
		pulse_mode = P_3;
	else
	{
		double expect_saw_freq = 0;
		if (48 <= wave_stat)
			expect_saw_freq = 622;
		else if (44 <= wave_stat)
			expect_saw_freq = 466;
		else if (40 <= wave_stat)
			expect_saw_freq = 698;
		else if (36 <= wave_stat)
			expect_saw_freq = 783;
		else if (32 <= wave_stat)
			expect_saw_freq = 698;
		else if (28 <= wave_stat)
			expect_saw_freq = 466;
		else if (20 <= wave_stat)
			expect_saw_freq = 932;
		else if (16 <= wave_stat)
			expect_saw_freq = 587;
		else if (12 <= wave_stat)
			expect_saw_freq = 622;
		else if (8 <= wave_stat)
			expect_saw_freq = 466;
		else if (4 <= wave_stat)
			expect_saw_freq = 622;
		else
			expect_saw_freq = 783;

		expect_saw_angle_freq = M_2PI * expect_saw_freq;
		pulse_mode = Not_In_Sync;
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_real_doremi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 80);

	double expect_saw_angle_freq = 0;
	Wave_Values wv;
	Pulse_Mode pulse_mode;
	if (80 <= wave_stat)
		pulse_mode = P_1;
	else if (57 <= wave_stat)
		pulse_mode = P_Wide_3;
	else if (50 <= wave_stat)
		pulse_mode = P_3;
	else if (43 <= wave_stat)
		pulse_mode = P_5;
	else if (35 <= wave_stat)
		pulse_mode = P_7;
	else if (30 <= wave_stat)
		pulse_mode = P_12;
	else if (27 <= wave_stat)
		pulse_mode = P_15;
	else if (24.5 <= wave_stat)
		pulse_mode = P_18;
	else
	{
		if (!brake)
		{
			double expect_saw_freq = 0;
			if (5.6 <= wave_stat)
				expect_saw_freq = 587;
			else if (5 <= wave_stat)
				expect_saw_freq = 523;
			else if (4.3 <= wave_stat)
				expect_saw_freq = 493;
			else if (3.4 <= wave_stat)
				expect_saw_freq = 440;
			else if (2.7 <= wave_stat)
				expect_saw_freq = 391;
			else if (2.0 <= wave_stat)
				expect_saw_freq = 349;
			else if (1.5 <= wave_stat)
				expect_saw_freq = 329;
			else if (0.5 <= wave_stat)
				expect_saw_freq = 293;
			else
				expect_saw_freq = 261;
			expect_saw_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Not_In_Sync;
		}
		else
		{
			if (wave_stat > 4)
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

	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_toubu_50050(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 61);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;
	if (61 <= wave_stat)
		pulse_Mode = P_1;
	else if (58 <= wave_stat)
	{
		pulse_Mode = P_Wide_3;
		amplitude = 0.8 + 0.2 / 4.0 * (wave_stat - 58);
	}
	else if (49 <= wave_stat)
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)680 + 1140 / 9.0 * (wave_stat - 49); //170.0/54.0*(wave_stat);
		expect_saw_angle_freq = M_2PI * base_freq;
	}
	else if (46 <= wave_stat)
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)730 - 50.0 / 49.0 * (wave_stat); //170.0/54.0*(wave_stat);
		expect_saw_angle_freq = M_2PI * base_freq;
	}
	else if (brake && wave_stat <= 4)
	{
		pulse_Mode = Not_In_Sync;
		expect_saw_angle_freq = M_2PI * 200;
	}
	else
	{
		pulse_Mode = Not_In_Sync;
		double base_freq = (double)730 - 50.0 / 49.0 * (wave_stat); //170.0/54.0*(wave_stat);
		expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_207_1000_update(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 60);

	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode;

	if (!brake)
	{
		if (60 <= wave_stat)
			pulse_mode = P_1;
		else if (59 <= wave_stat)
		{
			amplitude = 0.9 + 0.1 / 2.0 * (wave_stat - 59);
			pulse_mode = P_Wide_3;
		}
		else if (55 <= wave_stat)
			pulse_mode = P_3;
		else if (47 <= wave_stat)
			pulse_mode = P_5;
		else if (36 <= wave_stat)
			pulse_mode = P_9;
		else if (23 <= wave_stat)
			pulse_mode = P_15;
		else
		{
			pulse_mode = Not_In_Sync;
			double base_freq = 550 + 3.272727272727273 * wave_stat;
			expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
		}
	}
	else
	{
		amplitude = get_Amplitude(wave_stat, 80);
		if (80 <= wave_stat)
			pulse_mode = P_1;
		else if (72 <= wave_stat)
		{
			amplitude = 0.8 + 0.2 / 8.0 * (wave_stat - 72);
			pulse_mode = P_Wide_3;
		}
		else if (57 <= wave_stat)
			pulse_mode = P_3;
		else if (44 <= wave_stat)
			pulse_mode = P_5;
		else if (29 <= wave_stat)
			pulse_mode = P_9;
		else if (14 <= wave_stat)
			pulse_mode = P_15;
		else
		{
			pulse_mode = Not_In_Sync;
			double base_freq = 550 + 3.272727272727273 * wave_stat;
			expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
		}
	}

	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_225_5100_mitsubishi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (!brake)
	{
		if (56 <= wave_stat)
			pulse_Mode = P_1;
		else if (48 <= wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 1.1 + (2 - 1.1) / 8.0 * (wave_stat - 48);
		}
		else
		{
			amplitude = get_Amplitude(wave_stat, 48);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(1050, 30) * M_2PI;
		}
	}
	else
	{
		if (77 <= wave_stat)
			pulse_Mode = P_1;
		else if (59 <= wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 0.8 + (2 - 0.8) / 18.0 * (wave_stat - 59);
		}
		else
		{
			amplitude = get_Amplitude(wave_stat, 74);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq(1050, 30) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_321_hitachi(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (brake)
	{
		if (72 <= wave_stat)
			pulse_Mode = P_1;
		else if (56 <= wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 2 + (get_Amplitude(56, 72) - 2) / 16.0 * (72 - wave_stat);
		}
		else
		{
			amplitude = get_Amplitude(wave_stat, 72);
			pulse_Mode = Not_In_Sync;
			double base_freq = 1050;
			if (4 >= wave_stat)
				base_freq = 510 + ((wave_stat > 1) ? ((623 - 510) / 3.0 * (wave_stat - 1)) : 0);
			expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
		}
	}
	else
	{
		if (55 <= wave_stat)
			pulse_Mode = P_1;
		else if (40 <= wave_stat)
		{
			pulse_Mode = P_9;
			amplitude = 2 + (get_Amplitude(40, 55) - 2) / 15.0 * (55 - wave_stat);
		}
		else
		{
			amplitude = get_Amplitude(wave_stat, 55);
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = get_random_freq((int)1050, 30) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_toyo_GTO(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = 1.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;

	if (brake)
	{
		amplitude = get_Amplitude(wave_stat, 77);

		if (77 <= wave_stat)
			pulse_Mode = P_1;
		else if (74 <= wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 4.0 * (wave_stat - 74);
		}
		else if (69 <= wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8;
		}

		else if (60 <= wave_stat)
			pulse_Mode = P_3;
		else if (43 <= wave_stat)
			pulse_Mode = P_5;
		else if (25 <= wave_stat)
			pulse_Mode = P_9;
		else if (6 <= wave_stat && wave_stat <= 9)
		{
			expect_saw_angle_freq = M_2PI * (260 + (365 - 260) / 25.0 * (wave_stat - 3));
			pulse_Mode = Not_In_Sync;
		}
		else
		{
			if (wave_stat < 5)
			{
				Wave_Values wv;
				wv.sin_value = 0;
				wv.saw_value = 0;
				wv.pwm_value = 0;
				return wv;
			}
			pulse_Mode = Not_In_Sync;
			double base_freq = 260;
			if (wave_stat > 3)
				base_freq = (260 + (365 - 260) / 25.0 * (wave_stat - 3));
			expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
		}
	}
	else
	{
		amplitude = get_Amplitude(wave_stat, 60);

		if (60 <= wave_stat)
			pulse_Mode = P_1;
		else if (56 <= wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 5.0 * (wave_stat - 56);
		}

		else if (51 <= wave_stat)
			pulse_Mode = P_3;
		else if (43 <= wave_stat)
			pulse_Mode = P_5;
		else if (27 <= wave_stat)
			pulse_Mode = P_9;
		else if (6 <= wave_stat && wave_stat <= 9)
		{
			expect_saw_angle_freq = M_2PI * (260 + (365 - 260) / 23.0 * (wave_stat - 3));
			pulse_Mode = Not_In_Sync;
		}
		else
		{
			pulse_Mode = Not_In_Sync;

			double base_freq = 260;
			if (wave_stat > 3)
				base_freq = (260 + (365 - 260) / 23.0 * (wave_stat - 3));
			expect_saw_angle_freq = get_random_freq((int)base_freq, 30) * M_2PI;
		}
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_tokyu_9000_hitachi_gto(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = 0;
	double expect_saw_angle_freq = 0;
	Pulse_Mode pulse_mode = Not_In_Sync;
	if (brake) //settings for braking vvvf pattern
	{
		amplitude = get_Amplitude(wave_stat, 60);
		expect_saw_angle_freq = wave_stat;
		if (60 <= wave_stat)
		{
			pulse_mode = P_1;
		}
		else if (54 <= wave_stat)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 6.0 * (wave_stat - 54);
		}
		else if (50 <= wave_stat || (free_run && sin_angle_freq > 50 * M_2PI))
			pulse_mode = P_3;
		else if (41 <= wave_stat || (free_run && sin_angle_freq > 41 * M_2PI))
			pulse_mode = P_5;
		else if (27 <= wave_stat || (free_run && sin_angle_freq > 27 * M_2PI))
			pulse_mode = P_9;
		else if (14.5 <= wave_stat || (free_run && sin_angle_freq > 14.5 * M_2PI))
			pulse_mode = P_15;
		else if (8 <= wave_stat || (free_run && sin_angle_freq > 8 * M_2PI))
			pulse_mode = P_27;
		else if (7 <= wave_stat || (free_run && sin_angle_freq > 7 * M_2PI))
			pulse_mode = P_45;
		else if (5 <= wave_stat || (free_run && sin_angle_freq > 5 * M_2PI))
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
		amplitude = get_Amplitude(wave_stat, 45);
		expect_saw_angle_freq = wave_stat;
		if (45 <= wave_stat)
		{
			pulse_mode = P_1;
		}
		else if (43 <= wave_stat)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 2.0 * (wave_stat - 43);
		}
		else if (37 <= wave_stat || (free_run && sin_angle_freq > 37 * M_2PI))
			pulse_mode = P_3;
		else if (32 <= wave_stat || (free_run && sin_angle_freq > 32 * M_2PI))
			pulse_mode = P_5;
		else if (25 <= wave_stat || (free_run && sin_angle_freq > 25 * M_2PI))
			pulse_mode = P_9;
		else if (14 <= wave_stat || (free_run && sin_angle_freq > 14 * M_2PI))
			pulse_mode = P_15;
		else if (7 <= wave_stat || (free_run && sin_angle_freq > 7 * M_2PI))
			pulse_mode = P_27;
		else if (5 <= wave_stat || (free_run && sin_angle_freq > 5 * M_2PI))
			pulse_mode = P_45;
		else
		{
			expect_saw_angle_freq = M_2PI * 200;
			pulse_mode = Not_In_Sync;
		}
	}
	return calculate_common(pulse_mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_toei_6300_3(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 60);
	if (mascon_on == false && amplitude < 0.65)
		amplitude = 0.0;

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (60 <= wave_stat)
		pulse_Mode = P_1;
	else if (52.5 <= wave_stat || (free_run && sin_angle_freq > 52.5 * M_2PI))
		pulse_Mode = P_3;
	else if (15 <= wave_stat)
	{
		expect_saw_angle_freq = get_random_freq(1000, 20) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (5 <= wave_stat)
	{
		double expect_saw_freq = 220 + (1000 - 220) / 10 * (wave_stat - 5);
		expect_saw_angle_freq = get_random_freq((int)expect_saw_freq, 20) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else if (wave_stat <= 3 && brake)
	{
		double expect_saw_freq = 205 + (220 - 205) / 3.0 * (wave_stat);
		expect_saw_angle_freq = get_random_freq((int)expect_saw_freq, 20) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}
	else
	{
		expect_saw_angle_freq = get_random_freq(220, 20) * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_keihan_13000_toyo_IGBT(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{
	double amplitude = get_Amplitude(wave_stat, 55);

	if (53 <= wave_stat && wave_stat <= 55)
	{
		amplitude = 5 + (get_Amplitude(53, 55) - 5) / 2.0 * (55 - wave_stat);
	}

	double expect_saw_angle_freq = sin_angle_freq * 10;
	Pulse_Mode pulse_Mode = P_1;
	if (55 <= wave_stat)
		pulse_Mode = P_1;
	else if (34 <= wave_stat)
		pulse_Mode = P_9;
	else
	{
		expect_saw_angle_freq = 525 * M_2PI;
		pulse_Mode = Not_In_Sync;
	}

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}

Wave_Values calculate_tokyuu_5000(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat)
{

	double amplitude = get_Amplitude(wave_stat, 58);
	double expect_saw_angle_freq = 1;
	Pulse_Mode pulse_Mode = P_1;

	if (brake)
	{
		amplitude = get_Amplitude(wave_stat, 65);
		if (65 <= wave_stat)
			pulse_Mode = P_1;
		else if (61 <= wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 5.0 * (wave_stat - 61);
		}
		else if (50 <= wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)700 + 1100 / 11.0 * (wave_stat - 50); //170.0/54.0*(wave_stat);
			expect_saw_angle_freq = M_2PI * base_freq;
		}
		else if (23 <= wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)740 - 40.0 / (50 - 23) * (wave_stat - 23); //170.0/54.0*(wave_stat);
			expect_saw_angle_freq = base_freq * M_2PI;
		}
		else if (brake && wave_stat <= 4)
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
		if (58 <= wave_stat)
			pulse_Mode = P_1;
		else if (55 <= wave_stat)
		{
			pulse_Mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 4.0 * (wave_stat - 55);
		}
		else if (42 <= wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)700 + 1100 / 13.0 * (wave_stat - 42); //170.0/54.0*(wave_stat);
			expect_saw_angle_freq = M_2PI * base_freq;
		}
		else if (23 <= wave_stat)
		{
			pulse_Mode = Not_In_Sync;
			double base_freq = (double)740 - 40.0 / (46 - 23) * (wave_stat - 23); //170.0/54.0*(wave_stat);
			expect_saw_angle_freq = base_freq * M_2PI;
		}
		else
		{
			pulse_Mode = Not_In_Sync;
			expect_saw_angle_freq = M_2PI * 740;
		}
	}

	if (!mascon_on && free_run && wave_stat < 23)
		amplitude = 0;

	return calculate_common(pulse_Mode, expect_saw_angle_freq, initial_phase, amplitude);
}