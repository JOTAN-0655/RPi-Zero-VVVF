#include "rpi_lib/gpio.h"


typedef struct 
{
    double sin_value;
    double saw_value;
    double pwm_value;
} Wave_Values;

typedef enum {
    Not_In_Sync,P_1, P_Wide_3, P_10 ,P_12,P_18, P_3,P_5,P_7,P_9,P_11,P_13,P_15,P_17,P_19,
    P_21,P_23, P_25, P_27, P_29, P_31, P_33, P_35, P_37, P_39, P_41
    , P_43, P_45, P_47, P_49, P_51, P_53, P_55, P_57, P_59, P_61,

    SP_1, SP_Wide_3, SP_10, SP_12, SP_18,
	SP_3, SP_5, SP_7, SP_9, SP_11, SP_13, SP_15, SP_17, SP_19,
	SP_21, SP_23, SP_25, SP_27, SP_29, SP_31, SP_33, SP_35, SP_37, SP_39, SP_41
	, SP_43, SP_45, SP_47, SP_49, SP_51, SP_53, SP_55, SP_57, SP_59, SP_61
} Pulse_Mode;

double get_saw_value_simple(double x);

double get_saw_value(double time, double angle_frequency, double initial_phase);
double get_sin_value(double time, double angle_frequency, double initial_phase, double amplitude);
double get_pwm_value(double sin_value, double saw_value);


double get_Amplitude(double freq,double max_freq);
Wave_Values get_P_with_saw(double time, double sin_angle_frequency, double initial_phase, double voltage,double carrier_mul,bool saw_oppose);
Wave_Values get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage,bool saw_oppose);
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

Wave_Values calculate_common(Pulse_Mode pulse_mode,double expect_saw_angle_freq,double initial_phase,double amplitude);
Wave_Values calculate_E231(bool brake,bool mascon_on, bool free_run,double initial_phase,double wave_stat);
Wave_Values calculate_207(bool brake,bool mascon_on, bool free_run,double initial_phase,double wave_stat);
Wave_Values calculate_doremi(bool brake,bool mascon_on, bool free_run,double initial_phase,double wave_stat);
Wave_Values calculate_E235(bool brake,bool mascon_on, bool free_run,double initial_phase,double wave_stat);
Wave_Values calculate_E209(bool brake,bool mascon_on, bool free_run,double initial_phase,double wave_stat);
Wave_Values calculate_9820_mitsubishi(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_9820_hitachi(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_E233(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_silent(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_mitsubishi_gto(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_toyo_IGBT(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_Famima(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_real_doremi(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_toubu_50050(bool brake,bool mascon_on, bool free_run, double initial_phase,double wave_stat);
Wave_Values calculate_207_1000_update(bool brake,bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_225_5100_mitsubishi(bool brake,bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_321_hitachi(bool brake,bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_toyo_GTO(bool brake,bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_tokyu_9000_hitachi_gto(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_toei_6300_3(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_keihan_13000_toyo_IGBT(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat);
Wave_Values calculate_tokyuu_5000(bool brake, bool mascon_on, bool free_run, double initial_phase, double wave_stat);