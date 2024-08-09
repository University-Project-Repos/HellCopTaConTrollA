/* @file    PID.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/05/2018
 * @brief   PID file
 */

#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"
#include "stdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "circBufT.h"
#include "./OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "PWM_main.h"
#include "PWM_tail.h"
#include "height.h"
#include "yaw.h"
#include "buttons.h"
#include "PID.h"
#include "UART.h"

/**********************************************************
 * Defined constants
 **********************************************************/

#define PWM_MAIN_BASE           PWM0_BASE
#define PWM_MAIN_OUTBIT         PWM_OUT_7_BIT
#define PWM_TAIL_BASE           PWM1_BASE
#define PWM_TAIL_OUTBIT         PWM_OUT_5_BIT
#define MAX_STR_LEN             16
#define MAIN_PROPORTIONAL_GAIN  0.8
#define MAIN_INTEGRAL_GAIN      0.01
#define MAIN_DIFFERENTIAL_GAIN  0.01
#define TAIL_PROPORTIONAL_GAIN  0.8
#define TAIL_INTEGRAL_GAIN      0.01
#define TAIL_DIFFERENTIAL_GAIN  0       // Can be adjusted for tail gain
#define FALSE                   0
#define TRUE                    1
#define MIN_CYCL_RNG            5       // Minimum control duty cycle
#define MAX_CYCL_RNG            95      // Maximum control duty cycle
#define MIN_DTY_CYCL            0       // Minimum duty cycle
#define MAX_DTY_CYCL            98      // Maximum duty cycle
#define MAIN_OFFSET             10
#define TAIL_STABALIZER         0.85    // Stabilizes and centers helicopter
#define G_STATE_SIZE            20

/**********************************************************
 * Global variables
 **********************************************************/

char g_status_str [ MAX_STR_LEN + 1 ];
extern char g_state [ G_STATE_SIZE ];

_Bool g_pwm_on = FALSE;

float g_main_intgrl = 0;
float g_main_dfrntl = 0;
float g_main_last_error = 0;
float g_tail_intgrl = 0;
float g_tail_dfrntl = 0;
float g_tail_last_error = 0;
float g_main_integral_error = 0;
float g_tail_integral_error = 0;
float g_main_differential_error = 0;
float g_tail_differential_error = 0;
extern int g_duty_cycle_main;
extern int g_duty_cycle_tail;
extern int8_t g_altitude_percent;
extern int16_t g_yaw_angle;

/**
 * This function ensures that the PWM outputs are disabled when the
 * helicopter is landed, and on when the helicopter is flying/calibrating
 */
void
PWMtoggle ( void )
{
    if (( strcmp ( g_state, "Flying" ) == 0 || strcmp ( g_state, "Calibration" ) == 0 ) && !g_pwm_on )
    {
        g_pwm_on = TRUE;
        // Turn PWMs on
        PWMOutputState ( PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true );
        PWMOutputState ( PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true );
    }

    if ( strcmp ( g_state, "landed" ) == 0 && g_pwm_on && getAltitudePercentage () == 0 )
    {
        g_pwm_on = FALSE;
        // Turn PWMs off
        PWMOutputState ( PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false );
        PWMOutputState ( PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false );
    }
}

/**
 *  Controls helicopter height by altering the duty cycle of main PWM output
 */
void
mainControl ( void )
{
    //calculates the proportional and differential errors for the height
    float main_error = g_altitude_percent - getAltitudePercentage ();
    g_main_differential_error = main_error - g_main_last_error;
    g_main_last_error = main_error;

    //multiplies the three error signals by their respective gains
    float main_prptnl = MAIN_PROPORTIONAL_GAIN * main_error;
    g_main_intgrl =  MAIN_INTEGRAL_GAIN * g_main_integral_error;
    g_main_dfrntl = MAIN_DIFFERENTIAL_GAIN * g_main_differential_error;

    // Combines error signals into a single control signal to send to the motor
    int main_control = MAIN_OFFSET + main_prptnl + g_main_intgrl + g_main_dfrntl;

    g_duty_cycle_main = main_control;


    // Prevents integrator wind-up when calculating the integral error
    if ( g_duty_cycle_main > MIN_CYCL_RNG && g_duty_cycle_main < MAX_CYCL_RNG )
    {
        g_main_integral_error += main_error;
    }

    // Prevents the control signal exceeding the allowed duty cycle
    g_main_integral_error += main_error;

    if ( g_duty_cycle_main < MIN_CYCL_RNG )
    {
        g_duty_cycle_main = MIN_DTY_CYCL;
    }

    if ( g_duty_cycle_main >= MAX_CYCL_RNG )
    {
        g_duty_cycle_main = MAX_DTY_CYCL;
    }

    updateDisplayPWMmain ();

    setPWMmain ( g_duty_cycle_main );
}

/**
* Controls helicopter yaw by altering the duty cycle of the tail PWM output
*/
void
tailControl ( void )
{
    // Calculates the proportional and differential errors
    float tail_error = g_yaw_angle - getYaw ();
    g_tail_differential_error = tail_error - g_tail_last_error;
    g_tail_last_error = tail_error;

    // Multiplies the three error signals by their respective gains
    float tail_prprtnl = TAIL_PROPORTIONAL_GAIN * tail_error;
    g_tail_intgrl = TAIL_INTEGRAL_GAIN * g_tail_integral_error;
    g_tail_dfrntl = TAIL_DIFFERENTIAL_GAIN * g_tail_differential_error;

    // Combines error signals into a single control signal to send to the motor
    int tail_control = tail_prprtnl + g_tail_intgrl + g_tail_dfrntl;

    g_duty_cycle_tail = ( g_duty_cycle_main * TAIL_STABALIZER + tail_control );

    // Prevents integrator wind-up when calculating the integral error
    if ( g_duty_cycle_tail > MIN_CYCL_RNG && g_duty_cycle_tail < MAX_CYCL_RNG )
    {
        g_tail_integral_error += tail_error;
    }

    // Prevents the control signal exceeding the allowed duty cycle
    if ( g_duty_cycle_tail < MIN_CYCL_RNG )
    {
        g_duty_cycle_tail = MIN_DTY_CYCL;
    }

    if ( g_duty_cycle_tail >= MAX_CYCL_RNG )
    {
        g_duty_cycle_tail = MAX_DTY_CYCL;
    }
    updateDisplayPWMtail ();

    setPWMtail ( g_duty_cycle_tail );
}

/**
 * Displays PID related information to a terminal for debugging purposes
 */
void
UARTmessage ( void )
{
    // Displays the current and target yaw (�)
    usprintf ( g_status_str, "Yaw: %2d [%2d] \r\n", ( int ) getYaw (), g_yaw_angle );
    sendUART ( g_status_str );

    // Displays the current and target altitude (%)
    usprintf ( g_status_str, "Alt: %2d [%2d] \r\n", ( int ) getAltitudePercentage (), g_altitude_percent );
    sendUART ( g_status_str );

    // Displays the PWM for the main rotor motor
    usprintf ( g_status_str, "Main %2d tail %2d \r\n", ( int ) g_duty_cycle_main, ( int ) g_duty_cycle_tail );
    sendUART ( g_status_str );

    // Displays the PWM for the tail rotor motor
    usprintf ( g_status_str, "Mode %s \r\n", g_state );
    sendUART ( g_status_str );

    // Displays a break point to distinguish between each UART output message
    usprintf ( g_status_str, "-------------- \r\n" );
    sendUART ( g_status_str );
}
