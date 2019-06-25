/* 2019 04 10 */
/* By hxdyxd */
#include <stdlib.h>
#include "app_debug.h"
#include "user_main.h"
#include "adc_algorithm.h"
#include "function_task.h"


#define MAX_OUTPUT_DUTY    (PWM_GET_DUTY_MAX(PWM_CH1) - 10)



static float adc_voltage_array[ADC_CHANNEL_NUMBER + 1] = {0};
static float adc_real_voltage_array[ADC_CHANNEL_NUMBER + 1] = {0};
static float adc_real_value_array[ADC_CHANNEL_NUMBER + 1] = {0};


static float f_led_power_set = 1.0;

static int16_t s16_cur_duty_max = 0;
static int16_t s16_cur_duty_min = 9999;

static int ctrl_counter = 0;
static uint8_t u8_mode_set = MODE_LED_OUT;

static pidc_t pid_buck = {
    .kp = 10,
    .ki = 0,
    .kd = 1,
};

static pidc_t pid_boost = {
    .kp = 100,
    .ki = 0,
    .kd = 1,
};



static char *string_adc_ch[ADC_CHANNEL_NUMBER] = {
    "PA0_BAT_I_ADC",  //PA0_BAT_I_ADC
    "PA1_PV_ADC   ",     //PA1_PV_ADC
    "PA4_BAT_ADC  ",     //PA4_BAT_ADC
    "PA5_NTC_ADC  ",      //PA5_NTC_ADC
    "PA6_LED_I_ADC",  //PA6_LED_I_ADC
    "PB0_LED_ADC  ",     //PB0_LED_ADC
};

static char *string_tail_adc_ch[ADC_CHANNEL_NUMBER] = {
    "A",  //PA0_BAT_I_ADC
    "V",     //PA1_PV_ADC
    "V",     //PA4_BAT_ADC
    " ",      //PA5_NTC_ADC
    "A",  //PA6_LED_I_ADC
    "V",     //PB0_LED_ADC
};

static char *string_mode[MODE_NUM] = {
    "MODE_MPPT",
    "MODE_LED_OUT",
    "MODE_BAT_PID",
};



void led_proc(void)
{
    LED_REV(LED_PV_BASE);
    LED_REV(LED_BAT_BASE);
    
    SWITCH_TASK_INIT(task1);
    
    SWITCH_TASK(task1){
        printf("\r\n");
    }
    
    SWITCH_TASK(task1){
        APP_DEBUG(
            "%s, ctl= %d cnt/s, duty recent 1s= (%4d)%.1f%%-(%4d)%.1f%% \r\n",
            string_mode[u8_mode_set],
            ctrl_counter,
            s16_cur_duty_min,
            s16_cur_duty_min*100.0/PWM_GET_DUTY_MAX(PWM_CH1),
            s16_cur_duty_max,
            s16_cur_duty_max*100.0/PWM_GET_DUTY_MAX(PWM_CH1) 
        );
        ctrl_counter = 0;
        s16_cur_duty_max = 0;
        s16_cur_duty_min = MAX_OUTPUT_DUTY;
    }
    
    SWITCH_TASK(task1){
        APP_DEBUG(
            "[BAT] %.2fV * %.2fA = %.2fW (s:%.2f)\r\n",
            GET_BAT_VOLTAGE(),
            GET_BAT_CURRENT(),
            GET_BAT_VOLTAGE()*GET_BAT_CURRENT(),
            pid_buck.setval
        );
    }
    
    SWITCH_TASK(task1){
        APP_DEBUG(
            "[LED] %.2fV * %.2fA = %.2fW (s:%.2f)\r\n",
            GET_LED_VOLTAGE(),
            GET_LED_CURRENT(),
            GET_LED_VOLTAGE()*GET_LED_CURRENT(),
            pid_boost.setval
        );
    }
    
    SWITCH_TASK(task1){
        printf("--------------------------------[ADC]--------------------------\r\n");
    }
    
    SWITCH_TASK(task1){
        
        SWITCH_TASK_DO(task1);
        
        //printf("task1ss %d taskfor %d \r\n", task1ss, task1for);
        APP_DEBUG(
            "%s %02d [ (%.3fV) real: %.3fV adj: %.3f%s ]\r\n",
            string_adc_ch[task1for],
            task1for,
            adc_voltage_array[task1for], 
            adc_real_voltage_array[task1for],
            adc_real_value_array[task1for],
            string_tail_adc_ch[task1for]
        );
        
        SWITCH_TASK_WHILE(task1, ADC_CHANNEL_NUMBER);
    }
    
    SWITCH_TASK(task1){
        printf("--------------------------------[ADC]--------------------------\r\n");
    }
    
    SWITCH_TASK_END(task1);
}


#define  MPPY_STEP_SIZE         (1)

