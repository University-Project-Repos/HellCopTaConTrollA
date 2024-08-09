/* @file    buttons.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/04/2018
 * @brief   Buttons file
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "./OrbitOLED/OrbitOLEDInterface.h"
#include "inc/tm4c123gh6pm.h"       // Board specific defines (for PF0)
#include "buttons.h"
#include "PWM_main.h"
#include "PWM_tail.h"
#include "height.h"
#include "yaw.h"
#include "UART.h"

//*****************************************************************************
// Defined constants
//*****************************************************************************

#define NUM_BUT_POLLS           3   // The number of button polls
#define YAW_STEP                15  // The yaw degrees per yaw change
#define ALT_STEP                10  // The altitude percent per altitude change
#define CEILING_HEIGHT          100 // The ceiling (max) altitude percentage
#define FLOOR                   0   // The floor (min) altitude percentage
#define TRUE                    1   // 1 for true as C doesn't use true/false
#define FALSE                   0   // 0 for false as C doesn't use true/false
#define ERROR                   2   // Error margin for each altitude step
#define INIT_ALT_STEP           10  // Init altitude step from ground mit error
#define INIT_MAIN_DTY_CYCL      15  // Init for the main rotor motor duty cycle
#define G_STATE_SIZE            20

// *******************************************************
// Globals to module
// *******************************************************

char g_state [ G_STATE_SIZE ] = "landed";

static bool g_but_state [ NUM_BUTS ];    // Corresponds to the electrical g_state
static bool g_but_flag [ NUM_BUTS ];
static bool g_but_normal [ NUM_BUTS ];
_Bool g_flying_from_landed = FALSE;
_Bool g_start_up_sequence = TRUE;
_Bool g_calibrate = FALSE;
_Bool g_landing_sequence = FALSE;
extern _Bool g_ref_found;

static uint8_t g_but_count [ NUM_BUTS ];
int8_t g_altitude_percent = 0;
int16_t g_yaw_angle = 0;
int8_t g_button_state;
extern int g_duty_cycle_main;

/**
 * Initializes the variables associated with the set of buttons
 */
void
initButtons ( void )
{
    // UP button (active HIGH)
    SysCtlPeripheralEnable ( UP_BUT_PERIPH );
    GPIOPinTypeGPIOInput ( UP_BUT_PORT_BASE, UP_BUT_PIN );
    GPIOPadConfigSet ( UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );
    g_but_normal [ UP ] = UP_BUT_NORMAL;

    // DOWN button (active HIGH)
    SysCtlPeripheralEnable ( DOWN_BUT_PERIPH );
    GPIOPinTypeGPIOInput ( DOWN_BUT_PORT_BASE, DOWN_BUT_PIN );
    GPIOPadConfigSet ( DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );
    g_but_normal [ DOWN ] = DOWN_BUT_NORMAL;

    // LEFT button (active LOW)
    SysCtlPeripheralEnable ( LEFT_BUT_PERIPH );
    GPIOPinTypeGPIOInput ( LEFT_BUT_PORT_BASE, LEFT_BUT_PIN );
    GPIOPadConfigSet ( LEFT_BUT_PORT_BASE, LEFT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU );
    g_but_normal [ LEFT ] = LEFT_BUT_NORMAL;

    // RIGHT button (active LOW)
    SysCtlPeripheralEnable ( RIGHT_BUT_PERIPH );
    //---Unlock PF0 for the right button:
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0; // PF0 unlocked
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;
    GPIOPinTypeGPIOInput ( RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN );
    GPIOPadConfigSet ( RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU );
    g_but_normal [ RIGHT ] = RIGHT_BUT_NORMAL;

    // Switch one (MODE)
    SysCtlPeripheralEnable ( SWITCH_ONE_PERIPH );
    GPIOPinTypeGPIOInput ( SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN );
    GPIOPadConfigSet( SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD );
    g_but_normal [ SWITCH ] = SWITCH_ONE_NORMAL;
    int i;

    for ( i = 0; i < NUM_BUTS; i++ )
    {
        g_but_state [ i ] = g_but_normal [ i ];
        g_but_count [ i ] = 0;
        g_but_flag [ i ] = false;
    }
}

/**
 * Handles all of the states
 */
