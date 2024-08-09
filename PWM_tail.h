/* @file    PWM_tail.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    25/05/2018
 * @brief   Heade file for tail rotor motor PWM
 */

#ifndef PWM_TAIL_H_
#define PWM_TAIL_H_

/**********************************************************
 * Defined constants
 **********************************************************/

//  PWM Hardware Details M0PWM5
//  ---Tail Rotor PWM: PF5, J3-10
#define PWM_TAIL_BASE           PWM1_BASE
#define PWM_TAIL_GEN            PWM_GEN_2
#define PWM_TAIL_OUTNUM         PWM_OUT_5
#define PWM_TAIL_OUTBIT         PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM     SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO    SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE      GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG    GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN       GPIO_PIN_1

/**
 * Sets the PWM to a percentage (%) for the tail rotor motor
 * @param pwm: The PWM value
 */
void
setPWMtail ( int pwm );

/**
 * Returns the PWM duty cycle as a percentage (%) for the tail rotor motor
 */
int
getPWMtail ( void );

/**
 * Update to LED display the PWM duty cycle as a percentage (%) for tail rotor
 */
void
updateDisplayPWMtail ( void );

/**
 * Initializes the PWM for the tail rotor motor
 */
void
initPWMtail ( void );

#endif /* PWM_TAIL_H_ */
