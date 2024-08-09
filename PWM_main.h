/* @file    PWM_main.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    25/05/2018
 * @brief   Header file for PWM_main
 */

#ifndef PWM_MAIN_H_
#define PWM_MAIN_H_

/**********************************************************
 * Defined constants
 **********************************************************/

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE           PWM0_BASE
#define PWM_MAIN_GEN            PWM_GEN_3
#define PWM_MAIN_OUTNUM         PWM_OUT_7
#define PWM_MAIN_OUTBIT         PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM     SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO    SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE      GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG    GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN       GPIO_PIN_5

/**
 * Sets the PWM to a percentage (%) for the main rotor motor
 * @param pwm   PWM value
 */
void
setPWMmain ( int pwm );

/**
 * Returns the PWM duty cycle as a percentage (%) for the main rotor motor
 */
int
getPWMmain ( void );

/**
 * Update to LED display the PWM duty cycle as a percentage (%) for main rotor
 */
void
updateDisplayPWMmain ( void );

/**
 * Initializes the PWM for the main rotor motor
 */
void
initPWMmain ( void );

#endif /* PWM_MAIN_H_ */
