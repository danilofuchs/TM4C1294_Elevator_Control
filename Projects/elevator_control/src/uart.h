#ifndef __UART_H__
#define __UART_H__

#define UART_TX_BUFFER_SIZE 1024
#define UART_BAUD_RATE 115200

void UARTInit(void);
void UART0_Handler(void);

#endif