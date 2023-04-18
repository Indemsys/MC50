/* generated configuration header file - do not edit */
#ifndef BSP_MCU_FAMILY_CFG_H_
  #define BSP_MCU_FAMILY_CFG_H_
  #include "bsp_mcu_device_pn_cfg.h"
  #include "bsp_mcu_device_cfg.h"
  #include "../../../synergy/ssp/src/bsp/mcu/s5d9/bsp_mcu_info.h"
  #include "bsp_clock_cfg.h"
  #define BSP_MCU_GROUP_S5D9 (1)
  #define BSP_LOCO_HZ                   (32768)
  #define BSP_MOCO_HZ                   (8000000)
  #define BSP_SUB_CLOCK_HZ              (32768)
  #if   BSP_CFG_HOCO_FREQUENCY == 0
    #define BSP_HOCO_HZ                 (16000000)
  #elif BSP_CFG_HOCO_FREQUENCY == 1
    #define BSP_HOCO_HZ                 (18000000)
  #elif BSP_CFG_HOCO_FREQUENCY == 2
    #define BSP_HOCO_HZ                 (20000000)
  #else
    #error "Invalid HOCO frequency chosen (BSP_CFG_HOCO_FREQUENCY) in bsp_clock_cfg.h"
  #endif

  #define BSP_CORTEX_VECTOR_TABLE_ENTRIES       (16U)
  #define BSP_VECTOR_TABLE_MAX_ENTRIES          (112U)
  #define BSP_MCU_VBATT_SUPPORT                 (1)

//  #define OFS_SEQ1                              0xA001A001  | (1 << 1)  | (3 << 2)
//  #define OFS_SEQ2                              (15 << 4)   | (3 << 8)  | (3 << 10)
//  #define OFS_SEQ3                              (1 << 12)   | (1 << 14) | (1 << 17)
//  #define OFS_SEQ4                              (3 << 18)   |(15 << 20) | (3 << 24) | (3 << 26)
//  #define OFS_SEQ5                              (1 << 28)   | (1 << 30)
//  #define BSP_CFG_ROM_REG_OFS0                  (OFS_SEQ1   | OFS_SEQ2  | OFS_SEQ3  | OFS_SEQ4 | OFS_SEQ5)

  #define     LSFT(v,n) (v << n)

// Иницализация двух вотчдогов. Один WDT другой  IWDT.
// WDT   тактируется от PCLKB (60Мгц)
// IWDT  тактируется от независимого таймера 15 Кгц
// WDT   не запускаем
// IWDT  запускаем на время 8.7 сек, счетчик остановливается в режимах c пониженным потреблением

  #define  B1_31  1  //  Reserved      | The write value should be 1.
  #define  B1_30  1  //  WDTSTPCTL     | WDT Stop Control                          | 0: Continue counting, 1: Stop counting when entering Sleep mode
  #define  B1_29  1  //  Reserved      | The write value should be 1.
  #define  B1_28  1  //  WDTRSTIRQS    | WDT Reset Interrupt Request select        | 0: NMI , 1: Reset
  #define  B1_26  3  //  WDTRPSS[1:0]  | WDT Window Start Position select          | 0 0: 25%, 0 1: 50%, 1 0: 75%, 1 1: 100% (No window start position setting)
  #define  B1_24  3  //  WDTRPES[1:0]  | WDT Window End Position select            | 0 0: 75%, 0 1: 50%, 1 0: 25%, 1 1: 0% (No window end position setting)
  #define  B1_20  15 //  WDTCKS[3:0]   | WDT Clock Frequency Division Ratio Select | 0 0 0 1: PCLKB divided by 4, 0 1 0 0 - 64, 1 1 1 1 - 128, 0 1 1 0 - 512, 0 1 1 1 - 2048, 1 0 0 0- 8192.
  #define  B1_18  3  //  WDTTOPS[1:0]  | WDT Timeout Period Select                 | 0 0: 1024 cycles (03FFh), 0 1: 4096 cycles (0FFFh), 1 0: 8192 cycles (1FFFh), 1 1: 16384 cycles (3FFFh).
  #define  B1_17  1  //  WDTSTRT       | WDT Start Mode Select                     | 0: Automatically activate WDT after a reset, 1: Stop WDT after a reset (register-start mode).
  #define  B1_16  1  //  Reserved      | The write value should be 1.
  #define  B1_15  1  //  Reserved      | The write value should be 1.
  #define  B1_14  1  //  IWDTSTPCTL    | IWDT Stop Control                         | 0: Continue counting, 1: Stop counting when in Sleep, Snooze mode, or Software Standby mode.
  #define  B1_13  1  //  Reserved      | The write value should be 1.
  #define  B1_12  1  //  IWDTRSTIRQS   | IWDT Reset Interrupt Request Select       | 0: Enable non-maskable interrupt requests or interrupt requests, 1: Enable resets
  #define  B1_10  3  //  IWDTRPSS[1:0] | IWDT Window Start Position select         | 0 0: 25%, 0 1: 50%, 1 0: 75%, 1 1: 100% (No window start position setting)
  #define  B1__8  3  //  IWDTRPES[1:0] | IWDT Window End Position select           | 0 0: 75%, 0 1: 50%, 1 0: 25%, 1 1: 0% (No window end position setting)
  #define  B1__4  5  //  IWDTCKS[3:0]  | IWDT Dedicated Clock Frequency Division Ratio  | 0 0 0 0: 1/1, 0 0 1 0: 1/16, 0 0 1 1: 1/32, 0 1 0 0: 1/64, 1 1 1 1: 1/128, 0 1 0 1: 1/256
  #define  B1__2  1  //  IWDTTOPS[1:0] | IWDT Timeout Period Select                | 0 0: 128 cycles (007Fh), 0 1: 512 cycles (01FFh), 1 0: 1024 cycles (03FFh), 1 1: 2048 cycles (07FFh)

