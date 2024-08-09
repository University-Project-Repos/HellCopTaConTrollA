/* @file    UART.c
 * @author  Gus Ellerm, Andrew Limmer-Wood, Adam Ross
 * @date    28/05/2018
 * @brief   UART file
 */

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include "yaw.h"
#include "buttons.h"
#include "PWM_main.h"
#include "PWM_tail.h"
#include "UART.h"
#include "height.h"
#include "inc/hw_memmap.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

/**********************************************************
 * Defined constants
 **********************************************************/

#define BAUD_RATE               9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX

/**
 * Initializes the UART
 */
void
initUART ( void )
{
    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable ( UART_USB_PERIPH_UART );
    SysCtlPeripheralEnable ( UART_USB_PERIPH_GPIO );

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART ( UART_USB_GPIO_BASE, UART_USB_GPIO_PINS );
    UARTConfigSetExpClk ( UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
                          UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE );
    UARTFIFOEnable ( UART_USB_BASE );
    UARTEnable ( UART_USB_BASE );
}

/**
 * Sends the UART
 * @param *pucBuffer
 */
void
sendUART ( char *pucBuffer )
{
    // Loop while there are more characters to send.
    while ( *pucBuffer )
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut ( UART0_BASE, *pucBuffer );
        ++pucBuffer;
    }
}
