#ifndef VVVF_CALCULATE_H

#define VVVF_CALCULATE_H

#include "rpi_lib/gpio.h"


typedef struct {
	bool brake;
	bool mascon_on;
	bool free_run;
	double initial_phase;
	double wave_stat;
} Control_Values;

typedef enum {
    Async,P_1, P_Wide_3, P_10 ,P_12,P_18, 

	
	// Current harmonic minimum Pulse width modulation
	CHMP_3, CHMP_Wide_3, CHMP_5, CHMP_Wide_5, CHMP_7, CHMP_Wide_7,
	CHMP_9, CHMP_Wide_9, CHMP_11, CHMP_Old_11, CHMP_Wide_11, CHMP_13,
	CHMP_Old_13, CHMP_Wide_13, CHMP_15, CHMP_Old_15, CHMP_Wide_15,		
	// Selective harmonic elimination Pulse width modulation
	SHEP_3, SHEP_5, SHEP_7, SHEP_9, SHEP_11, SHEP_13, SHEP_15,
	
	
	P_3,P_5,P_7,P_9,P_11,P_13,P_15,P_17,P_19,
    P_21,P_23, P_25, P_27, P_29, P_31, P_33, P_35, P_37, P_39, P_41
    , P_43, P_45, P_47, P_49, P_51, P_53, P_55, P_57, P_59, P_61,

    SP_1, SP_Wide_3, SP_10, SP_12, SP_18,
	SP_3, SP_5, SP_7, SP_9, SP_11, SP_13, SP_15, SP_17, SP_19,
	SP_21, SP_23, SP_25, SP_27, SP_29, SP_31, SP_33, SP_35, SP_37, SP_39, SP_41
	, SP_43, SP_45, SP_47, SP_49, SP_51, SP_53, SP_55, SP_57, SP_59, SP_61
} Pulse_Mode;

typedef enum  { 
	Sound_E231, Sound_207, Sound_doremi, Sound_E235, Sound_E209, Sound_9820_mitsubishi, Sound_9820_hitachi,
	Sound_E233, Sound_silent, Sound_mitsubishi_gto, Sound_toyo_IGBT, Sound_Famima, Sound_real_doremi, Sound_toubu_50050,
	Sound_207_1000_update, Sound_225_5100_mitsubishi, Sound_321_hitachi, Sound_toyo_GTO, Sound_keihan_13000_toyo_IGBT,
	Sound_toei_6300_3, Sound_tokyu_9000_hitachi_gto, Sound_tokyuu_5000, Sound_keio_8000_gto, Sound_tokyuu_1000_1500_IGBT,
	Sound_E233_3000
} VVVF_Sound_Names;

double get_saw_value_simple(double x);

double get_saw_value(double x);
double get_sin_value(double x, double amplitude);
char get_pwm_value(double sin_value, double saw_value);


double get_Amplitude(double freq,double max_freq);
char get_P_with_saw(double time, double sin_angle_frequency, double initial_phase, double voltage,double carrier_mul,bool saw_oppose);
char get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage,bool saw_oppose);
int get_Pulse_Num(Pulse_Mode mode);

//sin value definitions
extern double sin_angle_freq,sin_time;

//saw value definitions
extern double saw_angle_freq,saw_time;

extern bool disconnect;

extern int random_freq_move_count;
extern int pre_saw_random_freq;

void reset_all_variables();
int get_random_freq(int base_freq, int range);
double get_pattern_random(int lowest, int highest,int interval_count);

char calculate_two_level(Pulse_Mode pulse_mode,double expect_saw_angle_freq,double initial_phase,double amplitude);
char calculate_three_level(Pulse_Mode pulse_mode, double expect_saw_angle_freq, double initial_phase, double amplitude,double dipolar);


#endif 