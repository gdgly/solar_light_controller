
#include "oled.h"
#include "oledfont.h"
#include "i2c.h"
#include "app_debug.h"

#define OLED_ADDR   0x78

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


void OLED_WR_Byte(unsigned char dat, unsigned cmd)
{
    uint8_t buf[2];
    if(cmd) {
        buf[0] = 0x40;
        buf[1] = dat;
    }else{
        buf[0] = 0x00;
        buf[1] = dat;
    }
    
    HAL_StatusTypeDef i2c_ret;
    if((i2c_ret = HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, 50)) != HAL_OK) {
        APP_ERROR("i2c[%02x] transmit failed: %d \r\n", OLED_ADDR, i2c_ret);
    }
}

static void delay_ms(uint32_t i)
{
    i *= 18000;
    while(i--);
}

void OLED_init(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

    delay_ms(200);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
//    delay_ms(200);

    OLED_WR_Byte(0xAE,OLED_CMD);//--display off
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  
    OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
    OLED_WR_Byte(0x81,OLED_CMD); // contract control
    OLED_WR_Byte(0xFF,OLED_CMD);//--128   
    OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap 
    OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
    OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00,OLED_CMD);//

    OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
    OLED_WR_Byte(0x80,OLED_CMD);//

    OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
    OLED_WR_Byte(0x05,OLED_CMD);//

    OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
    OLED_WR_Byte(0xF1,OLED_CMD);//

    OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
    OLED_WR_Byte(0x12,OLED_CMD);//

    OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
    OLED_WR_Byte(0x30,OLED_CMD);//

    OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
    OLED_WR_Byte(0x14,OLED_CMD);//

    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}

//坐标设置
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{
    OLED_WR_Byte(0xb0+y, OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10, OLED_CMD);
    OLED_WR_Byte((x&0x0f), OLED_CMD); 
}

//开启OLED显示  
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

//关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}

//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void)  
{
    u8 i,n;
    for(i=0;i<8;i++) {
        OLED_WR_Byte(0xb0+i, OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);      //设置显示位置—列高地址   
        for(n=0;n<128;n++) {
            OLED_WR_Byte(0, OLED_DATA);
        }
    } //更新显示
}

void OLED_On(void)  
{  
    u8 i, n;
    for(i=0;i<8;i++) {
        OLED_WR_Byte(0xb0+i, OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte(0x00, OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte(0x10, OLED_CMD);      //设置显示位置—列高地址   
        for(n=0;n<128;n++) {
            OLED_WR_Byte(1, OLED_DATA);
        }
    } //更新显示
}

#define Max_Column  128
#define Max_Row     64

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12 
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 Char_Size)
{
    unsigned char c=0,i=0;
    c=chr-' ';//得到偏移后的值
    if(x>Max_Column-1) {
        x=0;
        y=y+2;
    }
    if(Char_Size ==16) {
        OLED_Set_Pos(x, y);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        OLED_Set_Pos(x, y+1);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    } else {
        OLED_Set_Pos(x, y);
        for(i=0;i<6;i++) {
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);
        }
    }
}

void OLED_ShowString(u8 x, u8 y, char *chr, u8 Char_Size)
{
    unsigned char j=0;
    while (chr[j]!='\0') {
        OLED_ShowChar(x, y, (uint8_t)chr[j], Char_Size);
        x+=8;
        if(x>120) {
            x=0;
            y+=2;
        }
        j++;
    }
}

int OLED_ShowStringFill(u8 x, u8 y, const char *chr, u8 Char_Size)
{
    unsigned char j;
    for(j=0;x<=120;j++) {
        if(j < strlen(chr)) {
            OLED_ShowChar(x, y, (uint8_t)chr[j], Char_Size);
        } else {
            OLED_ShowChar(x, y, (uint8_t)' ', Char_Size);
        }
        x += 8;
    }
    return 0;
}

void OLED_ShowSignal(uint8_t x, uint8_t y, uint8_t level)
{
    uint8_t i, local = 3;
    if(level > 5) {
        return;
    }
    level =  BmpSignal[0] - (5-level)*4;
    
    OLED_Set_Pos(x, y);
    for(i=0; i < BmpSignal[0]; i++) {
        if(i < level)
            OLED_WR_Byte(~BmpSignal[local], OLED_DATA);
        else
            OLED_WR_Byte(0x00, OLED_DATA);
        local++;
    }
    OLED_Set_Pos(x, y+1);
    for(i=0; i < BmpSignal[0]; i++) {
        if(i < level)
            OLED_WR_Byte(~(BmpSignal[local] | 0x80), OLED_DATA);
        else
            OLED_WR_Byte(0x00, OLED_DATA);
        local++;
    }
}

