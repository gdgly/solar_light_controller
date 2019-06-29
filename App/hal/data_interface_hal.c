/* 
 *
 *2018 09 21 & hxdyxd
 *
 */

#include <stdio.h>
#include "data_interface_hal.h"

#include "tim.h"

#include "adc.h"
#include "usart.h"


#define TIM_CHANNEL_1N                      (0x0002U)
#define TIM_CHANNEL_2N                      (0x0006U)





uint16_t adc_dma_buffer[ADC_CONV_NUMBER*ADC_CHANNEL_NUMBER];
uint8_t adc_ok = 0;

uint8_t usart_buffer[UART_BUFFER_SIZE + 1];
volatile uint32_t usart_rx_timer = 0;
volatile uint16_t usart_rx_counter = 0;
volatile uint8_t USART_RX_FLAG = 0;

#define USART_RX_TIMEOUT_MS  (10)

/* some low level platform function */
/* public hal function */


void data_interface_hal_init(void)
{
    //tim
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1N);
    //HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    //HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2N);
    //set ccr
    PWM_SET_DUTY(PWM_CH1, PWM_GET_DUTY_MAX(PWM_CH1)/2);
    
    HAL_ADC_Start_DMA(&hadc, (void *)adc_dma_buffer, ADC_CONV_NUMBER*ADC_CHANNEL_NUMBER);
    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_1);
    
    usart_rx_counter = 0;
    HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
}


void usart_rx_proc( void (* usart_rx_callback)(uint8_t *, uint8_t) )
{
    if(usart_rx_counter != 0 && (hal_read_TickCounter() - usart_rx_timer) > USART_RX_TIMEOUT_MS ) {
        //timeout detect
        USART_RX_FLAG = 1;
        usart_rx_callback(usart_buffer, usart_rx_counter);
        usart_rx_counter = 0;
        USART_RX_FLAG = 0;
        HAL_UART_AbortReceive_IT(&huart2);
        HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
        //APP_WARN("RXD\r\n");
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if( USART_RX_FLAG == 0 ) {
        if(usart_rx_counter != 0 && (hal_read_TickCounter() - usart_rx_timer) > USART_RX_TIMEOUT_MS ) {
            //timeout detect
            USART_RX_FLAG = 1;
        } else if(usart_rx_counter >= UART_BUFFER_SIZE) {
            //usart_rx_counter = 0;
            usart_rx_counter = 0;
            HAL_UART_AbortReceive_IT(&huart2);
            HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
        } else {
            usart_rx_counter++;
            HAL_UART_Receive_IT(huart, usart_buffer+usart_rx_counter, 1);
            usart_rx_timer = hal_read_TickCounter();
        }
    }
}


void adc_rx_proc( void (*func_cb)(void *, int len) )
{
    if(adc_ok) {
        func_cb(adc_dma_buffer, sizeof(adc_dma_buffer));
        adc_ok = 0;
        HAL_ADC_Start_DMA(&hadc, (void *)adc_dma_buffer, ADC_CONV_NUMBER*ADC_CHANNEL_NUMBER);
    }
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    adc_ok = 1;
    HAL_ADC_Stop_DMA(hadc);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    printf("---------------HAL_ADC_ErrorCallback----------\n");
    adc_ok = 1;
}

/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