void mppt_control_proc(void)
{
    static uint8_t last_status = 1;
    
    static float max_bat_voltage = 0;
    static float last_bat_voltage = 0;
    static float bat_voltage = 0;
    static int16_t ctrl_duty = 0;
    static int16_t max_ctrl_duty = 0;
    static uint32_t count = 50;
    
    static int16_t shake_head  = 140;
    static int16_t shake_tail  = 200;
    static char shake_range = 30;
    
    
    bat_voltage = GET_BAT_VOLTAGE()*GET_BAT_CURRENT();
    
    if(count == 50)    //开始扫描前设定扫描范围
    {
        count++;    //count自加一，使进入扫描
        max_bat_voltage = 0;
        if(ctrl_duty <= 200)
        {
            ctrl_duty = 200;
        }
        
        shake_head = ctrl_duty - shake_range;
        shake_tail = ctrl_duty + shake_range;
        printf("%d - %d\r\n", shake_head,  shake_tail);
    }
    
    
    if(count == 51) {
        if(bat_voltage > max_bat_voltage)    //记录最大值
        {
            max_bat_voltage = bat_voltage;
            max_ctrl_duty = ctrl_duty;
        }
        
        if(shake_head == shake_tail)    //判断是否扫描完成
        {
            ctrl_duty = max_ctrl_duty;
            count = 0;
        } else {
            shake_head++;
            ctrl_duty = shake_head;
        }
        
    } else {
        count++;
        
        if(last_status) {
            last_status = (bat_voltage  - last_bat_voltage > 0);
        } else {
            last_status = (bat_voltage  - last_bat_voltage < 0);
        }
        last_bat_voltage = bat_voltage;
        
        ctrl_duty += last_status ? MPPY_STEP_SIZE : -MPPY_STEP_SIZE;
    }
    
    
    if(ctrl_duty > MAX_OUTPUT_DUTY) {
        ctrl_duty = MAX_OUTPUT_DUTY;
    }
    if(ctrl_duty < 0) {
        ctrl_duty = 0;
    }
    PWM_SET_DUTY(PWM_CH1, ctrl_duty);
    //debug 1s duty value
    if(ctrl_duty > s16_cur_duty_max)
        s16_cur_duty_max = ctrl_duty;
    if(ctrl_duty < s16_cur_duty_min)
        s16_cur_duty_min = ctrl_duty;
}



void pid_buck_control_proc(void)
{
    int16_t ctrl_duty = pid_ctrl(&pid_buck, GET_BAT_VOLTAGE()*GET_BAT_CURRENT() );
    PWM_SET_DUTY(PWM_CH1, ctrl_duty);
    
     //debug 1s duty value
    if(ctrl_duty > s16_cur_duty_max)
        s16_cur_duty_max = ctrl_duty;
    if(ctrl_duty < s16_cur_duty_min)
        s16_cur_duty_min = ctrl_duty;
}


#define BOOST_MAX_OUTPUT_DUTY    (MAX_OUTPUT_DUTY-300)
#define BOOST_MIN_OUTPUT_DUTY    (10)

void pid_boost_control_proc(void)
{
    float vvvvv = GET_LED_CURRENT();
    int16_t ctrl_duty = pid_ctrl(&pid_boost, vvvvv );
    
    if(GET_BAT_VOLTAGE() < 3) {
        ctrl_duty = pid_boost.output = BOOST_MIN_OUTPUT_DUTY;
    }
    //printf("%d %.2f \r\n", ctrl_duty, vvvvv);

    PWM_SET_DUTY(PWM_CH1, PWM_GET_DUTY_MAX(PWM_CH1) - ctrl_duty);
     //debug 1s duty value
    if(ctrl_duty > s16_cur_duty_max)
        s16_cur_duty_max = ctrl_duty;
    if(ctrl_duty < s16_cur_duty_min)
        s16_cur_duty_min = ctrl_duty;
}



void adc_receive_proc(void *pbuf, int len)
{
    TIMER_TASK(adc_task, 3, 1) {
        uint16_t *u16pbuf = (uint16_t *)pbuf;
        
        for(int i=0; i<ADC_CHANNEL_NUMBER; i++) {
            //简单滤波算法
            uint16_t adc_value = No_Max_Min_Filter(u16pbuf, ADC_CONV_NUMBER, ADC_CHANNEL_NUMBER, i);
            //计算电压值
            adc_voltage_array[i] = adc_value*3.3/4095;
            //计算校准电压值
            adc_real_voltage_array[i] = get_param_value(adc_voltage_array[i], i);
            //计算实际电压或电流值
            adc_real_value_array[i] = value_adc_adjustment(adc_real_voltage_array[i], i);
        }
        
        switch(u8_mode_set) {
        case MODE_MPPT:
            LED_HIGH(LED_CTRL_BASE);
            mppt_control_proc();
            break;
        case MODE_LED_OUT:
            LED_LOW(LED_CTRL_BASE);
            pid_boost_control_proc();
            break;
        case MODE_BAT_PID:
            LED_HIGH(LED_CTRL_BASE);
            pid_buck_control_proc();
            break;
        default:
            LED_HIGH(LED_CTRL_BASE);
            break;
        }
        ctrl_counter++;
        LED_REV(LED_LOAD_BASE);
    }
}


void user_system_setup(void)
{
}

void user_setup(void)
{
    PRINTF("\r\n\r\n[LIGHT] Build , %s %s \r\n", __DATE__, __TIME__);
    
    //Initial hardware
    data_interface_hal_init();
    
    param_default_value_init();
    
    //pid controller
    pid_set_output_limit(&pid_buck, MAX_OUTPUT_DUTY, 0);
    pid_set_output_limit(&pid_boost, BOOST_MAX_OUTPUT_DUTY, BOOST_MIN_OUTPUT_DUTY);
    
    pid_set_value(&pid_buck, 15);
    pid_set_value(&pid_boost, f_led_power_set);
}

void user_loop(void)
{
    TIMER_TASK(led_task, 1000.0/7, 1) { led_proc(); }
    adc_rx_proc(adc_receive_proc);
}


/*****************************END OF FILE***************************/
