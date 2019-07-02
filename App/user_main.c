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


static float f_led_power_set = 0;
static float f_bat_power_set = 1.0;

static int16_t s16_cur_duty_max = 0;
static int16_t s16_cur_duty_min = 9999;

static int ctrl_counter = 0;
static uint8_t u8_mode_set = MODE_BAT_PID;
static uint8_t u8_mode_fault = 0;

static pidc_t pid_buck = {
    .kp = 33,  //current
    .ki = 0,
    .kd = 0.3,
};

static pidc_t pid_boost = {
    .kp = 100,  //current
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
    "MPPT充电",
    "放电",
    "充电",
};

static char *string_mode_gbk[MODE_NUM] = {
    "MPPT\xb3\xe4\xb5\xe7",
    "\xb7\xc5\xb5\xe7",
    "\xb3\xe4\xb5\xe7",
};




void led_proc(void)
{
    LED_REV(LED_PV_BASE);
    LED_REV(LED_BAT_BASE);
    
    SWITCH_TASK_INIT(task1);
    
    SWITCH_TASK(task1){
        printf("\r\n\r\n");
        printf("--------------------------------[");
        if(u8_mode_set == MODE_LED_OUT) {
            printf(GREEN_FONT, string_mode[u8_mode_set]);
        } else if(u8_mode_set == MODE_BAT_PID) {
            printf(RED_FONT, string_mode[u8_mode_set]);
        }
        printf("]--------------------------------");
        printf("\r\n");
    }
    
    SWITCH_TASK(task1){
        static uint32_t last_time = 0;
        APP_DEBUG(
            "%s, ctl= %d cnt/s, duty recent 1s= (%4d)%.1f%%-(%4d)%.1f%% \r\n",
            string_mode[u8_mode_set],
            ctrl_counter*1000/(TIMER_TASK_GET_TICK_COUNT() - last_time),
            s16_cur_duty_min,
            s16_cur_duty_min*100.0/PWM_GET_DUTY_MAX(PWM_CH1),
            s16_cur_duty_max,
            s16_cur_duty_max*100.0/PWM_GET_DUTY_MAX(PWM_CH1) 
        );
        last_time = TIMER_TASK_GET_TICK_COUNT();
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
    
    SWITCH_TASK(task1){
        printf("$TEST,%.3f,%.3f,%.3f,%.3f,%.3f,\r\n", 
            GET_PV_VOLTAGE(), GET_BAT_VOLTAGE(), GET_LED_VOLTAGE(), GET_BAT_CURRENT(), GET_LED_CURRENT()
        );
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft0.txt=\"%.3fV\"\xff\xff\xff\r\n", GET_PV_VOLTAGE());
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft1.txt=\"%.3fV\"\xff\xff\xff\r\n", GET_BAT_VOLTAGE());
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft2.txt=\"%.3fV\"\xff\xff\xff\r\n", GET_LED_VOLTAGE());
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft3.txt=\"%.3fA\"\xff\xff\xff\r\n", GET_BAT_CURRENT());
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft4.txt=\"%.3fA\"\xff\xff\xff\r\n", GET_LED_CURRENT());
    }
    
    SWITCH_TASK(task1){
        printf("\xff\xff\xfft5.txt=\"%s\"\xff\xff\xff\r\n", string_mode_gbk[u8_mode_set]);
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



#define BUCK_MAX_OUTPUT_DUTY    (MAX_OUTPUT_DUTY-50)
#define BUCK_MIN_OUTPUT_DUTY    (200)

void pid_buck_control_proc(void)
{
    float vvvvv = GET_BAT_CURRENT();
    int16_t ctrl_duty = pid_ctrl(&pid_buck, vvvvv );
    
    if(GET_PV_VOLTAGE() < 5) {
        ctrl_duty = pid_buck.output = BUCK_MAX_OUTPUT_DUTY;
    }
    
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
    
    if(GET_BAT_VOLTAGE() < 5) {
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
            //简单去极值滤波算法
            uint16_t adc_value = No_Max_Min_Filter(u16pbuf, ADC_CONV_NUMBER, ADC_CHANNEL_NUMBER, i);
            //计算电压值
            adc_voltage_array[i] = adc_value*3.3/4095;
            //计算校准电压值
            adc_real_voltage_array[i] = get_param_value(adc_voltage_array[i], i);
            //计算实际电压或电流值
            adc_real_value_array[i] = value_adc_adjustment(adc_real_voltage_array[i], i);
        }
        
        /**********************************[mode ctrl]***********************************/
        u8_mode_fault = 0;
        //滞回比较器
        if(GET_PV_VOLTAGE() > GET_BAT_VOLTAGE() ) {
            //太阳能板电压高于电池电压，进入充电模式
            if(u8_mode_set != MODE_BAT_PID) {
                pid_buck.output = BUCK_MAX_OUTPUT_DUTY;
                u8_mode_set = MODE_BAT_PID; //Charging mode
            }
        } else if(GET_PV_VOLTAGE() < 5) {
            //太阳能板电压低于光照下限，进入放电模式
            if(u8_mode_set != MODE_LED_OUT) {
                pid_boost.output = BOOST_MIN_OUTPUT_DUTY;
                u8_mode_set = MODE_LED_OUT; //Discharge, LED On
            }
        } else {
            //电池满，光照足，异常
            //电池空，光照弱，异常
            u8_mode_fault = 1;
        }
        /**********************************[mode ctrl]***********************************/
        
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


void uart_proc(uint8_t *sbuf, uint8_t len)
{
    static uint8_t flag = 0;
    printf("rx %d, cmd[%.*s]\r\n", len, len, sbuf);
    sbuf[len] = 0;
    float f_set_led_current = 0;
    
    if(sscanf( (char *)sbuf, "POWER:%f", &f_set_led_current) != 1) {
        APP_ERROR("cmd parse error\r\n");
        return;
    }
    
    f_set_led_current /= 30;
    
    if(f_set_led_current <= 1.0 && f_set_led_current >= 0.0) {
        APP_DEBUG("set current = %.3fA\r\n", f_set_led_current);
        pid_set_value(&pid_boost, f_set_led_current);
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
    //降压上限防止高管长导通，下限防止电池短路
    pid_set_output_limit(&pid_buck, BUCK_MAX_OUTPUT_DUTY, BUCK_MIN_OUTPUT_DUTY);
    //升压上限防止电池短路，下限防止高管长导通
    pid_set_output_limit(&pid_boost, BOOST_MAX_OUTPUT_DUTY, BOOST_MIN_OUTPUT_DUTY);
    
    pid_set_value(&pid_buck, f_bat_power_set);
    pid_set_value(&pid_boost, f_led_power_set);
}

void user_loop(void)
{
    TIMER_TASK(led_task, 180, 1) { led_proc(); }
    adc_rx_proc(adc_receive_proc);
    usart_rx_proc(uart_proc);
}


/*****************************END OF FILE***************************/
