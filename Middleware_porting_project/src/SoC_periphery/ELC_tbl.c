// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.06
// 8:31:00
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


const T_ELC_tbl_rec ELC_tbl[]=
{
  //                                           num    name                          ip                  chan   signal                                     unit    en_nvic en_dtc en_dmac  c_sn     c_ssb    c_dssb  Description
  {  ELC_EVENT_ICU_IRQ0                    ,     1,    "ICU_IRQ0                 ",  SSP_IP_ICU         , 0   , SSP_SIGNAL_ICU_IRQ0                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ1                    ,     2,    "ICU_IRQ1                 ",  SSP_IP_ICU         , 1   , SSP_SIGNAL_ICU_IRQ1                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ2                    ,     3,    "ICU_IRQ2                 ",  SSP_IP_ICU         , 2   , SSP_SIGNAL_ICU_IRQ2                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ3                    ,     4,    "ICU_IRQ3                 ",  SSP_IP_ICU         , 3   , SSP_SIGNAL_ICU_IRQ3                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ4                    ,     5,    "ICU_IRQ4                 ",  SSP_IP_ICU         , 4   , SSP_SIGNAL_ICU_IRQ4                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ5                    ,     6,    "ICU_IRQ5                 ",  SSP_IP_ICU         , 5   , SSP_SIGNAL_ICU_IRQ5                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ6                    ,     7,    "ICU_IRQ6                 ",  SSP_IP_ICU         , 6   , SSP_SIGNAL_ICU_IRQ6                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ7                    ,     8,    "ICU_IRQ7                 ",  SSP_IP_ICU         , 7   , SSP_SIGNAL_ICU_IRQ7                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ8                    ,     9,    "ICU_IRQ8                 ",  SSP_IP_ICU         , 8   , SSP_SIGNAL_ICU_IRQ8                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ9                    ,    10,    "ICU_IRQ9                 ",  SSP_IP_ICU         , 9   , SSP_SIGNAL_ICU_IRQ9                       , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ10                   ,    11,    "ICU_IRQ10                ",  SSP_IP_ICU         , 10  , SSP_SIGNAL_ICU_IRQ10                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ11                   ,    12,    "ICU_IRQ11                ",  SSP_IP_ICU         , 11  , SSP_SIGNAL_ICU_IRQ11                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ12                   ,    13,    "ICU_IRQ12                ",  SSP_IP_ICU         , 12  , SSP_SIGNAL_ICU_IRQ12                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ13                   ,    14,    "ICU_IRQ13                ",  SSP_IP_ICU         , 13  , SSP_SIGNAL_ICU_IRQ13                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ14                   ,    15,    "ICU_IRQ14                ",  SSP_IP_ICU         , 14  , SSP_SIGNAL_ICU_IRQ14                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_ICU_IRQ15                   ,    16,    "ICU_IRQ15                ",  SSP_IP_ICU         , 15  , SSP_SIGNAL_ICU_IRQ15                      , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_DMAC0_INT                   ,    32,    "DMAC0_INT                ",  SSP_IP_DMAC        , 0   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   "USB FS FIFO 0"  },
  {  ELC_EVENT_DMAC1_INT                   ,    33,    "DMAC1_INT                ",  SSP_IP_DMAC        , 1   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   "USB FS FIFO 1"  },
  {  ELC_EVENT_DMAC2_INT                   ,    34,    "DMAC2_INT                ",  SSP_IP_DMAC        , 2   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   "SD Card transfer"  },
  {  ELC_EVENT_DMAC3_INT                   ,    35,    "DMAC3_INT                ",  SSP_IP_DMAC        , 3   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   "WiFi module transfer"  },
  {  ELC_EVENT_DMAC4_INT                   ,    36,    "DMAC4_INT                ",  SSP_IP_DMAC        , 4   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DMAC5_INT                   ,    37,    "DMAC5_INT                ",  SSP_IP_DMAC        , 5   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DMAC6_INT                   ,    38,    "DMAC6_INT                ",  SSP_IP_DMAC        , 6   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DMAC7_INT                   ,    39,    "DMAC7_INT                ",  SSP_IP_DMAC        , 7   , SSP_SIGNAL_DMAC_INT                       , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DTC_COMPLETE                ,    41,    "DTC_COMPLETE             ",  SSP_IP_DTC         , 0   , SSP_SIGNAL_DTC_COMPLETE                   , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DTC_END                     ,    42,    "DTC_END                  ",  SSP_IP_DTC         , 0   , SSP_SIGNAL_DTC_END                        , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_ICU_SNOOZE_CANCEL           ,    45,    "ICU_SNOOZE_CANCEL        ",  SSP_IP_ICU         , 0   , SSP_SIGNAL_ICU_SNOOZE_CANCEL              , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_FCU_FIFERR                  ,    48,    "FCU_FIFERR               ",  SSP_IP_FCU         , 0   , SSP_SIGNAL_FCU_FIFERR                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_FCU_FRDYI                   ,    49,    "FCU_FRDYI                ",  SSP_IP_FCU         , 0   , SSP_SIGNAL_FCU_FRDYI                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_LVD_LVD1                    ,    56,    "LVD_LVD1                 ",  SSP_IP_LVD         , 1   , SSP_SIGNAL_LVD_LVD1                       , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_LVD_LVD2                    ,    57,    "LVD_LVD2                 ",  SSP_IP_LVD         , 2   , SSP_SIGNAL_LVD_LVD2                       , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_CGC_MOSC_STOP               ,    59,    "CGC_MOSC_STOP            ",  SSP_IP_CGC         , 0   , SSP_SIGNAL_CGC_MOSC_STOP                  , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_LPM_SNOOZE_REQUEST          ,    60,    "LPM_SNOOZE_REQUEST       ",  SSP_IP_LPM         , 0   , SSP_SIGNAL_LPM_SNOOZE_REQUEST             , 0    ,  0   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_AGT0_INT                    ,    64,    "AGT0_INT                 ",  SSP_IP_AGT         , 0   , SSP_SIGNAL_AGT_INT                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_AGT0_COMPARE_A              ,    65,    "AGT0_COMPARE_A           ",  SSP_IP_AGT         , 0   , SSP_SIGNAL_AGT_COMPARE_A                  , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_AGT0_COMPARE_B              ,    66,    "AGT0_COMPARE_B           ",  SSP_IP_AGT         , 0   , SSP_SIGNAL_AGT_COMPARE_B                  , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_AGT1_INT                    ,    67,    "AGT1_INT                 ",  SSP_IP_AGT         , 1   , SSP_SIGNAL_AGT_INT                        , 0    ,  1   ,   1  ,    1  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_AGT1_COMPARE_A              ,    68,    "AGT1_COMPARE_A           ",  SSP_IP_AGT         , 1   , SSP_SIGNAL_AGT_COMPARE_A                  , 0    ,  1   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_AGT1_COMPARE_B              ,    69,    "AGT1_COMPARE_B           ",  SSP_IP_AGT         , 1   , SSP_SIGNAL_AGT_COMPARE_B                  , 0    ,  1   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IWDT_UNDERFLOW              ,    70,    "IWDT_UNDERFLOW           ",  SSP_IP_IWDT        , 0   , SSP_SIGNAL_IWDT_UNDERFLOW                 , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    0   ,   " "  },
  {  ELC_EVENT_WDT_UNDERFLOW               ,    71,    "WDT_UNDERFLOW            ",  SSP_IP_WDT         , 0   , SSP_SIGNAL_WDT_UNDERFLOW                  , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_RTC_ALARM                   ,    72,    "RTC_ALARM                ",  SSP_IP_RTC         , 0   , SSP_SIGNAL_RTC_ALARM                      , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_RTC_PERIOD                  ,    73,    "RTC_PERIOD               ",  SSP_IP_RTC         , 0   , SSP_SIGNAL_RTC_PERIOD                     , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_RTC_CARRY                   ,    74,    "RTC_CARRY                ",  SSP_IP_RTC         , 0   , SSP_SIGNAL_RTC_CARRY                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_SCAN_END               ,    75,    "ADC0_SCAN_END            ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_SCAN_END                   , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_SCAN_END_B             ,    76,    "ADC0_SCAN_END_B          ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_SCAN_END_B                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_WINDOW_A               ,    77,    "ADC0_WINDOW_A            ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_WINDOW_A                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_WINDOW_B               ,    78,    "ADC0_WINDOW_B            ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_WINDOW_B                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_COMPARE_MATCH          ,    79,    "ADC0_COMPARE_MATCH       ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_COMPARE_MATCH              , 0    ,  0   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC0_COMPARE_MISMATCH       ,    80,    "ADC0_COMPARE_MISMATCH    ",  SSP_IP_ADC         , 0   , SSP_SIGNAL_ADC_COMPARE_MISMATCH           , 0    ,  0   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_SCAN_END               ,    81,    "ADC1_SCAN_END            ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_SCAN_END                   , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_SCAN_END_B             ,    82,    "ADC1_SCAN_END_B          ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_SCAN_END_B                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_WINDOW_A               ,    83,    "ADC1_WINDOW_A            ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_WINDOW_A                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_WINDOW_B               ,    84,    "ADC1_WINDOW_B            ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_WINDOW_B                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_COMPARE_MATCH          ,    85,    "ADC1_COMPARE_MATCH       ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_COMPARE_MATCH              , 0    ,  0   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ADC1_COMPARE_MISMATCH       ,    86,    "ADC1_COMPARE_MISMATCH    ",  SSP_IP_ADC         , 1   , SSP_SIGNAL_ADC_COMPARE_MISMATCH           , 0    ,  0   ,   1  ,    1  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS0_INT                ,    87,    "COMP_HS0_INT             ",  SSP_IP_COMP_HS     , 0   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS1_INT                ,    88,    "COMP_HS1_INT             ",  SSP_IP_COMP_HS     , 1   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS2_INT                ,    89,    "COMP_HS2_INT             ",  SSP_IP_COMP_HS     , 2   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS3_INT                ,    90,    "COMP_HS3_INT             ",  SSP_IP_COMP_HS     , 3   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS4_INT                ,    91,    "COMP_HS4_INT             ",  SSP_IP_COMP_HS     , 4   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_COMP_HS5_INT                ,    92,    "COMP_HS5_INT             ",  SSP_IP_COMP_HS     , 5   , SSP_SIGNAL_COMP_HS_INT                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_USBFS_FIFO_0                ,    95,    "USBFS_FIFO_0             ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_FIFO_0                     , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_USBFS_FIFO_1                ,    96,    "USBFS_FIFO_1             ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_FIFO_1                     , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_USBFS_INT                   ,    97,    "USBFS_INT                ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_INT                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "USB device VCOM ports"  },
  {  ELC_EVENT_USBFS_RESUME                ,    98,    "USBFS_RESUME             ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_RESUME                     , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    1   ,   " "  },
  {  ELC_EVENT_IIC0_RXI                    ,    99,    "IIC0_RXI                 ",  SSP_IP_IIC         , 0   , SSP_SIGNAL_IIC_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "External I2C RX"  },
  {  ELC_EVENT_IIC0_TXI                    ,   100,    "IIC0_TXI                 ",  SSP_IP_IIC         , 0   , SSP_SIGNAL_IIC_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "External I2C TX"  },
  {  ELC_EVENT_IIC0_TEI                    ,   101,    "IIC0_TEI                 ",  SSP_IP_IIC         , 0   , SSP_SIGNAL_IIC_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "External I2C END"  },
  {  ELC_EVENT_IIC0_ERI                    ,   102,    "IIC0_ERI                 ",  SSP_IP_IIC         , 0   , SSP_SIGNAL_IIC_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "External I2C ERR"  },
  {  ELC_EVENT_IIC0_WUI                    ,   103,    "IIC0_WUI                 ",  SSP_IP_IIC         , 0   , SSP_SIGNAL_IIC_WUI                        , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   "External I2C WUI"  },
  {  ELC_EVENT_IIC1_RXI                    ,   104,    "IIC1_RXI                 ",  SSP_IP_IIC         , 1   , SSP_SIGNAL_IIC_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IIC1_TXI                    ,   105,    "IIC1_TXI                 ",  SSP_IP_IIC         , 1   , SSP_SIGNAL_IIC_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IIC1_TEI                    ,   106,    "IIC1_TEI                 ",  SSP_IP_IIC         , 1   , SSP_SIGNAL_IIC_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IIC1_ERI                    ,   107,    "IIC1_ERI                 ",  SSP_IP_IIC         , 1   , SSP_SIGNAL_IIC_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IIC2_RXI                    ,   109,    "IIC2_RXI                 ",  SSP_IP_IIC         , 2   , SSP_SIGNAL_IIC_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "Internal I2C RX "  },
  {  ELC_EVENT_IIC2_TXI                    ,   110,    "IIC2_TXI                 ",  SSP_IP_IIC         , 2   , SSP_SIGNAL_IIC_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "Internal I2C TX "  },
  {  ELC_EVENT_IIC2_TEI                    ,   111,    "IIC2_TEI                 ",  SSP_IP_IIC         , 2   , SSP_SIGNAL_IIC_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "Internal I2C TEI"  },
  {  ELC_EVENT_IIC2_ERI                    ,   112,    "IIC2_ERI                 ",  SSP_IP_IIC         , 2   , SSP_SIGNAL_IIC_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "Internal I2C ERI"  },
  {  ELC_EVENT_SSI0_TXI                    ,   114,    "SSI0_TXI                 ",  SSP_IP_SSI         , 0   , SSP_SIGNAL_SSI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "DA7217 SSI TX "  },
  {  ELC_EVENT_SSI0_RXI                    ,   115,    "SSI0_RXI                 ",  SSP_IP_SSI         , 0   , SSP_SIGNAL_SSI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "DA7217 SSI RX "  },
  {  ELC_EVENT_SSI0_INT                    ,   117,    "SSI0_INT                 ",  SSP_IP_SSI         , 0   , SSP_SIGNAL_SSI_INT                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "DA7217 SSI IF "  },
  {  ELC_EVENT_SSI1_TXI_RXI                ,   120,    "SSI1_TXI_RXI             ",  SSP_IP_SSI         , 1   , SSP_SIGNAL_SSI_TXI_RXI                    , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SSI1_INT                    ,   121,    "SSI1_INT                 ",  SSP_IP_SSI         , 1   , SSP_SIGNAL_SSI_INT                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SRC_INPUT_FIFO_EMPTY        ,   122,    "SRC_INPUT_FIFO_EMPTY     ",  SSP_IP_SRC         , 0   , SSP_SIGNAL_SRC_INPUT_FIFO_EMPTY           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SRC_OUTPUT_FIFO_FULL        ,   123,    "SRC_OUTPUT_FIFO_FULL     ",  SSP_IP_SRC         , 0   , SSP_SIGNAL_SRC_OUTPUT_FIFO_FULL           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SRC_OUTPUT_FIFO_OVERFLOW    ,   124,    "SRC_OUTPUT_FIFO_OVERFLOW ",  SSP_IP_SRC         , 0   , SSP_SIGNAL_SRC_OUTPUT_FIFO_OVERFLOW       , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SRC_OUTPUT_FIFO_UNDERFLOW   ,   125,    "SRC_OUTPUT_FIFO_UNDERFLOW",  SSP_IP_SRC         , 0   , SSP_SIGNAL_SRC_OUTPUT_FIFO_UNDERFLOW      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SRC_CONVERSION_END          ,   126,    "SRC_CONVERSION_END       ",  SSP_IP_SRC         , 0   , SSP_SIGNAL_SRC_CONVERSION_END             , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_PDC_RECEIVE_DATA_READY      ,   127,    "PDC_RECEIVE_DATA_READY   ",  SSP_IP_PDC         , 0   , SSP_SIGNAL_PDC_RECEIVE_DATA_READY         , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_PDC_FRAME_END               ,   128,    "PDC_FRAME_END            ",  SSP_IP_PDC         , 0   , SSP_SIGNAL_PDC_FRAME_END                  , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_PDC_INT                     ,   129,    "PDC_INT                  ",  SSP_IP_PDC         , 0   , SSP_SIGNAL_PDC_INT                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CTSU_WRITE                  ,   130,    "CTSU_WRITE               ",  SSP_IP_CTSU        , 0   , SSP_SIGNAL_CTSU_WRITE                     , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CTSU_READ                   ,   131,    "CTSU_READ                ",  SSP_IP_CTSU        , 0   , SSP_SIGNAL_CTSU_READ                      , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CTSU_END                    ,   132,    "CTSU_END                 ",  SSP_IP_CTSU        , 0   , SSP_SIGNAL_CTSU_END                       , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_KEY_INT                     ,   133,    "KEY_INT                  ",  SSP_IP_KEY         , 0   , SSP_SIGNAL_KEY_INT                        , 0    ,  1   ,   0  ,    0  ,     1  ,     1  ,    0   ,   " "  },
  {  ELC_EVENT_DOC_INT                     ,   134,    "DOC_INT                  ",  SSP_IP_DOC         , 0   , SSP_SIGNAL_DOC_INT                        , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAC_FREQUENCY_ERROR         ,   135,    "CAC_FREQUENCY_ERROR      ",  SSP_IP_CAC         , 0   , SSP_SIGNAL_CAC_FREQUENCY_ERROR            , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAC_MEASUREMENT_END         ,   136,    "CAC_MEASUREMENT_END      ",  SSP_IP_CAC         , 0   , SSP_SIGNAL_CAC_MEASUREMENT_END            , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAC_OVERFLOW                ,   137,    "CAC_OVERFLOW             ",  SSP_IP_CAC         , 0   , SSP_SIGNAL_CAC_OVERFLOW                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAN0_ERROR                  ,   138,    "CAN0_ERROR               ",  SSP_IP_CAN         , 0   , SSP_SIGNAL_CAN_ERROR                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "CAN0_ERROR     "  },
  {  ELC_EVENT_CAN0_FIFO_RX                ,   139,    "CAN0_FIFO_RX             ",  SSP_IP_CAN         , 0   , SSP_SIGNAL_CAN_FIFO_RX                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "CAN0_FIFO_RX   "  },
  {  ELC_EVENT_CAN0_FIFO_TX                ,   140,    "CAN0_FIFO_TX             ",  SSP_IP_CAN         , 0   , SSP_SIGNAL_CAN_FIFO_TX                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "CAN0_FIFO_TX   "  },
  {  ELC_EVENT_CAN0_MAILBOX_RX             ,   141,    "CAN0_MAILBOX_RX          ",  SSP_IP_CAN         , 0   , SSP_SIGNAL_CAN_MAILBOX_RX                 , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "CAN0_MAILBOX_RX"  },
  {  ELC_EVENT_CAN0_MAILBOX_TX             ,   142,    "CAN0_MAILBOX_TX          ",  SSP_IP_CAN         , 0   , SSP_SIGNAL_CAN_MAILBOX_TX                 , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "CAN0_MAILBOX_TX"  },
  {  ELC_EVENT_CAN1_ERROR                  ,   143,    "CAN1_ERROR               ",  SSP_IP_CAN         , 1   , SSP_SIGNAL_CAN_ERROR                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAN1_FIFO_RX                ,   144,    "CAN1_FIFO_RX             ",  SSP_IP_CAN         , 1   , SSP_SIGNAL_CAN_FIFO_RX                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAN1_FIFO_TX                ,   145,    "CAN1_FIFO_TX             ",  SSP_IP_CAN         , 1   , SSP_SIGNAL_CAN_FIFO_TX                    , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAN1_MAILBOX_RX             ,   146,    "CAN1_MAILBOX_RX          ",  SSP_IP_CAN         , 1   , SSP_SIGNAL_CAN_MAILBOX_RX                 , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_CAN1_MAILBOX_TX             ,   147,    "CAN1_MAILBOX_TX          ",  SSP_IP_CAN         , 1   , SSP_SIGNAL_CAN_MAILBOX_TX                 , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IOPORT_EVENT_1              ,   148,    "IOPORT_EVENT_1           ",  SSP_IP_IOPORT      , 0   , SSP_SIGNAL_IOPORT_EVENT_1                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IOPORT_EVENT_2              ,   149,    "IOPORT_EVENT_2           ",  SSP_IP_IOPORT      , 0   , SSP_SIGNAL_IOPORT_EVENT_2                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IOPORT_EVENT_3              ,   150,    "IOPORT_EVENT_3           ",  SSP_IP_IOPORT      , 0   , SSP_SIGNAL_IOPORT_EVENT_3                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_IOPORT_EVENT_4              ,   151,    "IOPORT_EVENT_4           ",  SSP_IP_IOPORT      , 0   , SSP_SIGNAL_IOPORT_EVENT_4                 , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ELC_SOFTWARE_EVENT_0        ,   152,    "ELC_SOFTWARE_EVENT_0     ",  SSP_IP_ELC         , 0   , SSP_SIGNAL_ELC_SOFTWARE_EVENT_0           , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_ELC_SOFTWARE_EVENT_1        ,   153,    "ELC_SOFTWARE_EVENT_1     ",  SSP_IP_ELC         , 0   , SSP_SIGNAL_ELC_SOFTWARE_EVENT_1           , 0    ,  1   ,   1  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_POEG0_EVENT                 ,   154,    "POEG0_EVENT              ",  SSP_IP_POEG        , 0   , SSP_SIGNAL_POEG_EVENT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_POEG1_EVENT                 ,   155,    "POEG1_EVENT              ",  SSP_IP_POEG        , 1   , SSP_SIGNAL_POEG_EVENT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_POEG2_EVENT                 ,   156,    "POEG2_EVENT              ",  SSP_IP_POEG        , 2   , SSP_SIGNAL_POEG_EVENT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_POEG3_EVENT                 ,   157,    "POEG3_EVENT              ",  SSP_IP_POEG        , 3   , SSP_SIGNAL_POEG_EVENT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_CAPTURE_COMPARE_A      ,   176,    "GPT0_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_CAPTURE_COMPARE_B      ,   177,    "GPT0_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COMPARE_C              ,   178,    "GPT0_COMPARE_C           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COMPARE_D              ,   179,    "GPT0_COMPARE_D           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COMPARE_E              ,   180,    "GPT0_COMPARE_E           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COMPARE_F              ,   181,    "GPT0_COMPARE_F           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COUNTER_OVERFLOW       ,   182,    "GPT0_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_COUNTER_UNDERFLOW      ,   183,    "GPT0_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_AD_TRIG_A              ,   184,    "GPT0_AD_TRIG_A           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT0_AD_TRIG_B              ,   185,    "GPT0_AD_TRIG_B           ",  SSP_IP_GPT         , 0   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_CAPTURE_COMPARE_A      ,   186,    "GPT1_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_CAPTURE_COMPARE_B      ,   187,    "GPT1_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COMPARE_C              ,   188,    "GPT1_COMPARE_C           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COMPARE_D              ,   189,    "GPT1_COMPARE_D           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COMPARE_E              ,   190,    "GPT1_COMPARE_E           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COMPARE_F              ,   191,    "GPT1_COMPARE_F           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COUNTER_OVERFLOW       ,   192,    "GPT1_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_COUNTER_UNDERFLOW      ,   193,    "GPT1_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_AD_TRIG_A              ,   194,    "GPT1_AD_TRIG_A           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT1_AD_TRIG_B              ,   195,    "GPT1_AD_TRIG_B           ",  SSP_IP_GPT         , 1   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_CAPTURE_COMPARE_A      ,   196,    "GPT2_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_CAPTURE_COMPARE_B      ,   197,    "GPT2_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COMPARE_C              ,   198,    "GPT2_COMPARE_C           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COMPARE_D              ,   199,    "GPT2_COMPARE_D           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COMPARE_E              ,   200,    "GPT2_COMPARE_E           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COMPARE_F              ,   201,    "GPT2_COMPARE_F           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COUNTER_OVERFLOW       ,   202,    "GPT2_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_COUNTER_UNDERFLOW      ,   203,    "GPT2_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_AD_TRIG_A              ,   204,    "GPT2_AD_TRIG_A           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT2_AD_TRIG_B              ,   205,    "GPT2_AD_TRIG_B           ",  SSP_IP_GPT         , 2   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_CAPTURE_COMPARE_A      ,   206,    "GPT3_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_CAPTURE_COMPARE_B      ,   207,    "GPT3_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COMPARE_C              ,   208,    "GPT3_COMPARE_C           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COMPARE_D              ,   209,    "GPT3_COMPARE_D           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COMPARE_E              ,   210,    "GPT3_COMPARE_E           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COMPARE_F              ,   211,    "GPT3_COMPARE_F           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COUNTER_OVERFLOW       ,   212,    "GPT3_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_COUNTER_UNDERFLOW      ,   213,    "GPT3_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_AD_TRIG_A              ,   214,    "GPT3_AD_TRIG_A           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT3_AD_TRIG_B              ,   215,    "GPT3_AD_TRIG_B           ",  SSP_IP_GPT         , 3   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_CAPTURE_COMPARE_A      ,   216,    "GPT4_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_CAPTURE_COMPARE_B      ,   217,    "GPT4_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COMPARE_C              ,   218,    "GPT4_COMPARE_C           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COMPARE_D              ,   219,    "GPT4_COMPARE_D           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COMPARE_E              ,   220,    "GPT4_COMPARE_E           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COMPARE_F              ,   221,    "GPT4_COMPARE_F           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COUNTER_OVERFLOW       ,   222,    "GPT4_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_COUNTER_UNDERFLOW      ,   223,    "GPT4_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_AD_TRIG_A              ,   224,    "GPT4_AD_TRIG_A           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT4_AD_TRIG_B              ,   225,    "GPT4_AD_TRIG_B           ",  SSP_IP_GPT         , 4   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_CAPTURE_COMPARE_A      ,   226,    "GPT5_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_CAPTURE_COMPARE_B      ,   227,    "GPT5_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COMPARE_C              ,   228,    "GPT5_COMPARE_C           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COMPARE_D              ,   229,    "GPT5_COMPARE_D           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COMPARE_E              ,   230,    "GPT5_COMPARE_E           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COMPARE_F              ,   231,    "GPT5_COMPARE_F           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COUNTER_OVERFLOW       ,   232,    "GPT5_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_COUNTER_UNDERFLOW      ,   233,    "GPT5_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_AD_TRIG_A              ,   234,    "GPT5_AD_TRIG_A           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT5_AD_TRIG_B              ,   235,    "GPT5_AD_TRIG_B           ",  SSP_IP_GPT         , 5   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_CAPTURE_COMPARE_A      ,   236,    "GPT6_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_CAPTURE_COMPARE_B      ,   237,    "GPT6_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COMPARE_C              ,   238,    "GPT6_COMPARE_C           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COMPARE_D              ,   239,    "GPT6_COMPARE_D           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COMPARE_E              ,   240,    "GPT6_COMPARE_E           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COMPARE_F              ,   241,    "GPT6_COMPARE_F           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COUNTER_OVERFLOW       ,   242,    "GPT6_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_COUNTER_UNDERFLOW      ,   243,    "GPT6_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_AD_TRIG_A              ,   244,    "GPT6_AD_TRIG_A           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT6_AD_TRIG_B              ,   245,    "GPT6_AD_TRIG_B           ",  SSP_IP_GPT         , 6   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_CAPTURE_COMPARE_A      ,   246,    "GPT7_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_CAPTURE_COMPARE_B      ,   247,    "GPT7_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COMPARE_C              ,   248,    "GPT7_COMPARE_C           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COMPARE_D              ,   249,    "GPT7_COMPARE_D           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COMPARE_E              ,   250,    "GPT7_COMPARE_E           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COMPARE_F              ,   251,    "GPT7_COMPARE_F           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COUNTER_OVERFLOW       ,   252,    "GPT7_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_COUNTER_UNDERFLOW      ,   253,    "GPT7_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_AD_TRIG_A              ,   254,    "GPT7_AD_TRIG_A           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT7_AD_TRIG_B              ,   255,    "GPT7_AD_TRIG_B           ",  SSP_IP_GPT         , 7   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_CAPTURE_COMPARE_A      ,   256,    "GPT8_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_CAPTURE_COMPARE_B      ,   257,    "GPT8_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COMPARE_C              ,   258,    "GPT8_COMPARE_C           ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COMPARE_D              ,   259,    "GPT8_COMPARE_D           ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COMPARE_E              ,   260,    "GPT8_COMPARE_E           ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COMPARE_F              ,   261,    "GPT8_COMPARE_F           ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COUNTER_OVERFLOW       ,   262,    "GPT8_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT8_COUNTER_UNDERFLOW      ,   263,    "GPT8_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 8   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_CAPTURE_COMPARE_A      ,   266,    "GPT9_CAPTURE_COMPARE_A   ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_AD_TRIG_A                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_CAPTURE_COMPARE_B      ,   267,    "GPT9_CAPTURE_COMPARE_B   ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_AD_TRIG_B                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COMPARE_C              ,   268,    "GPT9_COMPARE_C           ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COMPARE_D              ,   269,    "GPT9_COMPARE_D           ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COMPARE_E              ,   270,    "GPT9_COMPARE_E           ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COMPARE_F              ,   271,    "GPT9_COMPARE_F           ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COUNTER_OVERFLOW       ,   272,    "GPT9_COUNTER_OVERFLOW    ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT9_COUNTER_UNDERFLOW      ,   273,    "GPT9_COUNTER_UNDERFLOW   ",  SSP_IP_GPT         , 9   , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_CAPTURE_COMPARE_A     ,   276,    "GPT10_CAPTURE_COMPARE_A  ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_CAPTURE_COMPARE_B     ,   277,    "GPT10_CAPTURE_COMPARE_B  ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COMPARE_C             ,   278,    "GPT10_COMPARE_C          ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COMPARE_D             ,   279,    "GPT10_COMPARE_D          ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COMPARE_E             ,   280,    "GPT10_COMPARE_E          ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COMPARE_F             ,   281,    "GPT10_COMPARE_F          ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COUNTER_OVERFLOW      ,   282,    "GPT10_COUNTER_OVERFLOW   ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT10_COUNTER_UNDERFLOW     ,   283,    "GPT10_COUNTER_UNDERFLOW  ",  SSP_IP_GPT         , 10  , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_CAPTURE_COMPARE_A     ,   286,    "GPT11_CAPTURE_COMPARE_A  ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_CAPTURE_COMPARE_B     ,   287,    "GPT11_CAPTURE_COMPARE_B  ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COMPARE_C             ,   288,    "GPT11_COMPARE_C          ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COMPARE_D             ,   289,    "GPT11_COMPARE_D          ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COMPARE_E             ,   290,    "GPT11_COMPARE_E          ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COMPARE_F             ,   291,    "GPT11_COMPARE_F          ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COUNTER_OVERFLOW      ,   292,    "GPT11_COUNTER_OVERFLOW   ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT11_COUNTER_UNDERFLOW     ,   293,    "GPT11_COUNTER_UNDERFLOW  ",  SSP_IP_GPT         , 11  , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_CAPTURE_COMPARE_A     ,   296,    "GPT12_CAPTURE_COMPARE_A  ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_CAPTURE_COMPARE_B     ,   297,    "GPT12_CAPTURE_COMPARE_B  ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COMPARE_C             ,   298,    "GPT12_COMPARE_C          ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COMPARE_D             ,   299,    "GPT12_COMPARE_D          ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COMPARE_E             ,   300,    "GPT12_COMPARE_E          ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COMPARE_F             ,   301,    "GPT12_COMPARE_F          ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COUNTER_OVERFLOW      ,   302,    "GPT12_COUNTER_OVERFLOW   ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT12_COUNTER_UNDERFLOW     ,   303,    "GPT12_COUNTER_UNDERFLOW  ",  SSP_IP_GPT         , 12  , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_CAPTURE_COMPARE_A     ,   306,    "GPT13_CAPTURE_COMPARE_A  ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_A          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_CAPTURE_COMPARE_B     ,   307,    "GPT13_CAPTURE_COMPARE_B  ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_CAPTURE_COMPARE_B          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COMPARE_C             ,   308,    "GPT13_COMPARE_C          ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COMPARE_C                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COMPARE_D             ,   309,    "GPT13_COMPARE_D          ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COMPARE_D                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COMPARE_E             ,   310,    "GPT13_COMPARE_E          ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COMPARE_E                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COMPARE_F             ,   311,    "GPT13_COMPARE_F          ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COMPARE_F                  , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COUNTER_OVERFLOW      ,   312,    "GPT13_COUNTER_OVERFLOW   ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COUNTER_OVERFLOW           , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GPT13_COUNTER_UNDERFLOW     ,   313,    "GPT13_COUNTER_UNDERFLOW  ",  SSP_IP_GPT         , 13  , SSP_SIGNAL_GPT_COUNTER_UNDERFLOW          , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_OPS_UVW_EDGE                ,   336,    "OPS_UVW_EDGE             ",  SSP_IP_OPS         , 0   , SSP_SIGNAL_OPS_UVW_EDGE                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_EPTPC_IPLS                  ,   352,    "EPTPC_IPLS               ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_IPLS                     , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_EPTPC_MINT                  ,   353,    "EPTPC_MINT               ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_MINT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_EPTPC_PINT                  ,   354,    "EPTPC_PINT               ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_PINT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_EDMAC0_EINT                 ,   355,    "EDMAC0_EINT              ",  SSP_IP_EDMAC       , 0   , SSP_SIGNAL_EDMAC_EINT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
//{  ELC_EVENT_EDMAC1_EINT                 ,   356,    "EDMAC1_EINT              ",  SSP_IP_EDMAC       , 1   , SSP_SIGNAL_EDMAC_EINT                     , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER0_RISE           ,   357,    "EPTPC_TIMER0_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER0_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER1_RISE           ,   358,    "EPTPC_TIMER1_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER0_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER2_RISE           ,   359,    "EPTPC_TIMER2_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER1_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER3_RISE           ,   360,    "EPTPC_TIMER3_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER1_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER4_RISE           ,   361,    "EPTPC_TIMER4_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER2_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER5_RISE           ,   362,    "EPTPC_TIMER5_RISE        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER2_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER0_FALL           ,   363,    "EPTPC_TIMER0_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER3_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER1_FALL           ,   364,    "EPTPC_TIMER1_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER3_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER2_FALL           ,   365,    "EPTPC_TIMER2_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER4_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER3_FALL           ,   366,    "EPTPC_TIMER3_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER4_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER4_FALL           ,   367,    "EPTPC_TIMER4_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER5_FALL              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_EPTPC_TIMER5_FALL           ,   368,    "EPTPC_TIMER5_FALL        ",  SSP_IP_EPTPC       , 0   , SSP_SIGNAL_EPTPC_TIMER5_RISE              , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_USBHS_FIFO_0                ,   369,    "USBHS_FIFO_0             ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_FIFO_0                     , 1    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_USBHS_FIFO_1                ,   370,    "USBHS_FIFO_1             ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_FIFO_1                     , 1    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_USBHS_USB_INT_RESUME        ,   371,    "USBHS_USB_INT_RESUME     ",  SSP_IP_USB         , 0   , SSP_SIGNAL_USB_INT                        , 1    ,  1   ,   0  ,    0  ,     1  ,     1  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_RXI                    ,   372,    "SCI0_RXI                 ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_TXI                    ,   373,    "SCI0_TXI                 ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_TEI                    ,   374,    "SCI0_TEI                 ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_ERI                    ,   375,    "SCI0_ERI                 ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_AM                     ,   376,    "SCI0_AM                  ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI0_RXI_OR_ERI             ,   377,    "SCI0_RXI_OR_ERI          ",  SSP_IP_SCI         , 0   , SSP_SIGNAL_SCI_RXI_OR_ERI                 , 0    ,  0   ,   0  ,    0  ,     1  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI1_RXI                    ,   378,    "SCI1_RXI                 ",  SSP_IP_SCI         , 1   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI1_TXI                    ,   379,    "SCI1_TXI                 ",  SSP_IP_SCI         , 1   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI1_TEI                    ,   380,    "SCI1_TEI                 ",  SSP_IP_SCI         , 1   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI1_ERI                    ,   381,    "SCI1_ERI                 ",  SSP_IP_SCI         , 1   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI1_AM                     ,   382,    "SCI1_AM                  ",  SSP_IP_SCI         , 1   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI2_RXI                    ,   384,    "SCI2_RXI                 ",  SSP_IP_SCI         , 2   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI2_TXI                    ,   385,    "SCI2_TXI                 ",  SSP_IP_SCI         , 2   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI2_TEI                    ,   386,    "SCI2_TEI                 ",  SSP_IP_SCI         , 2   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI2_ERI                    ,   387,    "SCI2_ERI                 ",  SSP_IP_SCI         , 2   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI2_AM                     ,   388,    "SCI2_AM                  ",  SSP_IP_SCI         , 2   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI3_RXI                    ,   390,    "SCI3_RXI                 ",  SSP_IP_SCI         , 3   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "DCDC RX"  },
  {  ELC_EVENT_SCI3_TXI                    ,   391,    "SCI3_TXI                 ",  SSP_IP_SCI         , 3   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "DCDC TX"  },
  {  ELC_EVENT_SCI3_TEI                    ,   392,    "SCI3_TEI                 ",  SSP_IP_SCI         , 3   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "DCDC END"  },
  {  ELC_EVENT_SCI3_ERI                    ,   393,    "SCI3_ERI                 ",  SSP_IP_SCI         , 3   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "DCDC ERR"  },
  {  ELC_EVENT_SCI3_AM                     ,   394,    "SCI3_AM                  ",  SSP_IP_SCI         , 3   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "DCDC AN"  },
  {  ELC_EVENT_SCI4_RXI                    ,   396,    "SCI4_RXI                 ",  SSP_IP_SCI         , 4   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI4_TXI                    ,   397,    "SCI4_TXI                 ",  SSP_IP_SCI         , 4   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI4_TEI                    ,   398,    "SCI4_TEI                 ",  SSP_IP_SCI         , 4   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI4_ERI                    ,   399,    "SCI4_ERI                 ",  SSP_IP_SCI         , 4   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI4_AM                     ,   400,    "SCI4_AM                  ",  SSP_IP_SCI         , 4   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI5_RXI                    ,   402,    "SCI5_RXI                 ",  SSP_IP_SCI         , 5   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "SX1262 RX"  },
  {  ELC_EVENT_SCI5_TXI                    ,   403,    "SCI5_TXI                 ",  SSP_IP_SCI         , 5   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "SX1262 TX"  },
  {  ELC_EVENT_SCI5_TEI                    ,   404,    "SCI5_TEI                 ",  SSP_IP_SCI         , 5   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "SX1262 TEI"  },
  {  ELC_EVENT_SCI5_ERI                    ,   405,    "SCI5_ERI                 ",  SSP_IP_SCI         , 5   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "SX1262 ERI"  },
  {  ELC_EVENT_SCI5_AM                     ,   406,    "SCI5_AM                  ",  SSP_IP_SCI         , 5   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI6_RXI                    ,   408,    "SCI6_RXI                 ",  SSP_IP_SCI         , 6   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI6_TXI                    ,   409,    "SCI6_TXI                 ",  SSP_IP_SCI         , 6   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI6_TEI                    ,   410,    "SCI6_TEI                 ",  SSP_IP_SCI         , 6   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI6_ERI                    ,   411,    "SCI6_ERI                 ",  SSP_IP_SCI         , 6   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI6_AM                     ,   412,    "SCI6_AM                  ",  SSP_IP_SCI         , 6   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCI7_RXI                    ,   414,    "SCI7_RXI                 ",  SSP_IP_SCI         , 7   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "RS485 RX"  },
  {  ELC_EVENT_SCI7_TXI                    ,   415,    "SCI7_TXI                 ",  SSP_IP_SCI         , 7   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "RS485 TX"  },
  {  ELC_EVENT_SCI7_TEI                    ,   416,    "SCI7_TEI                 ",  SSP_IP_SCI         , 7   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "RS485 END"  },
  {  ELC_EVENT_SCI7_ERI                    ,   417,    "SCI7_ERI                 ",  SSP_IP_SCI         , 7   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "RS485 ERR"  },
  {  ELC_EVENT_SCI7_AM                     ,   418,    "SCI7_AM                  ",  SSP_IP_SCI         , 7   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "RS485 AM"  },
  {  ELC_EVENT_SCI8_RXI                    ,   420,    "SCI8_RXI                 ",  SSP_IP_SCI         , 8   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "ADS7028, AB1815 RX"  },
  {  ELC_EVENT_SCI8_TXI                    ,   421,    "SCI8_TXI                 ",  SSP_IP_SCI         , 8   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "ADS7028, AB1815 TX"  },
  {  ELC_EVENT_SCI8_TEI                    ,   422,    "SCI8_TEI                 ",  SSP_IP_SCI         , 8   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "ADS7028, AB1815 END" },
  {  ELC_EVENT_SCI8_ERI                    ,   423,    "SCI8_ERI                 ",  SSP_IP_SCI         , 8   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "ADS7028, AB1815 ERR" },
  {  ELC_EVENT_SCI8_AM                     ,   424,    "SCI8_AM                  ",  SSP_IP_SCI         , 8   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "ADS7028, AB1815 AM"  },
  {  ELC_EVENT_SCI9_RXI                    ,   426,    "SCI9_RXI                 ",  SSP_IP_SCI         , 9   , SSP_SIGNAL_SCI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "Display RX"  },
  {  ELC_EVENT_SCI9_TXI                    ,   427,    "SCI9_TXI                 ",  SSP_IP_SCI         , 9   , SSP_SIGNAL_SCI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "Display TX"  },
  {  ELC_EVENT_SCI9_TEI                    ,   428,    "SCI9_TEI                 ",  SSP_IP_SCI         , 9   , SSP_SIGNAL_SCI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "Display END"  },
  {  ELC_EVENT_SCI9_ERI                    ,   429,    "SCI9_ERI                 ",  SSP_IP_SCI         , 9   , SSP_SIGNAL_SCI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "Display ERR"  },
  {  ELC_EVENT_SCI9_AM                     ,   430,    "SCI9_AM                  ",  SSP_IP_SCI         , 9   , SSP_SIGNAL_SCI_AM                         , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "Display AM"  },
  {  ELC_EVENT_SPI0_RXI                    ,   444,    "SPI0_RXI                 ",  SSP_IP_SPI         , 0   , SSP_SIGNAL_SPI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI0_TXI                    ,   445,    "SPI0_TXI                 ",  SSP_IP_SPI         , 0   , SSP_SIGNAL_SPI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI0_IDLE                   ,   446,    "SPI0_IDLE                ",  SSP_IP_SPI         , 0   , SSP_SIGNAL_SPI_IDLE                       , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI0_ERI                    ,   447,    "SPI0_ERI                 ",  SSP_IP_SPI         , 0   , SSP_SIGNAL_SPI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI0_TEI                    ,   448,    "SPI0_TEI                 ",  SSP_IP_SPI         , 0   , SSP_SIGNAL_SPI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI1_RXI                    ,   449,    "SPI1_RXI                 ",  SSP_IP_SPI         , 1   , SSP_SIGNAL_SPI_RXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI1_TXI                    ,   450,    "SPI1_TXI                 ",  SSP_IP_SPI         , 1   , SSP_SIGNAL_SPI_TXI                        , 0    ,  1   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI1_IDLE                   ,   451,    "SPI1_IDLE                ",  SSP_IP_SPI         , 1   , SSP_SIGNAL_SPI_IDLE                       , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI1_ERI                    ,   452,    "SPI1_ERI                 ",  SSP_IP_SPI         , 1   , SSP_SIGNAL_SPI_ERI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SPI1_TEI                    ,   453,    "SPI1_TEI                 ",  SSP_IP_SPI         , 1   , SSP_SIGNAL_SPI_TEI                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_QSPI_INT                    ,   454,    "QSPI_INT                 ",  SSP_IP_QSPI        , 0   , SSP_SIGNAL_QSPI_INT                       , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SDHIMMC0_ACCS               ,   455,    "SDHIMMC0_ACCS            ",  SSP_IP_SDHIMMC     , 0   , SSP_SIGNAL_SDHIMMC_ACCS                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "SD CARD"  },
  {  ELC_EVENT_SDHIMMC0_SDIO               ,   456,    "SDHIMMC0_SDIO            ",  SSP_IP_SDHIMMC     , 0   , SSP_SIGNAL_SDHIMMC_SDIO                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "SD CARD"  },
  {  ELC_EVENT_SDHIMMC0_CARD               ,   457,    "SDHIMMC0_CARD            ",  SSP_IP_SDHIMMC     , 0   , SSP_SIGNAL_SDHIMMC_CARD                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "SD CARD"  },
  {  ELC_EVENT_SDHIMMC0_DMA_REQ            ,   458,    "SDHIMMC0_DMA_REQ         ",  SSP_IP_SDHIMMC     , 0   , SSP_SIGNAL_SDHIMMC_DMA_REQ                , 0    ,  0   ,   1  ,    1  ,     0  ,     0  ,    0   ,   "SD CARD"  },
  {  ELC_EVENT_SDHIMMC1_ACCS               ,   459,    "SDHIMMC1_ACCS            ",  SSP_IP_SDHIMMC     , 1   , SSP_SIGNAL_SDHIMMC_ACCS                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "WiFi module"  },
  {  ELC_EVENT_SDHIMMC1_SDIO               ,   460,    "SDHIMMC1_SDIO            ",  SSP_IP_SDHIMMC     , 1   , SSP_SIGNAL_SDHIMMC_SDIO                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "WiFi module"  },
  {  ELC_EVENT_SDHIMMC1_CARD               ,   461,    "SDHIMMC1_CARD            ",  SSP_IP_SDHIMMC     , 1   , SSP_SIGNAL_SDHIMMC_CARD                   , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   "WiFi module"  },
  {  ELC_EVENT_SDHIMMC1_DMA_REQ            ,   462,    "SDHIMMC1_DMA_REQ         ",  SSP_IP_SDHIMMC     , 1   , SSP_SIGNAL_SDHIMMC_DMA_REQ                , 0    ,  0   ,   1  ,    1  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_SCE_PROC_BUSY               ,   481,    "SCE_PROC_BUSY            ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_PROC_BUSY                  , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_ROMOK                   ,   482,    "SCE_ROMOK                ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_ROMOK                      , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_LONG_PLG                ,   483,    "SCE_LONG_PLG             ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_LONG_PLG                   , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_TEST_BUSY               ,   484,    "SCE_TEST_BUSY            ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_TEST_BUSY                  , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_WRRDY_0                 ,   485,    "SCE_WRRDY_0              ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_WRRDY_0                    , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_WRRDY_1                 ,   486,    "SCE_WRRDY_1              ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_WRRDY_1                    , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_WRRDY_4                 ,   487,    "SCE_WRRDY_4              ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_WRRDY_4                    , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_RDRDY_0                 ,   488,    "SCE_RDRDY_0              ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_RDRDY_0                    , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_RDRDY_1                 ,   489,    "SCE_RDRDY_1              ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_RDRDY_1                    , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_INTEGRATE_WRRDY         ,   490,    "SCE_INTEGRATE_WRRDY      ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_INTEGRATE_WRRDY            , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_SCE_INTEGRATE_RDRDY         ,   491,    "SCE_INTEGRATE_RDRDY      ",  SSP_IP_SCE         , 0   , SSP_SIGNAL_SCE_INTEGRATE_RDRDY            , 0    ,  -1  ,   -1 ,    -1 ,     -1 ,     -1 ,    -1  ,   " "  },
  {  ELC_EVENT_GLCDC_LINE_DETECT           ,   506,    "GLCDC_LINE_DETECT        ",  SSP_IP_GLCDC       , 0   , SSP_SIGNAL_GLCDC_LINE_DETECT              , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GLCDC_UNDERFLOW_1           ,   507,    "GLCDC_UNDERFLOW_1        ",  SSP_IP_GLCDC       , 0   , SSP_SIGNAL_GLCDC_UNDERFLOW_1              , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_GLCDC_UNDERFLOW_2           ,   508,    "GLCDC_UNDERFLOW_2        ",  SSP_IP_GLCDC       , 0   , SSP_SIGNAL_GLCDC_UNDERFLOW_2              , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_DRW_INT                     ,   509,    "DRW_INT                  ",  SSP_IP_DRW         , 0   , SSP_SIGNAL_DRW_INT                        , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_JPEG_JEDI                   ,   510,    "JPEG_JEDI                ",  SSP_IP_JPEG        , 0   , SSP_SIGNAL_JPEG_JEDI                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
  {  ELC_EVENT_JPEG_JDTI                   ,   511,    "JPEG_JDTI                ",  SSP_IP_JPEG        , 0   , SSP_SIGNAL_JPEG_JDTI                      , 0    ,  1   ,   0  ,    0  ,     0  ,     0  ,    0   ,   " "  },
};


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_ELC_tbl_size(void)
{
  return sizeof(ELC_tbl) / sizeof(ELC_tbl[0]);
}


/*-----------------------------------------------------------------------------------------------------


  \param indx

  \return T_ELC_tbl_rec*
-----------------------------------------------------------------------------------------------------*/
const T_ELC_tbl_rec* Get_ELC_tbl_rec(uint32_t indx)
{
  if (indx >= Get_ELC_tbl_size())
  {
    return NULL;
  }
  return &ELC_tbl[indx];
}


/*-----------------------------------------------------------------------------------------------------


  \param num

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Find_ELC_tbl_rec_indx_by_num(uint16_t num)
{
  for (uint32_t i=0; i < Get_ELC_tbl_size(); i++)
  {
    if (Get_ELC_tbl_rec(i)->num == num)
    {
      return i;
    }
  }
  return -1;
}


/*-----------------------------------------------------------------------------------------------------
  Поиск в таблице дескрипторов векторов номера прерывания по аттрибутам периферии объявившей прерывание

  Поскольку таблица векторов прерываний и дескрипторов  формируется линкером,
  то нам нужно искать в таблице созданной линкером по какому индексу находится объявленное прерывание, этот индекс совпадает с номером прерывания

  \param feature
  \param signal
  \param p_irq
  \param p_event

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t Find_IRQ_number_by_attrs(uint32_t ssp_ip, uint32_t channel, uint32_t unit, uint32_t signaln,  IRQn_Type *p_irq, elc_event_t *p_event)
{
  extern ssp_vector_info_t *const gp_vector_information;
  extern uint32_t                  g_vector_information_size;
  for (uint32_t i = 0U; i < g_vector_information_size; i++)
  {
    if (gp_vector_information[i].ip_id == ssp_ip)
    {
      if (gp_vector_information[i].ip_channel == channel)
      {
        if (gp_vector_information[i].ip_unit == unit)
        {
          if (gp_vector_information[i].ip_signal == signaln)
          {
            *p_irq   = (IRQn_Type) i;
            *p_event = (elc_event_t) gp_vector_information[i].event_number;
            return SSP_SUCCESS;
          }
        }
      }
    }
  }
  *p_irq   = (IRQn_Type) SSP_INVALID_VECTOR;
  *p_event = (elc_event_t) 0xFF;
  return SSP_ERR_IRQ_BSP_DISABLED;
}

/*-----------------------------------------------------------------------------------------------------
  Поиск в таблице дескрипторов векторов номера прерывания по номеру события вызывающего прерываний
  Поскольку в микроконроллерах Synergy нет жесткого назначения периферии номеров прерываний
  Периферийному модулю может быть назначен любой номер прерывания

  Поскольку таблица векторов прерываний и дескрипторов  формируется линкером,
  то нам нужно искать в таблице созданной линкером по какому индексу находится объявленное прерывание, этот индекс совпадает с номером прерывания
-----------------------------------------------------------------------------------------------------*/
int32_t Find_IRQ_number_by_evt(elc_event_t evt)
{
  extern ssp_vector_info_t *const gp_vector_information;
  extern uint32_t                  g_vector_information_size;

  for (uint32_t i = 0U; i < g_vector_information_size; i++)
  {
    if (gp_vector_information[i].event_number == (uint32_t)evt)
    {
      return i;
    }
  }
  return -1;
}