// Вотчдог включен
//#define  B1__1  0  //  IWDTSTRT      | IWDT Start Mode Select                    | 0: Automatically activate IWDT after a reset , 1: Disable IWDT

// Вотчдог выключен
  #define  B1__1  1  //  IWDTSTRT      | IWDT Start Mode Select                    | 0: Automatically activate IWDT after a reset , 1: Disable IWDT

  #define  B1__0  1  //  Reserved      | The write value should be 1.

  #define BSP_CFG_ROM_REG_OFS0  (0\
                         | LSFT(B1_31, 31)\
                         | LSFT(B1_30, 30)\
                         | LSFT(B1_29, 29)\
                         | LSFT(B1_28, 28)\
                         | LSFT(B1_26, 26)\
                         | LSFT(B1_24, 24)\
                         | LSFT(B1_20, 20)\
                         | LSFT(B1_18, 18)\
                         | LSFT(B1_17, 17)\
                         | LSFT(B1_16, 16)\
                         | LSFT(B1_15, 15)\
                         | LSFT(B1_14, 14)\
                         | LSFT(B1_13, 13)\
                         | LSFT(B1_12, 12)\
                         | LSFT(B1_10, 10)\
                         | LSFT(B1__8,  8)\
                         | LSFT(B1__4,  4)\
                         | LSFT(B1__2,  2)\
                         | LSFT(B1__1,  1)\
                         | LSFT(B1__0,  0))

// HOCO осцилятор и детектор напряжения выключены.
//
  #define  B2_31  1   //  Reserved      | The write value should be 1.
  #define  B2_30  1   //  Reserved      | The write value should be 1.
  #define  B2_29  1   //  Reserved      | The write value should be 1.
  #define  B2_28  1   //  Reserved      | The write value should be 1.
  #define  B2_27  1   //  Reserved      | The write value should be 1.
  #define  B2_26  1   //  Reserved      | The write value should be 1.
  #define  B2_25  1   //  Reserved      | The write value should be 1.
  #define  B2_24  1   //  Reserved      | The write value should be 1.
  #define  B2_23  1   //  Reserved      | The write value should be 1.
  #define  B2_22  1   //  Reserved      | The write value should be 1.
  #define  B2_21  1   //  Reserved      | The write value should be 1.
  #define  B2_20  1   //  Reserved      | The write value should be 1.
  #define  B2_19  1   //  Reserved      | The write value should be 1.
  #define  B2_18  1   //  Reserved      | The write value should be 1.
  #define  B2_17  1   //  Reserved      | The write value should be 1.
  #define  B2_16  1   //  Reserved      | The write value should be 1.
  #define  B2_15  1   //  Reserved      | The write value should be 1.
  #define  B2_14  1   //  Reserved      | The write value should be 1.
  #define  B2_13  1   //  Reserved      | The write value should be 1.
  #define  B2_12  1   //  Reserved      | The write value should be 1.
  #define  B2_11  1   //  Reserved      | The write value should be 1.
  #define  B2__9  3   //  HOCOFRQ0[1:0] | HOCO Frequency Setting 0          | 0 0: 16 MHz, 0 1: 18 MHz, 1 0: 20 MHz, 1 1: Setting prohibited
  #define  B2__8  1   //  HOCOEN        | HOCO Oscillation Enable           | 0: Enable HOCO oscillation after a reset, 1: Disable HOCO oscillation after a reset.
  #define  B2__7  1   //  Reserved      | The write value should be 1.
  #define  B2__6  1   //  Reserved      | The write value should be 1.
  #define  B2__5  1   //  Reserved      | The write value should be 1.
  #define  B2__4  1   //  Reserved      | The write value should be 1.
  #define  B2__3  1   //  Reserved      | The write value should be 1.
  #define  B2__2  0   //  LVDAS         | Voltage Detection 0 Circuit Start | 0: Enable voltage monitor 0 reset after a reset, 1: Disable voltage monitor 0 reset after a reset.
  #define  B2__0  3   //  VDSEL0[1:0]   | Voltage Detection 0 Level Select  | 0 0: Setting prohibited, 0 1: Select 2.94 V, 1 0: Select 2.87 V, 1 1: Select 2.80 V.

