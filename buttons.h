/* @file    buttons.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    15/05/2018
 * @brief   Header file for buttons ...
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
// Defined constants
//*****************************************************************************

// Enumerations
enum butNames { UP = 0, DOWN, LEFT, RIGHT, SWITCH, SWITCH_2, NUM_BUTS };
enum butStates { RELEASED = 0, PUSHED, NO_CHANGE };

// UP button
#define UP_BUT_PERIPH           SYSCTL_PERIPH_GPIOE
#define UP_BUT_PORT_BASE        GPIO_PORTE_BASE
#define UP_BUT_PIN              GPIO_PIN_0
#define UP_BUT_NORMAL           false

// DOWN button
#define DOWN_BUT_PERIPH         SYSCTL_PERIPH_GPIOD
#define DOWN_BUT_PORT_BASE      GPIO_PORTD_BASE
#define DOWN_BUT_PIN            GPIO_PIN_2
#define DOWN_BUT_NORMAL         false

// LEFT button
#define LEFT_BUT_PERIPH         SYSCTL_PERIPH_GPIOF
#define LEFT_BUT_PORT_BASE      GPIO_PORTF_BASE
#define LEFT_BUT_PIN            GPIO_PIN_4
#define LEFT_BUT_NORMAL         true

// RIGHT button
#define RIGHT_BUT_PERIPH        SYSCTL_PERIPH_GPIOF
#define RIGHT_BUT_PORT_BASE     GPIO_PORTF_BASE
#define RIGHT_BUT_PIN           GPIO_PIN_0
#define RIGHT_BUT_NORMAL        true

// Switch one (MODE)
#define SWITCH_ONE_PERIPH       SYSCTL_PERIPH_GPIOA
#define SWITCH_ONE_PORT_BASE    GPIO_PORTA_BASE
#define SWITCH_ONE_PIN          GPIO_PIN_7
#define SWITCH_ONE_NORMAL       false

// Switch one (MODE)
#define SWITCH_TWO_PERIPH       SYSCTL_PERIPH_GPIOA
#define SWITCH_TWO_PORT_BASE    GPIO_PORTA_BASE
#define SWITCH_TWO_PIN          GPIO_PIN_6
#define SWITCH_TWO_NORMAL       true

//*****************************************************************************
// Function declarations
//*****************************************************************************

// Copies a string '*src' to string '*dest'
char *strcpy ( char *dest, const char *src );

// Compares two strings and returns <, ==, > 0; == 0 is equal strings
int strcmp ( const char *str1, const char *str2 );

/**
 * Initializes the variables associated with the set of buttons
 */
void
initButtons ( void );

/**
 * Lands the helicopter at a controlled rate when switch turned down
 */
void
controlled_landing ( void );

/**
 * Checks the state of the buttons and does something with the information
 */
void
checkButState ( void );

/**
 * Polls all buttons once and updates variables associated with buttons if necessary
 */
void
updateButtons ( void );

/**
 * Returns the new button logical state if the button logical state
 * (PUSHED or RELEASED) has changed since the last call
 * @param The name of the button
 * @return State of the button
 */
uint8_t
checkButton ( uint8_t butName );

/**
 * Handles all of the states
 */
void
stateHandler ( void );

#endif /* BUTTONS_H_ */
