/* @file    g_yaw.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/04/2018
 * @brief   Helicopter g_yaw file
 */

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "./OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"
#include "UART.h"

//****************************************************************************
// Defined constants
//****************************************************************************

#define YAW_STEP                1.60714285714      // 360 degrees / (2 * 112)
#define DISPLAY_POS             3                  // The position on display
#define MESSAGE_SIZE            24                 // Size of message for display

//****************************************************************************
// Global variables
//****************************************************************************

volatile float g_yaw = 0;
volatile uint32_t g_pin0_state = 0;
volatile uint32_t g_pin1_state = 0;
volatile int g_state_11 = 0;
volatile int g_state_00 = 0;

_Bool g_ref_found = 0;

/**
 * Interrupt handler for the reference g_yaw
 */
void
refIntHandler ( void )
{
    uint32_t int_pin = GPIOIntStatus ( GPIO_PORTC_BASE, false );

    if ( int_pin & GPIO_PIN_4 )
    {
        GPIOIntClear ( GPIO_PORTC_BASE, GPIO_PIN_4 );

        if ( !g_ref_found )
        {
            g_yaw = 0;
            g_ref_found = 1;
        }
    }
}

/**
 * Yaw interrupt handler
 */
void
yawIntHandler ( void )
{
    uint32_t int_pin = GPIOIntStatus ( GPIO_PORTB_BASE, false );

    // If an interrupt is triggered from PB0 pin
    if ( int_pin & GPIO_PIN_0 )
    {
        // Clear the interrupt so it can run again
        GPIOIntClear ( GPIO_PORTB_BASE, GPIO_PIN_0 );

        // Read the pin to get its state
        g_pin0_state = GPIOPinRead ( GPIO_PORTB_BASE, GPIO_PIN_0 );

        if ( g_state_11 )
        {
            g_state_11 = 0;

            if ( g_pin1_state )
            {
                g_yaw -= YAW_STEP;
            }
        }

        if ( g_state_00 )
        {
            g_state_00 = 0;

            if ( g_pin0_state )
            {
                g_yaw -= YAW_STEP;
            }
        }
    }

    // If an interrupt is triggered from PB1 pin
    if ( int_pin & GPIO_PIN_1 )
    {
        GPIOIntClear ( GPIO_PORTB_BASE, GPIO_PIN_1 );
        g_pin1_state = GPIOPinRead ( GPIO_PORTB_BASE, GPIO_PIN_1 );

        if ( g_state_11 )
        {
            g_state_11 = 0;

            if ( g_pin0_state )
            {
                g_yaw += YAW_STEP;
            }
        }

        if ( g_state_00 )
        {
            g_state_00 = 0;

            if ( g_pin1_state )
            {
                g_yaw += YAW_STEP;
            }
        }
    }

    if ( g_pin0_state && g_pin1_state )
    {
        g_state_11 = 1;
    }

    if ( !g_pin0_state && !g_pin1_state )
    {
        g_state_00 = 1;
    }
}

/**
 * Initialize the yaw interrupt ports
 */
void
initYaw ( void )
{
    GPIOIntRegister ( GPIO_PORTB_BASE, yawIntHandler );
    GPIOIntRegister ( GPIO_PORTC_BASE, refIntHandler );

    // Initialize pins on port B
    GPIOPinTypeGPIOInput ( GPIO_PORTB_BASE, GPIO_PIN_0 );

    GPIOPadConfigSet ( GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );

    // Pin change interrupt, both edges
    GPIOIntTypeSet ( GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES );

    // Enable the pin change interrupt
    GPIOIntEnable ( GPIO_PORTB_BASE, GPIO_PIN_0 );

    // Second pin
    GPIOPinTypeGPIOInput ( GPIO_PORTB_BASE, GPIO_PIN_1 );

    GPIOPadConfigSet ( GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );

    // Pin change interrupt, both edges
    GPIOIntTypeSet ( GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_BOTH_EDGES );

    // Enable the pin change interrupt
    GPIOIntEnable ( GPIO_PORTB_BASE, GPIO_PIN_1 );

    // Initialize pins on port C
    GPIOPinTypeGPIOInput ( GPIO_PORTC_BASE, GPIO_PIN_4 );

    GPIOPadConfigSet ( GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );

    // Pin change interrupt, both edges
    GPIOIntTypeSet ( GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_BOTH_EDGES );

    // Enable the pin change interrupt
    GPIOIntEnable ( GPIO_PORTC_BASE, GPIO_PIN_4 );

    // Enable the interrupts
    IntEnable ( INT_GPIOB );
    IntEnable ( INT_GPIOC );
}

/**
 * Display the helicopter rigs current yaw
 */
void
displayYaw ( void )
{
    char message [ MESSAGE_SIZE ];
    usprintf ( message, "yaw: %4d ", ( int ) g_yaw );
    OLEDStringDraw ( message, 0, DISPLAY_POS );
}

/**
 * Returns the current yaw in degrees
 * @return The current yaw in degrees
 */
int
getYaw ( void )
{
    return g_yaw;
}
