/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#define MODE_MPPT        0
#define MODE_LED_OUT     1
#define MODE_BAT_PID     2

#define MODE_NUM         3 


#define GET_PV_VOLTAGE()         adc_real_value_array[1]
#define GET_BAT_VOLTAGE()        adc_real_value_array[2]
#define GET_LED_VOLTAGE()        adc_real_value_array[5]

#define GET_BAT_CURRENT()        adc_real_value_array[0]
#define GET_LED_CURRENT()        adc_real_value_array[4]


void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