void
stateHandler ( void )
{
    if ( g_flying_from_landed )
    {
        if ( !g_start_up_sequence )
        {
            g_button_state = checkButton ( SWITCH );

            switch ( g_button_state )
            {
            case PUSHED:
                strcpy ( g_state, "Flying" );
                break;
            case RELEASED:
                strcpy ( g_state, "landing" );
                g_landing_sequence = TRUE;
                break;
            }
        } else
        {
            g_button_state = checkButton ( SWITCH );

            switch ( g_button_state )
            {
            case PUSHED:
                g_calibrate = TRUE;
                break;
            }
        }
    } else
    {
        g_button_state = checkButton ( SWITCH );

        switch ( g_button_state )
        {
        case RELEASED:
            g_flying_from_landed = TRUE;
            break;
        }
    }

    if ( g_landing_sequence )
    {
        if ( getAltitudePercentage () < ( INIT_ALT_STEP - ERROR ))
        {
            g_landing_sequence = FALSE;
            g_altitude_percent = 0;
            strcpy ( g_state, "landed" );
        }
        else
        {
            g_yaw_angle = 0;
            g_altitude_percent = INIT_ALT_STEP;
            g_duty_cycle_main = INIT_MAIN_DTY_CYCL;
        }
    }

    if ( g_calibrate )
    {
        strcpy ( g_state, "Calibration" );
        g_altitude_percent = INIT_ALT_STEP * 2;   // Sets the PWM for the tail to on

        if ( g_ref_found )
        {
            g_altitude_percent = 0;
            g_start_up_sequence = FALSE;
            g_calibrate = FALSE;
            g_yaw_angle = 0;
            sendUART ( "Yaw angle reset here" );
            strcpy ( g_state, "Flying" );
        } else
        {
            if (( getYaw () < ( g_yaw_angle + ERROR )) && ( getYaw () ) >  ( g_yaw_angle - ERROR ))
            {
                g_yaw_angle -= YAW_STEP;
            }
        }
    }
}

/**
 * Checks the state of the buttons and does something with the information
 */
void
checkButState ( void )
{
    updateButtons ();

    if ( strcmp ( g_state, "Flying" ) == 0 )
    {
        g_button_state = checkButton ( UP );

        switch ( g_button_state )
        {
        case PUSHED:
            if ( g_altitude_percent < CEILING_HEIGHT )
            {
                g_altitude_percent += ALT_STEP;
            }
            break;
        case RELEASED:
           break;
        }
        g_button_state = checkButton ( DOWN );

        switch ( g_button_state )
        {
        case PUSHED:
            if ( g_altitude_percent > FLOOR )
            {
                g_altitude_percent -= ALT_STEP;
            }
            break;
        case RELEASED:
            break;
        }
        g_button_state = checkButton ( LEFT );

        switch ( g_button_state )
        {
        case PUSHED:
            g_yaw_angle -= YAW_STEP;
            break;
        case RELEASED:
            break;
        }
        g_button_state = checkButton ( RIGHT );

        switch ( g_button_state )
        {
        case PUSHED:
            g_yaw_angle += YAW_STEP;
            break;
        case RELEASED:
            break;
        }
    }
}

/**
 * Polls all buttons once and updates variables associated with buttons if necessary
 */
void
updateButtons ( void )
{
    bool but_value [ NUM_BUTS ];

    // Read the pins; true means HIGH, false means LOW
    but_value [ UP ] = ( GPIOPinRead ( UP_BUT_PORT_BASE, UP_BUT_PIN ) == UP_BUT_PIN );
    but_value [ DOWN ] = ( GPIOPinRead ( DOWN_BUT_PORT_BASE, DOWN_BUT_PIN ) == DOWN_BUT_PIN );
    but_value [ LEFT ] = ( GPIOPinRead ( LEFT_BUT_PORT_BASE, LEFT_BUT_PIN ) == LEFT_BUT_PIN );
    but_value [ RIGHT ] = ( GPIOPinRead ( RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN ) == RIGHT_BUT_PIN );
    but_value [ SWITCH ] = ( GPIOPinRead ( SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN ) == SWITCH_ONE_PIN );
    but_value [ SWITCH_2 ] = ( GPIOPinRead ( SWITCH_TWO_PORT_BASE, SWITCH_TWO_PIN ) == SWITCH_TWO_PIN );
    int i;

    // Iterate through the buttons, updating button variables as required
    for ( i = 0; i < NUM_BUTS; i++ )
    {
        if ( but_value [ i ] != g_but_state [ i ] )
        {
            g_but_count [ i ]++;

            if ( g_but_count [ i ] >= NUM_BUT_POLLS )
            {
                g_but_state [ i ] = but_value [ i ];
                g_but_flag [ i ] = true;
                g_but_count [ i ] = 0;
            }
        }
        else
            g_but_count [ i ] = 0;
    }
}

/**
 * Returns the new button logical state if the button logical state
 * (PUSHED or RELEASED) has changed since the last call
 * @param but_name  The name of the button
 * @return          The state of the button
 */
uint8_t
checkButton ( uint8_t but_name )
{
    if ( g_but_flag [ but_name ] )
    {
        g_but_flag [ but_name ] = false;

        if ( g_but_state [ but_name ] == g_but_normal [ but_name ] )
            return RELEASED;
        else
            return PUSHED;
    }
    return NO_CHANGE;
}
