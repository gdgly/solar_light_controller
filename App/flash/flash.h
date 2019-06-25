#ifndef _FLASH_H_
#define _FLASH_H_

#include "stm32f0xx.h"


//Page 63
#define FLASH_USER_START_ADDR    (0x0800FC00)
#define FLASH_USER_END_ADDR     (FLASH_USER_START_ADDR + FLASH_PAGE_SIZE)

typedef struct {
    uint32_t mask;
    float freq;
}FLASH_DAT_TYPE_DEF;

#define FLASH_WORD_MASK   0x1234cafe
#define FLASH_WORD_NUM      2


int stm32_flash_write(FLASH_DAT_TYPE_DEF *data);
FLASH_DAT_TYPE_DEF *stm32_flash_read(void);


#endif
