/* 
 *
 *2018 09 21 & hxdyxd
 *
 */


#ifndef __data_interface_hal_H__
#define __data_interface_hal_H__

#include <stdint.h>

#include "gpio.h"

/* ADC */
#define ADC_CONV_NUMBER      (50)
#define ADC_CHANNEL_NUMBER   (6)

/* PWM */
#define PWM_GET_DUTY_MAX(ch)  ((ch->ARR)+1)

#define PWM_SET_DUTY(ch ,d)   ch->CCR1 = (d)
#define PWM_GET_DUTY(ch)     (ch->CCR1)

#define PWM_CH1   TIM1

//UART
#define UART_BUFFER_SIZE   (256)

/* LEDS */
#define LED_OFF(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_ON(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_HIGH(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_LOW(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_REV(id)  HAL_GPIO_TogglePin(id)

#define LED_PV_BASE   GPIOB, PB6_LED_PV_Pin
#define LED_BAT_BASE   GPIOB, PB7_LED_BAT_Pin
#define LED_LOAD_BASE   GPIOB, PB8_LED_LOAD_Pin
#define LED_CTRL_BASE   GPIOA, PA12_LED_CTRL_Pin


/*******************************************************************************
* Function Name  : data_interface_hal_init.
* Description    :Hardware adaptation layer initialization.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void data_interface_hal_init(void);

/*******************************************************************************
* Function Name  : adc_rx_proc.
* Description    :Hardware adaptation layer adc process.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void adc_rx_proc( void (*func_cb)(void *, int len) );

/*******************************************************************************
* Function Name  : usart_rx_proc.
* Description    :Hardware adaptation layer adc process.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void usart_rx_proc( void (* usart_rx_callback)(uint8_t *, uint8_t) );



#define hal_read_TickCounter() HAL_GetTick()

void uart2_irq(UART_HandleTypeDef *huart);

#endif
