#define USE_FAST_CALICULATE

#include "rpi_lib/gpio.h"


typedef struct 
{
    double sin_value;
    double saw_value;
    double pwm_value;
} Wave_Values;

void generate_sin_table();
void test_sin_table();

double from_sin_table(double radian);
double get_saw_value_simple(double x);

double get_saw_value(double time, double angle_frequency, double initial_phase);
double get_sin_value(double time, double angle_frequency, double initial_phase, double amplitude);
double get_pwm_value(double sin_value, double saw_value);

double mod_d(double a,double b);
double get_Amplitude(double freq);

//sin value definitions
extern double sin_angle_freq,sin_time;

//saw value definitions
extern double saw_angle_freq,saw_time;

extern int saw_in_sync_mode,random_freq_move_count;

void reset_all_variables();

Wave_Values caliculate_E231(bool brake,double initial_phase);
Wave_Values caliculate_207(bool brake,double initial_phase);
Wave_Values caliculate_doremi(bool brake,double initial_phase);
Wave_Values caliculate_E235(bool brake,double initial_phase);
Wave_Values caliculate_E209(bool brake,double initial_phase);
Wave_Values caliculate_silent(bool brake,double initial_phase);