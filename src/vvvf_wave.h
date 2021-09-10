#define USE_FAST_CALICULATE

#include "rpi_lib/gpio.h"


typedef struct 
{
    double sin_value;
    double saw_value;
    double pwm_value;
} Wave_Values;

typedef enum {
    Not_In_Sync,P_1, P_Wide_3,P_10 ,P_12,P_18, P_3,P_5,P_7,P_9,P_11,P_13,P_15,P_17,P_19,
    P_21,P_23, P_25, P_27, P_29, P_31, P_33, P_35, P_37, P_39, P_41
    , P_43, P_45, P_47, P_49, P_51, P_53, P_55, P_57, P_59, P_61
} Pulse_Mode;

void generate_sin_table();
void test_sin_table();
double mod_d(double a,double b);

double from_sin_table(double radian);
double get_saw_value_simple(double x);

double get_saw_value(double time, double angle_frequency, double initial_phase);
double get_sin_value(double time, double angle_frequency, double initial_phase, double amplitude);
double get_pwm_value(double sin_value, double saw_value);


double get_Amplitude(double freq,double max_freq);
Wave_Values get_P_with_saw(double time, double sin_angle_frequency, double initial_phase, double voltage,double carrier_mul);
Wave_Values get_Wide_P_3(double time, double angle_frequency, double initial_phase, double voltage);
int get_Pulse_Num(Pulse_Mode mode);

//sin value definitions
extern double sin_angle_freq,sin_time;

//saw value definitions
extern double saw_angle_freq,saw_time;

extern bool disconnect;

extern int random_freq_move_count;

void reset_all_variables();

Wave_Values caliculate_common(Pulse_Mode pulse_mode,double expect_saw_angle_freq,double initial_phase,double amplitude);
Wave_Values caliculate_E231(bool brake,double initial_phase);
Wave_Values caliculate_207(bool brake,double initial_phase);
Wave_Values caliculate_doremi(bool brake,double initial_phase);
Wave_Values caliculate_E235(bool brake,double initial_phase);
Wave_Values caliculate_E209(bool brake,double initial_phase);
Wave_Values caliculate_silent(bool brake,double initial_phase);