// Предыдущие настройки
//#define BSP_CFG_ROM_REG_OFS1 (0xFFFFFEF8 | (1 << 2) | (3) |  (1 << 8))

  #define BSP_CFG_ROM_REG_OFS1  (0\
                         | LSFT(B2_31, 31)\
                         | LSFT(B2_30, 30)\
                         | LSFT(B2_29, 29)\
                         | LSFT(B2_28, 28)\
                         | LSFT(B2_27, 27)\
                         | LSFT(B2_26, 26)\
                         | LSFT(B2_25, 25)\
                         | LSFT(B2_24, 24)\
                         | LSFT(B2_23, 23)\
                         | LSFT(B2_22, 22)\
                         | LSFT(B2_21, 21)\
                         | LSFT(B2_20, 20)\
                         | LSFT(B2_19, 19)\
                         | LSFT(B2_18, 18)\
                         | LSFT(B2_17, 17)\
                         | LSFT(B2_16, 16)\
                         | LSFT(B2_15, 15)\
                         | LSFT(B2_14, 14)\
                         | LSFT(B2_13, 13)\
                         | LSFT(B2_12, 12)\
                         | LSFT(B2_11, 11)\
                         | LSFT(B2__9,  9)\
                         | LSFT(B2__8,  8)\
                         | LSFT(B2__7,  7)\
                         | LSFT(B2__6,  6)\
                         | LSFT(B2__5,  5)\
                         | LSFT(B2__4,  4)\
                         | LSFT(B2__3,  3)\
                         | LSFT(B2__2,  2)\
                         | LSFT(B2__0,  0))


  #define BSP_CFG_ROM_REG_MPU_PC0_ENABLE        (1)
  #define BSP_CFG_ROM_REG_MPU_PC0_START         (0xFFFFFFFC)
  #define BSP_CFG_ROM_REG_MPU_PC0_END           (0xFFFFFFFF)
  #define BSP_CFG_ROM_REG_MPU_PC1_ENABLE        (1)
  #define BSP_CFG_ROM_REG_MPU_PC1_START         (0xFFFFFFFC)
  #define BSP_CFG_ROM_REG_MPU_PC1_END           (0xFFFFFFFF)
  #define BSP_CFG_ROM_REG_MPU_REGION0_ENABLE    (1)
  #define BSP_CFG_ROM_REG_MPU_REGION0_START     (0x00FFFFFC)
  #define BSP_CFG_ROM_REG_MPU_REGION0_END       (0x00FFFFFF)
  #define BSP_CFG_ROM_REG_MPU_REGION1_ENABLE    (1)
  #define BSP_CFG_ROM_REG_MPU_REGION1_START     (0x200FFFFC)
  #define BSP_CFG_ROM_REG_MPU_REGION1_END       (0x200FFFFF)
  #define BSP_CFG_ROM_REG_MPU_REGION2_ENABLE    (1)
  #define BSP_CFG_ROM_REG_MPU_REGION2_START     (0x407FFFFC)
  #define BSP_CFG_ROM_REG_MPU_REGION2_END       (0x407FFFFF)
  #define BSP_CFG_ROM_REG_MPU_REGION3_ENABLE    (1)
  #define BSP_CFG_ROM_REG_MPU_REGION3_START     (0x400DFFFC)
  #define BSP_CFG_ROM_REG_MPU_REGION3_END       (0x400DFFFF)
#endif /* BSP_MCU_FAMILY_CFG_H_ */
