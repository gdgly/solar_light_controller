/* 2019 04 10 */
/* By hxdyxd */

#include <stdio.h>
#include "usart.h"
#include "kfifo.h"


static uint8_t uart_debug_fifo[128];
static int uart_debug_counter = 0;


void direct_uart_puts(int ch)
{
    while(!__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE)) {
         //Transmit data register empty flag
    }
    huart2.Instance->TDR = (char)ch;
}


int fputc(int ch, FILE *f)
{
    if(uart_debug_counter < sizeof(uart_debug_fifo)) {
        uart_debug_fifo[uart_debug_counter++] = ch;
    }
    
    if(ch == '\n') {
        HAL_UART_Transmit(&huart2, uart_debug_fifo, uart_debug_counter, 20);
        uart_debug_counter = 0;
    }
    return ch;
}


int fgetc(FILE *f)
{
    return 1;
}



/*****************************END OF FILE***************************/
