/* @file    PWM_main.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    25/05/2018
 * @brief   PWM for main rotor motor
 */

/**************************************************************
 *    Generates PWM output for the helicopter main rotor      *
 *************************************************************/

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
#include "buttons.h"
#include "PWM_main.h"

/**********************************************************
 * Defined constants
 **********************************************************/

#define G_DUTY_CYCLE_MAIN_SIZE  5
#define MESSAGE_SIZE            24                 // Size of message for display
// PWM configuration
#define PWM_START_RATE_HZ       250
#define PWM_DIVIDER_CODE        SYSCTL_PWMDIV_1
#define PWM_DIVIDER             1
#define DIVIDER                 100

/**********************************************************
 * Global variables
 **********************************************************/

extern int g_duty_cycle_main = G_DUTY_CYCLE_MAIN_SIZE;

/**
 * Sets the PWM to a percentage (%) for the main rotor motor
 * @param pwm   PWM value
 */
void
setPWMmain ( int pwm )
{
    g_duty_cycle_main = pwm;
    uint32_t ui32Period = SysCtlClockGet () / PWM_DIVIDER / PWM_START_RATE_HZ;
    PWMPulseWidthSet ( PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * g_duty_cycle_main / DIVIDER );
}

/**
 * Returns the PWM duty cycle as a percentage (%) for the main rotor motor
 */
int
getPWMmain ( void )
{
    return g_duty_cycle_main;
}

/**
 * Update to LED display the PWM duty cycle as a percentage (%) for main rotor
 */
void
updateDisplayPWMmain ( void )
{
    char cMessage [ MESSAGE_SIZE ];
    usprintf ( cMessage, "Main output: %d ", g_duty_cycle_main );
    OLEDStringDraw ( cMessage, 0, 0 );
}

/**
 * Initializes the PWM for the main rotor motor
 */
void
initPWMmain ( void )
{
    // Ensure that peripherals are re-enabled
    SysCtlPeripheralEnable ( PWM_MAIN_PERIPH_PWM );
    SysCtlPeripheralEnable ( PWM_MAIN_PERIPH_GPIO );

    GPIOPinConfigure ( PWM_MAIN_GPIO_CONFIG );

    GPIOPinTypePWM ( PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN );

    // Calculate the PWM period corresponding to PWM_START_RATE_HZ  .
    uint32_t ui32Period = SysCtlClockGet () / PWM_DIVIDER / PWM_START_RATE_HZ;

    PWMGenConfigure ( PWM_MAIN_BASE, PWM_MAIN_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC );

    PWMGenPeriodSet ( PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period );

    // Set the pulse width for PWM_START_PC % duty cycle.
    PWMPulseWidthSet ( PWM_MAIN_BASE, PWM_MAIN_OUTNUM, ui32Period * g_duty_cycle_main / DIVIDER );

    PWMGenEnable ( PWM_MAIN_BASE, PWM_MAIN_GEN );

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState ( PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false );

    // Updates the LED dsiplay on the Tiva board
    updateDisplayPWMmain ();
}
