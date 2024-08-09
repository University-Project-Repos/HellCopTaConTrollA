/* @file    height.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/04/2018
 * @brief   Header file for height (helicopter altitude)
 */

#ifndef HEIGHT_H_
#define HEIGHT_H_

/**
 * Initializes the ADC for the height sensor
 */
void
initADC ( void );

/**
 * Returns the ADC count converted into millivolts
 * @return  The ADC value in millivolts
 */
int
getADCVolt ( void );

/**
 * Sets the ground height
 */
void
ADCheightReference ( void );

/**
 * Calculates height as a percentage for displaying to Tiva LED
 * @return  The altitude as a percentage
 */
int16_t
getAltitudePercentage ( void );

/**
 * Displays the height of the helicopter as a percentage
 */
void
displayHeight ( void );

/**
 * Returns current height of ground
 * @return  The ground height
 */
uint32_t
getGround ( void );

#endif /* HEIGHT_H_ */
