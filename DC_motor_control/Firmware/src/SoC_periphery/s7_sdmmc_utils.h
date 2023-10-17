#ifndef MC50_SDMMC_UTILS_H
  #define MC50_SDMMC_UTILS_H


  #define SD_SET_PASSWORD            1
  #define SD_UNLOCK                  2
  #define SD_CLEAR_PASSWORD          3

  #define CARD_NORMAL_SPEED          ((uint32_t)0x00000000U)   // Normal Speed Card <12.5Mo/s , Spec Version 1.01
  #define CARD_HIGH_SPEED            ((uint32_t)0x00000100U)   // High Speed Card <25Mo/s , Spec version 2.00
  #define CARD_ULTRA_HIGH_SPEED      ((uint32_t)0x00000200U)   // UHS-I SD Card <50Mo/s for SDR50, DDR5 Cards and <104Mo/s for SDR104, Spec version 3.01

  #define CARD_SDSC                  ((uint32_t)0x00000000U)   // SD Standard Capacity <2Go
  #define CARD_SDHC_SDXC             ((uint32_t)0x00000001U)   // SD High Capacity <32Go, SD Extended Capacity <2To
  #define CARD_SECURED               ((uint32_t)0x00000003U)



typedef struct
{
    uint8_t  ManufacturerID;      // Manufacturer ID
    uint16_t OEM_AppliID;         // OEM/Application ID
    uint32_t ProdName1;           // Product Name part1
    uint8_t  ProdName2;           // Product Name part2
    uint8_t  ProdRev;             // Product Revision
    uint32_t ProdSN;              // Product Serial Number
    uint8_t  Reserved1;           // Reserved1
    uint16_t ManufactDate;        // Manufacturing Date
    uint8_t  CID_CRC;             // CID CRC
    uint8_t  Reserved2;           // Always 1

} T_sdcard_cid;

typedef struct
{
    uint8_t  CSDStruct;            // CSD structure
    uint8_t  SysSpecVersion;       // System specification version
    uint8_t  Reserved1;            // Reserved
    uint8_t  TAAC;                 // Data read access time 1
    uint8_t  NSAC;                 // Data read access time 2 in CLK cycles
    uint8_t  MaxBusClkFrec;        // Max. bus clock frequency
    uint16_t CardComdClasses;      // Card command classes
    uint8_t  RdBlockLen;           // Max. read data block length
    uint8_t  PartBlockRead;        // Partial blocks for read allowed
    uint8_t  WrBlockMisalign;      // Write block misalignment
    uint8_t  RdBlockMisalign;      // Read block misalignment
    uint8_t  DSRImpl;              // DSR implemented
    uint8_t  Reserved2;            // Reserved
    uint32_t DeviceSize;           // Device Size
    uint8_t  MaxRdCurrentVDDMin;   // Max. read current @ VDD min
    uint8_t  MaxRdCurrentVDDMax;   // Max. read current @ VDD max
    uint8_t  MaxWrCurrentVDDMin;   // Max. write current @ VDD min
    uint8_t  MaxWrCurrentVDDMax;   // Max. write current @ VDD max
    uint8_t  DeviceSizeMul;        // Device size multiplier
    uint8_t  EraseGrSize;          // Erase group size
    uint8_t  EraseGrMul;           // Erase group size multiplier
    uint8_t  WrProtectGrSize;      // Write protect group size
    uint8_t  WrProtectGrEnable;    // Write protect group enable
    uint8_t  ManDeflECC;           // Manufacturer default ECC
    uint8_t  WrSpeedFact;          // Write speed factor
    uint8_t  MaxWrBlockLen;        // Max. write data block length
    uint8_t  WriteBlockPaPartial;  // Partial blocks for write allowed
    uint8_t  Reserved3;            // Reserved
    uint8_t  ContentProtectAppli;  // Content protection application
    uint8_t  FileFormatGroup;      // File format group
    uint8_t  CopyFlag;             // Copy flag (OTP)
    uint8_t  PermWrProtect;        // Permanent write protection
    uint8_t  TempWrProtect;        // Temporary write protection
    uint8_t  File_Format;          // File format
    uint8_t  ECC;                  // ECC code
    uint8_t  CSD_CRC;              // CSD CRC
    uint8_t  Reserved4;            // Always 1
} T_sdcard_csd;

typedef struct
{
    uint32_t CardType;                  // Specifies the card Type
    uint32_t CardVersion;               // Specifies the card version
    uint32_t Class;                     // Specifies the class of the card class
    uint32_t RelCardAdd;                // Specifies the Relative Card Address
    uint32_t BlockNbr;                  // Specifies the Card Capacity in blocks
    uint32_t BlockSize;                 // Specifies one block size in bytes
    uint32_t LogBlockNbr;               // Specifies the Card logical Capacity in blocks
    uint32_t LogBlockSize;              // Specifies logical block size in bytes
    uint32_t CardSpeed;                 // Specifies the card Speed

} T_sdcard;


uint32_t   SD_card_open(void);
uint32_t   SD_card_close(void);
uint32_t   SD_get_password(char **buf, uint8_t *len);
uint8_t    SD_fill_lock_struct_with_password(T_Lock_Card_Data_Structure *p_lcds);
uint32_t   SD_card_pre_init(void);
uint32_t   SD_get_card_locked_satus(uint8_t *p_lock_stat);
uint32_t   SD_password_operations(uint8_t operation, char *pass_buff, uint8_t pass_len);

#endif



