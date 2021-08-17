#define USE_FAST_CALICULATE

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

//sin value definitions
extern double sin_angle_freq,sin_time;

//saw value definitions
extern double saw_angle_freq,saw_time;

extern int saw_in_sync_mode,random_freq_move_count;

void reset_all_variables();

Wave_Values caliculate_E231(double initial_phase);
Wave_Values caliculate_207(char brake,double initial_phase);
Wave_Values caliculate_doremi(char brake,double initial_phase);
Wave_Values caliculate_E235(char brake,double initial_phase);
Wave_Values caliculate_silent(char brake,double initial_phase);