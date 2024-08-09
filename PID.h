/* @file    PID.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/05/2018
 * @brief   Header file for PID
 */

#ifndef PID_H_
#define PID_H_

//*****************************************************************************
// Function declarations
//*****************************************************************************

// Compares two strings and returns <, ==, > 0; == 0 is equal strings
int strcmp ( const char *str1, const char *str2 );

/**
 * This function ensures that the PWM outputs are disabled when the
 * helicopter is landed, and on when the helicopter is flying/calibrating
 */
void
PWMtoggle ( void );

/**
 * Controls helicopter height by altering the duty cycle of main PWM output
 */
void
mainControl ( void );

/**
 * Controls helicopter yaw by altering the duty cycle of the tail PWM output
 */
void
tailControl ( void );

/**
 * Displays PID related information to a terminal for debugging purposes
 */
void
UARTmessage ( void );

#endif /* PID_H_ */
