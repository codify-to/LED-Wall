/* 
 * File:   uart.h
 * Author: lucasdupin
 *
 * Created on June 16, 2013, 11:51 PM
 */

#ifndef UART_H
#define	UART_H

#include "constants.h"

void uart_init(void)
{

    U1MODEbits.ON=0;
    U1MODEbits.SIDL=1;
    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, PBFCY, BRG);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

}
void SendDataBuffer(const char *buffer, unsigned int size)
{
    while(size)
    {
        while(!UARTTransmitterIsReady(UART1))
            ;

        UARTSendDataByte(UART1, *buffer);

        buffer++;
        size--;
    }

    while(!UARTTransmissionHasCompleted(UART1))
        ;
}

#endif	/* UART_H */

