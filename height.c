/* @file    height.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/04/2018
 * @brief   Contains the code relevant to the height calculation
 */

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "circBufT.h"
#include "./OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"

//****************************************************************************
// Defined constants
//****************************************************************************

#define BUF_SIZE                10
#define HELI_ALT_RANGE          800               // 0.8 V range in mV
#define MILLIVOLTS_PER_ADC_STEP 0.7324            // 3 V / 12 bits = 0.73 mV
#define SEQUENCE_NO             3                 // The sequence number
#define Y_POS                   3                 // The Y position on display
#define GROUND_HEIGHT_BUFFER    500000            // Ground height delay buffer
#define MESSAGE_SIZE            24                // Size of message for display

//****************************************************************************
// Global variables
//****************************************************************************

static circBuf_t g_inBuffer;
int32_t g_ground_height;

/**
 * ADC interrupt handler
 */
void
ADCIntHandler ( void )
{
    uint32_t ulValue;

    // Get the single sample from ADC0.  ADC_BASE is defined in inc/hw_memmap.h
    ADCSequenceDataGet ( ADC0_BASE, SEQUENCE_NO, &ulValue );

    // Place it in the circular buffer (advancing write index)
    writeCircBuf ( &g_inBuffer, ulValue );

    // Clean up, clearing the interrupt
    ADCIntClear ( ADC0_BASE, SEQUENCE_NO );
}

/**
 * Initializes the ADC for the height sensor
 */
void
initADC ( void )
{
    // Resets ADC0 peripheral
    SysCtlPeripheralReset ( SYSCTL_PERIPH_ADC0 );

    // Enable PE4 & ADC
    GPIOPinTypeADC ( GPIO_PORTE_BASE, GPIO_PIN_4 );

    // Enable the ADC that we are using
    SysCtlPeripheralEnable ( SYSCTL_PERIPH_ADC0 );

    // Configure ADC to use sequence 3 with processor trigger
    ADCSequenceConfigure ( ADC0_BASE, SEQUENCE_NO, ADC_TRIGGER_PROCESSOR, 0 );

    // Configure step 0 for ADC_CTL_CH0 (should be 9 for heli_rig, 0 is for experimentation)
    ADCSequenceStepConfigure ( ADC0_BASE, SEQUENCE_NO, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END );

    // Enable the sequence that we are using
    ADCSequenceEnable ( ADC0_BASE, SEQUENCE_NO );

    // Register the interrupt
    ADCIntRegister ( ADC0_BASE, SEQUENCE_NO, ADCIntHandler );

    // Clear ADC interrupt flag
    ADCIntClear ( ADC0_BASE, SEQUENCE_NO );

    // Enable the interrupt !! We might want to only enable this interrupt after the base height is set
    ADCIntEnable ( ADC0_BASE, SEQUENCE_NO );
}

/**
 * Calculates the rounded average ADC from buffer for displaying to Tiva LED
 * @return  The average ADC value
 */
uint16_t
getADCvalue ( void )
{
    int32_t sum = 0;
    int i;

    for ( i = 0; i < BUF_SIZE; i++ )
    {
        sum += readCircBuf ( &g_inBuffer );
    }
    return (( 2 * sum + BUF_SIZE ) / 2 / BUF_SIZE );
}

/**
 * Returns the ADC count converted into millivolts
 * @return  The ADC value in millivolts
 */
int
getADCVolt ( void )
{
    return getADCvalue () * MILLIVOLTS_PER_ADC_STEP;
}

/**
 * Sets the ground height
 */
void
ADCheightReference ( void )
{
    initCircBuf ( &g_inBuffer, BUF_SIZE );
    int count = 0;

    while ( count < GROUND_HEIGHT_BUFFER )
    {
        ++count;
    }
    g_ground_height = getADCVolt ();
}

/**
 * Calculates height as a percentage for displaying to Tiva LED
 * @return  The altitude as a percentage
 */
int16_t
getAltitudePercentage ( void )
{
    int ceiling = g_ground_height - HELI_ALT_RANGE;
    return 100 * ( g_ground_height - ( getADCVolt () )) / ( g_ground_height - ceiling );
}

/**
 * Displays the height of the helicopter as a percentage
 */
void
displayHeight ( void )
{
    char message [ MESSAGE_SIZE ];
    usprintf ( message, "Height: %d ",  getAltitudePercentage () );
    OLEDStringDraw ( message, 0, Y_POS );
}

/**
 * Returns current height of ground
 * @return  The ground height
 */
uint32_t
getGround ( void )
{
    return g_ground_height;
}
