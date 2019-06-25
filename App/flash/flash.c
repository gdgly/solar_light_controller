#include "flash.h"


int stm32_flash_write(FLASH_DAT_TYPE_DEF *data)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;
    data->mask = FLASH_WORD_MASK;
    
    HAL_FLASH_Unlock();//先解锁
    
    uint32_t Address = FLASH_USER_START_ADDR;
    uint32_t *userAddress = (uint32_t *)data;
    
    while (Address < FLASH_USER_END_ADDR) {
        if( ((FLASH_DAT_TYPE_DEF *)Address)->mask == 0xffffffff ) {
            
            for(int i=0; i<FLASH_WORD_NUM; i++) {
                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *userAddress) == HAL_OK) {
                    Address += 4;
                    userAddress += 1;
                } else {
                    /* 上锁 */
                    HAL_FLASH_Lock();
                    return -1;
                }
            }
            /* 上锁 */
            HAL_FLASH_Lock();
            return 0;
        }
        
        Address += 4*FLASH_WORD_NUM;
    }
    
    printf("[FLASH] Erase start ...\r\n");
    
    /* Erase the user Flash area
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
    EraseInitStruct.NbPages = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        /*
          Error occurred while page erase.
          User can add here some code to deal with this error.
          PageError will contain the faulty page and then to know the code error on this page,
          user can call function 'HAL_FLASH_GetError()'
        */
        /* Infinite loop */
        while (1)
        {
            /* User doing something here */
        }
    }
    
    printf("[FLASH] Erase success ...\r\n");
    
    /* Program the user Flash area word by word
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    Address = FLASH_USER_START_ADDR;
    for(int i=0; i<FLASH_WORD_NUM; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *userAddress) == HAL_OK) {
            Address += 4;
            userAddress += 1;
        } else {
            /* 上锁 */
            HAL_FLASH_Lock();
            return -1;
        }
    }
    
    /* 上锁 */
    HAL_FLASH_Lock();
    return 0;
}

FLASH_DAT_TYPE_DEF *stm32_flash_read(void)
{
    uint32_t Address = FLASH_USER_START_ADDR;
    while(Address < FLASH_USER_END_ADDR) {
        FLASH_DAT_TYPE_DEF *fdata = (FLASH_DAT_TYPE_DEF *)Address;
        if(fdata->mask != FLASH_WORD_MASK) {
            FLASH_DAT_TYPE_DEF *fdata_n = (FLASH_DAT_TYPE_DEF *)(Address - 4*FLASH_WORD_NUM);
            if(fdata_n->mask == FLASH_WORD_MASK) {
                return fdata_n;
            } else {
                return NULL;
            }
        }
        
        Address += 4*FLASH_WORD_NUM;
    }
    return NULL;
}


