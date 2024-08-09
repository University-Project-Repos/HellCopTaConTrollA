/* @file    main.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    15/03/2018
 * @brief   Program for Tiva Board piloted helicopter
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "./OrbitOLED/OrbitOLEDInterface.h"
#include "circBufT.h"
#include "height.h"
#include "yaw.h"
#include "buttons.h"
#include "PWM_main.h"
#include "PWM_tail.h"
#include "PID.h"
#include "UART.h"

//****************************************************************************
// Defined constants
//****************************************************************************

#define SAMPLE_RATE_HZ          80      // Equation: 2NFm (2 * 10 * 4)
#define COUNT                   150     // For the SysCtlDelay
#define SEQUENCE_NO             3       // Process trigger sequence number
#define TIVA_DISPLAY_STEP       25      // Value for Tiva display increments
#define BUTTON_TIMER_STEP       4       // Value for button display increments
#define UART_TIMER_STEP         75      // Value for UART timer increments

//****************************************************************************
// Global variables
//****************************************************************************

static uint32_t g_ulSampCnt = 0;               // Counter for the interrupts
uint32_t g_tiva_display = TIVA_DISPLAY_STEP;   // Timer for Tiva display
uint32_t g_button_timer = BUTTON_TIMER_STEP;   // Timer for buttons
uint32_t g_UART_timer = UART_TIMER_STEP;       // Timer for UART
uint32_t g_control_timer = 1;                  // Timer for control
extern float g_yaw_angle;                      // External global for yaw angle

/**
 * The interrupt handler for the SysTick interrupt
 */
void
SysTickIntHandler ( void )
{
    // Trigger ADC conversion
    ADCProcessorTrigger ( ADC0_BASE, SEQUENCE_NO );

    // Counts the number of interrupts
    ++g_ulSampCnt;
}

/**
 * Sets program initializers
 */
void
initClock ( void )
{
    // Sets the clock rate to 20 MHz
    SysCtlClockSet ( SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ );

    // Sets the Sys Tick period
    SysTickPeriodSet ( SysCtlClockGet () / SAMPLE_RATE_HZ );

    // Sets regular interrupts
    SysTickIntRegister ( SysTickIntHandler );

    // Initializes SysTick interrupt enabler
    SysTickIntEnable ();

    // Enables the SysTick interrupt
    SysTickEnable ();
}

/**
 * Initialize the Orbit OLED display
 */
void
initDisplay ( void )
{
    OLEDInitialise ();  // Initializes the OLED
}

/**
 * Resets the SysCtl peripherals for main and tail rotors
 */
void
peripheralReset ( void )
{
    SysCtlPeripheralReset ( PWM_MAIN_PERIPH_PWM );  // Resets peripheral PWM 0
    SysCtlPeripheralReset ( PWM_MAIN_PERIPH_GPIO ); // Resets peripheral GPIO C
    SysCtlPeripheralReset ( PWM_TAIL_PERIPH_PWM );  // Resets peripheral PWM 1
    SysCtlPeripheralReset ( PWM_TAIL_PERIPH_GPIO ); // Resets peripheral GPIO F
}

/**
 * Enables all ports
 */
void
peripheralEnable ( void )
{
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOA ); // Enable MODE, RESET port
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOB ); // Enable yaw channel port
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOC ); // Enable yaw, main port
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOD ); // Enable DOWN port
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOE ); // Enable UP, altitude port
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF ); // Enable CW and CCW port
}

int
main ( void )
{
    // Initializers
    peripheralReset ();     // Resets peripherals for main and tail rotors
    IntMasterDisable ();    // Disables all internal and external interrupts
    initClock ();           // Initializes the clock rate
    peripheralEnable ();    // Enables all ports
    initDisplay ();         // Initializes the LED display
    initButtons ();         // Initializes the buttons
    initADC ();             // Initializes the ADC
    initYaw ();             // Initializes the yaw
    initPWMmain ();         // Initializes the main rotor PWM
    initPWMtail ();         // Initializes the tail rotor PWM
    initUART ();            // Initializes the UART
    IntMasterEnable ();     // Re-enables all internal and external interrupts

    // Sets the initial ADC ground value
    ADCheightReference ();

    while ( 1 )
    {
        if ( g_ulSampCnt > g_control_timer ) {
            g_control_timer += 1;
            mainControl ();
            tailControl ();
        }

        if ( g_ulSampCnt > g_button_timer ) {
            g_button_timer += BUTTON_TIMER_STEP;
            checkButState ();
            PWMtoggle ();
            stateHandler ();
        }

        if ( g_ulSampCnt > g_tiva_display ) {
            g_tiva_display += TIVA_DISPLAY_STEP;
            displayYaw ();
        }

        if ( g_ulSampCnt > g_UART_timer ) {
            g_UART_timer += UART_TIMER_STEP;
            UARTmessage ();
        }

        // Sets the SysTick delay at approximately 50 Hz polling
        SysCtlDelay ( SysCtlClockGet () / COUNT );
    }
}
