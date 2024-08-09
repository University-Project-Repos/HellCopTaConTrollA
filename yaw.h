/* @file    yaw.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/04/2018
 * @brief   Header file for helicopter yaw
 */

#ifndef YAW_H_
#define YAW_H_

/**
 * Initialize the yaw interrupt ports
 */
void
initYaw ( void );

/**
 * Display the helicopter rigs current yaw
 */
void
displayYaw ( void );

/**
 * Returns the current yaw in degrees
 * @return The current yaw in degrees
 */
int
getYaw ( void );

#endif /* YAW_H_ */
