/* @file    UART.h
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/05/2018
 * @brief   Header file for UART
 */

#ifndef UART_H_
#define UART_H_

/**
 * Initializes the UART
 */
void
initUART ( void );

/**
 * sends the UART
 */
void
sendUART ( char *stringBuffer );

#endif /* UART_H_ */
