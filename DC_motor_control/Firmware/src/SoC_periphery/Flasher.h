#ifndef MC50_FLASHER_H
  #define MC50_FLASHER_H

// Сруктура Code Flash памяти кода
// 0x00000000...0x0000FFFF - стирание по 8192   байта, запись по 256 байт
// 0x00010000...0x003FFFFF - стирание по 32768  байта, запись по 256 байт

#define CODE_FLASH_START      0x00000000
#define CODE_FLASH_END        0x001FFFFF
#define CODE_FLASH_EBLOCK_SZ  32768          // Размер стираемого блока
#define CODE_FLASH_WR_SZ      256            // Размер записываемого блока

// Сруктура Data Flash памяти данных
// 0x40100000...0x4010FFFF - стирание по 64 байта, запись по 4 байта

#define DATA_FLASH_START      0x40100000
#define DATA_FLASH_END        0x4010FFFF
#define DATA_FLASH_EBLOCK_SZ  64          // Размер стираемого блока
#define DATA_FLASH_WR_SZ      4           // Размер записываемого блока



#define   BIT_FLASH_EVENT_ERASE_COMPLETE    BIT(0)
#define   BIT_FLASH_EVENT_WRITE_COMPLETE    BIT(1)
#define   BIT_FLASH_EVENT_BLANK             BIT(2)
#define   BIT_FLASH_EVENT_NOT_BLANK         BIT(3)
#define   BIT_FLASH_EVENT_ERR_DF_ACCESS     BIT(4)
#define   BIT_FLASH_EVENT_ERR_CF_ACCESS     BIT(5)
#define   BIT_FLASH_EVENT_ERR_CMD_LOCKED    BIT(6)
#define   BIT_FLASH_EVENT_ERR_FAILURE       BIT(7)
#define   BIT_FLASH_EVENT_ERR_ONE_BIT       BIT(8)

uint32_t Init_Flash_driver(void);
uint32_t Get_flash_op_status(void);
void     Set_flash_op_status(uint32_t stat);
uint32_t Wait_for_flash_op_end(ULONG wait_option);
uint32_t S7G2_DataFlash_EraseArea(uint32_t start_addr, uint32_t area_size);
uint32_t S7G2_DataFlash_WriteArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size);

#endif // APP_FLASHER_H



