#include "my_math.h"
#include "vvvf_wave.h"
#include "vvvf_calculate.h"

void calculate_E231(Control_Values *cv , Carrier_Settings *cs)
{
	double amplitude = 0;
	double carrier_angle_freq = 0;
	Pulse_Mode pulse_mode = P_1;
	if (cv->brake)
	{
		amplitude = get_Amplitude(cv->control_freq, 73);
		if (cv->control_freq > 73)
			pulse_mode = P_1;
		else if (cv->control_freq > 67)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.1 / 8.0 * (cv->control_freq - 67);
		}
		else if (cv->control_freq >= 56)
		{
			double expect_saw_freq = 700 + (1600 - 700) / 11 * (cv->control_freq - 56);
			carrier_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Async;
		}
		else if (cv->control_freq >= 29)
		{
			double expect_saw_freq = 1045 + (700 - 1045) / (55.9 - 29) * (cv->control_freq - 29);
			carrier_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Async;
		}
		else
		{
			carrier_angle_freq = 1045 * M_2PI;
			pulse_mode = Async;
		}
	}
	else
	{
		amplitude = get_Amplitude(cv->control_freq, 65);
		if (cv->control_freq > 67)
			pulse_mode = P_1;
		else if (cv->control_freq > 60)
		{
			pulse_mode = P_Wide_3;
			amplitude = 0.8 + 0.2 / 8.0 * (cv->control_freq - 60);
		}
		else if (cv->control_freq >= 49)
		{
			double expect_saw_freq = 710 + (1750 - 710) / 11 * (cv->control_freq - 49);
			carrier_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Async;
		}
		else if (cv->control_freq >= 23)
		{
			double expect_saw_freq = 1045 + (710 - 1045) / (48.9 - 23) * (cv->control_freq - 23);
			carrier_angle_freq = M_2PI * expect_saw_freq;
			pulse_mode = Async;
		}
		else
		{
			carrier_angle_freq = 1045 * M_2PI;
			pulse_mode = Async;
		}
	}

	cs->amplitude = amplitude;
	cs->carrier_angle_freq = carrier_angle_freq;
	cs->pulse_mode = pulse_mode;
	cs->level = 2;
	cs->dipolar = -1;

}

void calculate_keikyu_n1000_siemens_gto_2_level(Control_Values *cv , Carrier_Settings *cs)
{
	int a = 2, b = 3;
	double k[2][3] = {
		{0.0193294460641, 0.0222656250000, 0},
		{0.014763975813, 0.018464, 0.013504901961},
	};
	double B[2][3] = {
		{0.10000000000, -0.07467187500, 0},
		{0.10000000000, -0.095166, 0.100000000000},
	};

	Pulse_Mode pulse_mode;
	double carrier_angle_freq = 1;
	double amplitude;

	if (!cv->brake)
	{
		if (80 <= cv->control_freq)
		{
			pulse_mode = P_1;
		}
		else if (59 <= cv->control_freq)
		{
			a = 1;
			b = 2;
			pulse_mode = CHMP_Wide_3;
		}
		else if (cv->free_run && cv->sine_freq >= 57)
		{
			pulse_mode = P_3;
		}
		else if (57 <= cv->control_freq)
		{
			a = 1;
			b = 2;
			pulse_mode = CHMP_Wide_5;
		}
		else if (53.5 <= cv->control_freq)
		{
			a = 1;
			b = 2;
			pulse_mode = CHMP_Wide_7;
		}
		else if (43.5 <= cv->control_freq)
		{
			a = 1;
			b = 1;
			pulse_mode = CHMP_7;
		}
		else if (36.7 <= cv->control_freq)
		{
			a = 1;
			b = 1;
			pulse_mode = CHMP_9;
		}
		else if (30 <= cv->control_freq)
		{
			a = 1;
			b = 1;
			pulse_mode = CHMP_11;
		}
		else if (27 <= cv->control_freq)
		{
			a = 1;
			b = 1;
			pulse_mode = CHMP_13;
		}
		else if (24 <= cv->control_freq)
		{
			a = 1;
			b = 1;
			pulse_mode = CHMP_15;
		}
		else
		{
			a = 1;
			b = 1;
			double expect_saw_freq = 400;
			pulse_mode = Async_THI;
			if (5.6 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 5.6))
				expect_saw_freq = 400;
			else if (5 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 5.0))
				expect_saw_freq = 350;
			else if (4.3 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 4.3 ))
				expect_saw_freq = 311;
			else if (3.4 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 3.4))
				expect_saw_freq = 294;
			else if (2.7 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 2.7))
				expect_saw_freq = 262;
			else if (2.0 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 2.0))
				expect_saw_freq = 233;
			else if (1.5 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 1.5))
				expect_saw_freq = 223;
			else if (0.5 <= cv->control_freq || (cv->free_run && cv->sine_freq >= 0.5))
				expect_saw_freq = 196;
			else
				expect_saw_freq = 175;

			carrier_angle_freq = expect_saw_freq * M_2PI;
		}
	}
	else
	{
		if (79.5 <= cv->control_freq)
		{
			pulse_mode = P_1;
		}
		else if (70.7 <= cv->control_freq)
		{
			a = 2;
			b = 2;
			pulse_mode = CHMP_Wide_3;
		}
		else if (cv->free_run && cv->sine_freq >= 63.35)
		{
			pulse_mode = P_3;
		}
		else if (63.35 <= cv->control_freq)
		{
			a = 2;
			b = 2;
			pulse_mode = CHMP_Wide_5;
		}
		else if (56.84 <= cv->control_freq)
		{
			a = 2;
			b = 2;
			pulse_mode = CHMP_Wide_7;
		}
		else if (53.5 <= cv->control_freq)
		{
			a = 2;
			b = 1;
			pulse_mode = CHMP_7;
		}
		else if (41 <= cv->control_freq)
		{
			a = 2;
			b = 3;
			pulse_mode = CHMP_7;
		}
		else if (34.5 <= cv->control_freq)
		{
			a = 2;
			b = 3;
			pulse_mode = CHMP_9;
		}
		else if (28.9 <= cv->control_freq)
		{
			a = 2;
			b = 3;
			pulse_mode = CHMP_11;
		}
		else if (24.9 <= cv->control_freq)
		{
			a = 2;
			b = 3;
			pulse_mode = CHMP_13;
		}
		else if (22.4 <= cv->control_freq)
		{
			a = 2;
			b = 3;
			pulse_mode = CHMP_15;
		}
		else if (cv->control_freq > 4)
		{
			a = 2;
			b = 3;
			carrier_angle_freq = 400 * M_2PI;
			pulse_mode = Async_THI;
		}
		else
		{
			cs -> amplitude = 0;
			cs -> pulse_mode = Async;
			cs -> carrier_angle_freq = 400;
			cs -> level = 2;
			cs -> dipolar = -1;
			return;
		}
	}

	amplitude =  ((k[a - 1][b - 1] * cv->control_freq) + B[a - 1][b - 1]);
	amplitude = amplitude >= 1.25 ? 1.25 : amplitude;

	if (cv->free_run && amplitude < 0.498)
		amplitude = 0;
	if (cv->control_freq == 0)
		amplitude = 0;
	if (pulse_mode == P_3)
		amplitude /= 1.25;

	cs->amplitude = amplitude;
	cs->pulse_mode = pulse_mode;
	cs->carrier_angle_freq = carrier_angle_freq;
	cs->level = 2;
	cs->dipolar = -1;
}