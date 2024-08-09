/* @file    PWM_tail.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    25/05/2018
 * @brief   PWM for tail rotor motor
 */

/**************************************************************
 *    Generates PWM output for the helicopter tail rotor      *
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
#include "PWM_tail.h"

/**********************************************************
 * Defined constants
 **********************************************************/

#define MESSAGE_SIZE            24                 // Size of message for display
// PWM configuration
#define PWM_START_RATE_HZ       250
#define PWM_DIVIDER_CODE        SYSCTL_PWMDIV_1
#define PWM_DIVIDER             1
#define DIVIDER                 100

/**********************************************************
 * Global variables
 **********************************************************/

extern int g_duty_cycle_tail = 5;

/**
 * Sets the PWM to a percentage (%) for the tail rotor motor
 * @param pwm: The PWM value
 */
void
setPWMtail ( int pwm )
{
    g_duty_cycle_tail = pwm;
    uint32_t ui32Period = SysCtlClockGet () / PWM_DIVIDER / PWM_START_RATE_HZ;
    PWMPulseWidthSet ( PWM_TAIL_BASE, PWM_TAIL_OUTNUM, ui32Period * g_duty_cycle_tail / DIVIDER );
}

/**
 * Update to LED display the PWM duty cycle as a percentage (%) for tail rotor
 */
int
getPWMtail ( void )
{
    return g_duty_cycle_tail;
}

/**
 * Returns the PWM duty cycle as a percentage (%) for the tail rotor motor
 */
void
updateDisplayPWMtail ( void )
{
    char cMessage [ MESSAGE_SIZE ];
    usprintf ( cMessage, "Tail output: %d ", g_duty_cycle_tail );
    OLEDStringDraw ( cMessage, 0, 1 );
}

/**
 * Initializes the PWM for the tail rotor motor
 */
void
initPWMtail ( void )
{
    // Ensure that peripherals are re-enabled
    SysCtlPeripheralEnable ( PWM_TAIL_PERIPH_PWM );
    SysCtlPeripheralEnable ( PWM_TAIL_PERIPH_GPIO );

    GPIOPinConfigure ( PWM_TAIL_GPIO_CONFIG );

    GPIOPinTypePWM ( PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN );

    // Calculate the PWM period corresponding to PWM_START_RATE_HZ.
    uint32_t ui32Period = SysCtlClockGet () / PWM_DIVIDER / PWM_START_RATE_HZ;

    PWMGenConfigure ( PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC );
    PWMGenPeriodSet ( PWM_TAIL_BASE, PWM_TAIL_GEN, ui32Period );

    // Set the pulse width for PWM_START_PC % duty cycle.
    PWMPulseWidthSet ( PWM_TAIL_BASE, PWM_TAIL_OUTNUM, ui32Period * g_duty_cycle_tail / DIVIDER );

    PWMGenEnable ( PWM_TAIL_BASE, PWM_TAIL_GEN );

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState ( PWM_TAIL_BASE, PWM_TAIL_OUTBIT, false );

    // Updates the LED dsiplay on the Tiva board
    updateDisplayPWMtail ();
}
