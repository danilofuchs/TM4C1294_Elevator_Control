#ifndef __UART_H_
#define __UART_H_

#define UART_TX_BUFFER_SIZE 1024
#define UART_BAUD_RATE 115200

void UARTInit(void);
void UART0_Handler(void);

#endif