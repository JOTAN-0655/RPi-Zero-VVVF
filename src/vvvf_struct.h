#ifndef VVVF_STRUCT_H
#define VVVF_STRUCT_H

#include "rpi_lib/gpio.h"

typedef enum
{
	Async,
	P_1,
	P_Wide_3,
	P_10,
	P_12,
	P_18,

	Async_THI,
	// Current harmonic minimum Pulse width modulation
	CHMP_3,
	CHMP_Wide_3,
	CHMP_5,
	CHMP_Wide_5,
	CHMP_7,
	CHMP_Wide_7,
	CHMP_9,
	CHMP_Wide_9,
	CHMP_11,
	CHMP_Old_11,
	CHMP_Wide_11,
	CHMP_13,
	CHMP_Old_13,
	CHMP_Wide_13,
	CHMP_15,
	CHMP_Old_15,
	CHMP_Wide_15,
	// Selective harmonic elimination Pulse width modulation
	SHEP_3,
	SHEP_5,
	SHEP_7,
	SHEP_9,
	SHEP_11,
	SHEP_13,
	SHEP_15,

	P_3,
	P_5,
	P_7,
	P_9,
	P_11,
	P_13,
	P_15,
	P_17,
	P_19,
	P_21,
	P_23,
	P_25,
	P_27,
	P_29,
	P_31,
	P_33,
	P_35,
	P_37,
	P_39,
	P_41,
	P_43,
	P_45,
	P_47,
	P_49,
	P_51,
	P_53,
	P_55,
	P_57,
	P_59,
	P_61,

	SP_1,
	SP_Wide_3,
	SP_10,
	SP_12,
	SP_18,
	SP_3,
	SP_5,
	SP_7,
	SP_9,
	SP_11,
	SP_13,
	SP_15,
	SP_17,
	SP_19,
	SP_21,
	SP_23,
	SP_25,
	SP_27,
	SP_29,
	SP_31,
	SP_33,
	SP_35,
	SP_37,
	SP_39,
	SP_41,
	SP_43,
	SP_45,
	SP_47,
	SP_49,
	SP_51,
	SP_53,
	SP_55,
	SP_57,
	SP_59,
	SP_61
} Pulse_Mode;

typedef struct
{
	double sin_angle_freq;
	double sin_time;
	double saw_angle_freq;
	double saw_time;
} Calculate_Values;

/**
 * @brief 
 * brake , mascon_on , free_run , control_freq , sine_freq
 */
typedef struct {
	bool brake;
	bool mascon_on;
	bool free_run;
	double control_freq;
    double sine_freq;
} Control_Values;

/**
 * @brief 
 * pulse_mode , amplitude , carrier_angle_freq ,dipolar , level
 */
typedef struct {
    Pulse_Mode pulse_mode;
    double amplitude;
    double carrier_angle_freq;
    double dipolar;
    char level;
} Carrier_Settings;

#endif