// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.04.21
// 12:42:33
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


const T_IO_pins_configuration MC50_pins_cfg[] =
{
  {"P000",&R_PFS->P000PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_IU         " },  //     100  P000  ACMPHS0_IVCMP/ADC0_AN00/ICU0_IRQ06
  {"P001",&R_PFS->P001PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_IV         " },  //     99   P001  ACMPHS1_IVCMP/ADC0_AN01/ICU0_IRQ07
  {"P002",&R_PFS->P002PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_IW         " },  //     98   P002  ACMPHS2_IVCMP/ADC0_AN02/ICU0_IRQ08
  {"P003",&R_PFS->P003PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "VREF_R       " },  //     97   P003  ADC0_AN07/ADC0_PGAVSS0
  {"P004",&R_PFS->P004PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "POS_SENS     " },  //     96   P004  ACMPHS3_IVCMP/ADC1_AN00/ICU0_IRQ09
  {"P005",&R_PFS->P005PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "EXT_TEMP     " },  //     95   P005  ACMPHS4_IVCMP/ADC1_AN01/ICU0_IRQ10
  {"P006",&R_PFS->P006PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "MISC         " },  //     94   P006  ACMPHS5_IVCMP/ADC1_AN02/ICU0_IRQ11
  {"P007",&R_PFS->P007PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_VPWR       " },  //     93   P007  ADC1_AN07/ADC1_PGAVSS0
  {"P008",&R_PFS->P008PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_IPWR       " },  //     92   P008  ADC0_AN03/ICU0_IRQ12
  {"P014",&R_PFS->P014PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "TEMP         " },  //     85   P014  ACMPHS(0-5)_IVREF/ADC0_AN05/ADC1_AN05/DAC0_DA
  {"P015",&R_PFS->P015PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "SOUND        " },  //     84   P015  ACMPHS(0-5)_IVCMP/ADC0_AN06/ADC1_AN06/DAC1_DA/ICU0_IRQ13
  {"P100",&R_PFS->P100PFS,  PSEL_06 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "MDRV_MISO    " },  //     75   P100  AGT0_AGTIO/BUS_DQ00/LCD_EXTCLK/GPT_POEG0_GTETRG/GPT5_GTIOCB/ICU0_IRQ02/IIC1_SCL/KINT0_KRM0/SCI0_RXD/SCI0_SCL/SCI1_SCK/SPI0_MISO
  {"P101",&R_PFS->P101PFS,  PSEL_06 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "MDRV_MOSI    " },  //     74   P101  AGT0_AGTEE/BUS_DQ01/LCD_CLK/GPT_POEG1_GTETRG/GPT5_GTIOCA/ICU0_IRQ01/IIC1_SDA/KINT0_KRM1/SCI0_SDA/SCI0_TXD/SCI1_CTS/SPI0_MOSI
  {"P102",&R_PFS->P102PFS,  PSEL_06 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "MDRV_CLK     " },  //     73   P102  ADC0_ADTRG/AGT0_AGTO/BUS_DQ02/CAN0_CRX/LCD_TCON0/GPT_OPS0_GTOWLO/GPT2_GTIOCB/KINT0_KRM2/SCI0_SCK/SPI0_RSPCK
  {"P103",&R_PFS->P103PFS,  PSEL_06 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "LCD_CS       " },  //     72   P103  BUS_DQ03/CAN0_CTX/LCD_TCON1/GPT_OPS0_GTOWUP/GPT2_GTIOCA/KINT0_KRM3/SCI0_CTS/SPI0_SSL0
  {"P104",&R_PFS->P104PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "MDRV_SC      " },  //     71   P104  BUS_DQ04/LCD_TCON2/GPT_POEG1_GTETRG/GPT1_GTIOCB/ICU0_IRQ01/KINT0_KRM4/SCI8_RXD/SCI8_SCL/SPI0_SSL1
  {"P105",&R_PFS->P105PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "BT1          " },  //     70   P105  BUS_DQ05/LCD_TCON3/GPT_POEG0_GTETRG/GPT1_GTIOCA/ICU0_IRQ00/KINT0_KRM5/SCI8_SDA/SCI8_TXD/SPI0_SSL2
  {"P106",&R_PFS->P106PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "BT2          " },  //     69   P106  AGT0_AGTOB/BUS_DQ06/LCD_D00/GPT8_GTIOCB/KINT0_KRM6/SCI8_SCK/SPI0_SSL3
  {"P107",&R_PFS->P107PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "BT3          " },  //     68   P107  AGT0_AGTOA/BUS_DQ07/LCD_D01/GPT8_GTIOCA/KINT0_KRM7/SCI8_CTS
  {"P108",&R_PFS->P108PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "S5_SWDIO     " },  //     51   P108  DEBUG0_SWDIO/DEBUG0_TMS/GPT_OPS0_GTOULO/GPT0_GTIOCB/SCI9_CTS/SPI1_SSL0
  {"P109",&R_PFS->P109PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "S5_SWV       " },  //     52   P109  CAN1_CTX/CGC0_CLKOUT/DEBUG0_TDO/DEBUG0_TRACESWO/GPT_OPS0_GTOVUP/GPT1_GTIOCA/SCI9_SDA/SCI9_TXD/SPI1_MOSI
  {"P110",&R_PFS->P110PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "S5_TDI       " },  //     53   P110  ACMP(0-5)_VCOUT/CAN1_CRX/DEBUG0_TDI/GPT_OPS0_GTOVLO/GPT1_GTIOCB/ICU0_IRQ03/SCI2_CTS/SCI9_RXD/SCI9_SCL/SPI1_MISO
  {"P111",&R_PFS->P111PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "MDRV_EN      " },  //     54   P111  BUS_A05/LCD_D12/GPT3_GTIOCA/ICU0_IRQ04/SCI2_SCK/SCI9_SCK/SPI1_RSPCK
  {"P112",&R_PFS->P112PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "LED_CAN      " },  //     55   P112  BUS_A04/LCD_D11/GPT3_GTIOCB/SCI1_SCK/SCI2_SDA/SCI2_TXD/SPI1_SSL0/SSI0_SSISCK
  {"P113",&R_PFS->P113PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT2_A       " },  //     56   P113  BUS_A03/LCD_D10/GPT2_GTIOCA/SCI2_RXD/SCI2_SCL/SSI0_SSIWS
  {"P114",&R_PFS->P114PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT2_B       " },  //     57   P114  BUS_A02/LCD_D09/GPT2_GTIOCB/SSI0_SSIRXD
  {"P115",&R_PFS->P115PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "LED_RS485    " },  //     58   P115  BUS_A01/LCD_D08/GPT4_GTIOCA/SSI0_SSITXD
  {"P200",&R_PFS->P200PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "NMI          " },  //     40   P200  ICU0_NMI
  {"P201",&R_PFS->P201PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "BOOT         " },  //     39   P201  SYSTEM0_MD
  {"P205",&R_PFS->P205PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_DAT3      " },  //     32   P205  AGT1_AGTO/BUS_A16/CGC0_CLKOUT/CTSU0_TSCAP/ETH_MII0_WOL/ETH_RMII0_WOL/GPT_OPS0_GTIV/GPT4_GTIOCA/ICU0_IRQ01/IIC1_SCL/SCI4_SDA/SCI4_TXD/SCI9_CTS/SDHI0_DAT3/SPI1_SSL0/SSI1_SSIWS/USB_FS0_OVRCURA
  {"P206",&R_PFS->P206PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_DAT2      " },  //     31   P206  BUS_WAIT/CTSU0_TS01/ETH_MII0_LINKSTA/ETH_RMII0_LINKSTA/GPT_OPS0_GTIU/ICU0_IRQ00/IIC1_SDA/SCI4_RXD/SCI4_SCL/SDHI0_DAT2/SPI1_SSL1/SSI1_SSIDATA/USB_FS0_VBUSEN
  {"P207",&R_PFS->P207PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "             " },  //     30   P207  BUS_A17/CTSU0_TS02/LCD_D23/QSPI0_QSSL/SPI1_SSL2
  {"P208",&R_PFS->P208PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 1 , "VL           " },  //     37   P208  DEBUG_TRACE0_TDATA3/ETH_MII0_LINKSTA/ETH_RMII0_LINKSTA/LCD_D18/GPT_OPS0_GTOVLO/QSPI0_QIO3/SDHI0_DAT0
  {"P209",&R_PFS->P209PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 0 , "VH           " },  //     36   P209  DEBUG_TRACE0_TDATA2/ETH_MII0_EXOUT/ETH_RMII0_EXOUT/LCD_D19/GPT_OPS0_GTOVUP/QSPI0_QIO2/SDHI0_WP
  {"P210",&R_PFS->P210PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "LCD_RST      " },  //     35   P210  DEBUG_TRACE0_TDATA1/ETH_MII0_WOL/ETH_RMII0_WOL/LCD_D20/GPT_OPS0_GTIW/QSPI0_QIO1/SDHI0_CD
  {"P211",&R_PFS->P211PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "LCD_RS       " },  //     34   P211  DEBUG_TRACE0_TDATA0/ETH_MII0_MDIO/ETH_RMII0_MDIO/LCD_D21/GPT_OPS0_GTIV/QSPI0_QIO0/SDHI0_CMD
  {"P212",&R_PFS->P212PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "EXTAL        " },  //     14   P212  AGT1_AGTEE/CGC0_EXTAL/GPT_POEG3_GTETRG/GPT0_GTIOCB/ICU0_IRQ03/SCI1_RXD/SCI1_SCL
  {"P213",&R_PFS->P213PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "XTAL          " },  //     13   P213  ADC1_ADTRG/CGC0_XTAL/GPT_POEG2_GTETRG/GPT0_GTIOCA/ICU0_IRQ02/SCI1_SDA/SCI1_TXD
  {"P214",&R_PFS->P214PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "LCD_BLK      " },  //     33   P214  DEBUG_TRACE0_TCLK/ETH_MII0_MDC/ETH_RMII0_MDC/LCD_D22/GPT_OPS0_GTIU/QSPI0_QSPCLK/SDHI0_CLK
  {"P300",&R_PFS->P300PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "S5_SWCLK     " },  //     50   P300  DEBUG0_SWCLK/DEBUG0_TCK/GPT_OPS0_GTOUUP/GPT0_GTIOCA/SPI1_SSL1
  {"P301",&R_PFS->P301PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 1 , "UL           " },  //     49   P301  AGT0_AGTIO/BUS_A06/LCD_D13/GPT_OPS0_GTOULO/GPT4_GTIOCB/ICU0_IRQ06/SCI2_RXD/SCI2_SCL/SCI9_CTS/SPI1_SSL2
  {"P302",&R_PFS->P302PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 0 , "UH           " },  //     48   P302  BUS_A07/LCD_D14/GPT_OPS0_GTOUUP/GPT4_GTIOCA/ICU0_IRQ05/SCI2_SDA/SCI2_TXD/SPI1_SSL3
  {"P303",&R_PFS->P303PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "             " },  //     47   P303  BUS_A08/LCD_D15/GPT7_GTIOCB
  {"P304",&R_PFS->P304PFS,  PSEL_04 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SCI6_RXD BLE " },  //     44   P304  BUS_A09/LCD_D16/GPT_OPS0_GTOWLO/GPT7_GTIOCA/ICU0_IRQ09/SCI6_RXD/SCI6_SCL
  {"P305",&R_PFS->P305PFS,  PSEL_04 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SCI6_TXD BLE " },  //     43   P305  BUS_A10/LCD_D17/GPT_OPS0_GTOWUP/ICU0_IRQ08/QSPI0_QSPCLK/SCI6_SDA/SCI6_TXD
  {"P306",&R_PFS->P306PFS,  PSEL_04 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SCI6_SCK BLE " },  //     42   P306  BUS_A11/LCD_D18/GPT_OPS0_GTOULO/QSPI0_QSSL/SCI6_SCK
  {"P307",&R_PFS->P307PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "SCI6_CTS BLE " },  //     41   P307  BUS_A12/LCD_D19/GPT_OPS0_GTOUUP/QSPI0_QIO0/SCI6_CTS
  {"P400",&R_PFS->P400PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ__EN ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "MDRV_FAULT   " },  //     1    P400  ADC1_ADTRG/AGT1_AGTIO/ETH_MII0_WOL/ETH_RMII0_WOL/GPT6_GTIOCA/ICU0_IRQ00/IIC0_SCL/SCI4_SCK/SCI7_SCK/SSI_COMMON0_AUDIO_CLK
  {"P401",&R_PFS->P401PFS,  PSEL_16 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "CAN_TX       " },  //     2    P401  CAN0_CTX/ETH_MII0_MDC/ETH_RMII0_MDC/GPT_POEG0_GTETRG/GPT6_GTIOCB/ICU0_IRQ05/IIC0_SDA/SCI4_CTS/SCI7_SDA/SCI7_TXD
  {"P402",&R_PFS->P402PFS,  PSEL_16 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "CAN_RX       " },  //     3    P402  AGT0_AGTIO/AGT1_AGTIO/CAC0_CACREF/CAN0_CRX/ETH_MII0_MDIO/ETH_RMII0_MDIO/ICU0_IRQ04/PDC0_VSYNC/RTC0_RTCIC0/SCI7_RXD/SCI7_SCL/SSI_COMMON0_AUDIO_CLK
  {"P403",&R_PFS->P403PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "CAN_STB      " },  //     4    P403  AGT0_AGTIO/AGT1_AGTIO/ETH_MII0_LINKSTA/ETH_RMII0_LINKSTA/GPT3_GTIOCA/PDC0_PIXD7/RTC0_RTCIC1/SCI7_CTS/SDHI1_DAT7/SSI0_SSISCK
  {"P404",&R_PFS->P404PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 1 , "CAN_EN       " },  //     5    P404  ETH_MII0_EXOUT/ETH_RMII0_EXOUT/GPT3_GTIOCB/PDC0_PIXD6/RTC0_RTCIC2/SDHI1_DAT6/SSI0_SSIWS
  {"P405",&R_PFS->P405PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT1_A       " },  //     6    P405  ETH_MII0_TX_EN/ETH_RMII0_TXD_EN/GPT1_GTIOCA/PDC0_PIXD5/SDHI1_DAT5/SSI0_SSITXD
  {"P406",&R_PFS->P406PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT1_B       " },  //     7    P406  ETH_MII0_RX_ER/ETH_RMII0_TXD1/GPT1_GTIOCB/PDC0_PIXD4/SDHI1_DAT4/SPI1_SSL3/SSI0_SSIRXD
  {"P407",&R_PFS->P407PFS,  PSEL_19 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "USBHS_ST     " },  //     25   P407  ADC0_ADTRG/AGT0_AGTIO/CTSU0_TS03/ETH_MII0_EXOUT/ETH_RMII0_EXOUT/IIC0_SDA/RTC0_RTCOUT/SCI4_CTS/SPI1_SSL3/USB_FS0_VBUS
  {"P408",&R_PFS->P408PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 1 , "WL           " },  //     24   P408  CTSU0_TS04/ETH_MII0_CRS/ETH_RMII0_CRS_DV/GPT_OPS0_GTOWLO/GPT10_GTIOCB/ICU0_IRQ07/IIC0_SCL/PDC0_PIXCLK/SCI3_RXD/SCI3_SCL/USB_FS0_ID/USB_HS0_ID
  {"P409",&R_PFS->P409PFS,  PSEL_02 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_OUT , 0 , "WH           " },  //     23   P409  CTSU0_TS05/ETH_MII0_RX_CLK/ETH_RMII0_RX_ER/GPT_OPS0_GTOWUP/GPT10_GTIOCA/ICU0_IRQ06/PDC0_HSYNC/SCI3_SDA/SCI3_TXD/USB_FS0_EXICEN/USB_HS0_EXICEN
  {"P410",&R_PFS->P410PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_DAT1      " },  //     22   P410  AGT1_AGTOB/CTSU0_TS06/ETH_MII0_ERXD0/ETH_RMII0_RXD1/GPT_OPS0_GTOVLO/GPT9_GTIOCB/ICU0_IRQ05/PDC0_PIXD0/SCI0_RXD/SCI0_SCL/SCI3_SCK/SDHI0_DAT1/SPI0_MISO
  {"P411",&R_PFS->P411PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_DAT0      " },  //     21   P411  AGT1_AGTOA/CTSU0_TS07/ETH_MII0_ERXD1/ETH_RMII0_RXD0/GPT_OPS0_GTOVUP/GPT9_GTIOCA/ICU0_IRQ04/PDC0_PIXD1/SCI0_SDA/SCI0_TXD/SCI3_CTS/SDHI0_DAT0/SPI0_MOSI
  {"P412",&R_PFS->P412PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_CMD       " },  //     20   P412  AGT1_AGTEE/CTSU0_TS08/ETH_MII0_ETXD0/ETH_RMII0_REF50CK/GPT_OPS0_GTOULO/PDC0_PIXD2/SCI0_SCK/SDHI0_CMD/SPI0_RSPCK
  {"P413",&R_PFS->P413PFS,  PSEL_21 ,PULLUP__EN ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "SD_CLK       " },  //     19   P413  CTSU0_TS09/ETH_MII0_ETXD1/ETH_RMII0_TXD0/GPT_OPS0_GTOUUP/PDC0_PIXD3/SCI0_CTS/SDHI0_CLK/SPI0_SSL0
  {"P414",&R_PFS->P414PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT0_B       " },  //     18   P414  CTSU0_TS10/ETH_MII0_RX_ER/ETH_RMII0_TXD1/GPT0_GTIOCB/ICU0_IRQ09/PDC0_PIXD4/SDHI0_WP/SPI0_SSL1
  {"P415",&R_PFS->P415PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "GPT0_A       " },  //     17   P415  CTSU0_TS11/ETH_MII0_TX_EN/ETH_RMII0_TXD_EN/GPT0_GTIOCA/ICU0_IRQ08/PDC0_PIXD5/SDHI0_CD/SPI0_SSL2/USB_FS0_VBUSEN
  {"P500",&R_PFS->P500PFS,  PSEL_03 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "ENC_U        " },  //     76   P500  ACMPHS(0-5)_IVREF/ADC0_AN16/AGT0_AGTOA/GPT_OPS0_GTIU/GPT11_GTIOCA/QSPI0_QSPCLK/SDHI1_CLK/USB_FS0_VBUSEN
  {"P501",&R_PFS->P501PFS,  PSEL_03 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "ENC_V        " },  //     77   P501  ACMPHS(0-5)_IVREF/ADC1_AN16/AGT0_AGTOB/GPT_OPS0_GTIV/GPT11_GTIOCB/ICU0_IRQ11/QSPI0_QSSL/SCI5_SDA/SCI5_TXD/SDHI1_CMD/USB_FS0_OVRCURA
  {"P502",&R_PFS->P502PFS,  PSEL_03 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "ENC_W        " },  //     78   P502  ACMPHS(0-5)_IVCMP/ADC0_AN17/GPT_OPS0_GTIW/GPT12_GTIOCA/ICU0_IRQ12/QSPI0_QIO0/SCI5_RXD/SCI5_SCL/SDHI1_DAT0/USB_FS0_OVRCURB
  {"P503",&R_PFS->P503PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL__EN ,PORT__IO ,GP_INP , 0 , "V_V5V        " },  //     79   P503  ADC1_AN17/GPT_POEG2_GTETRG/GPT12_GTIOCB/QSPI0_QIO1/SCI5_SCK/SCI6_CTS/SDHI1_DAT1/USB_FS0_EXICEN
  {"P504",&R_PFS->P504PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "LED_RD       " },  //     80   P504  ADC0_AN18/BUS_ALE/GPT_POEG3_GTETRG/GPT13_GTIOCA/QSPI0_QIO2/SCI5_CTS/SCI6_SCK/SDHI1_DAT2/USB_FS0_ID
  {"P508",&R_PFS->P508PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "LED_GR       " },  //     81   P508  ADC0_AN20/SCI5_SCK/SCI6_SCK
  {"P600",&R_PFS->P600PFS,  PSEL_09 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "CLK_OUT      " },  //     67   P600  BUS_RD/CAC0_CACREF/CGC0_CLKOUT/LCD_D02/GPT6_GTIOCB/SCI9_SCK
  {"P601",&R_PFS->P601PFS,  PSEL_05 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "RS485_RX     " },  //     66   P601  BUS_WR_DQM/LCD_D03/GPT6_GTIOCA/SCI9_RXD/SCI9_SCL
  {"P602",&R_PFS->P602PFS,  PSEL_05 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT_PER ,GP_INP , 0 , "RS485_TX     " },  //     65   P602  BUS_BCLK_SDCLK/LCD_D04/GPT7_GTIOCB/SCI9_SDA/SCI9_TXD
  {"P608",&R_PFS->P608PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_OUT , 0 , "RS485_DE     " },  //     59   P608  BUS_BC_A_DQM00/LCD_D07/GPT4_GTIOCB
  {"P609",&R_PFS->P609PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "             " },  //     60   P609  BUS_CS_CKE1/CAN1_CTX/LCD_D06/GPT5_GTIOCA
  {"P610",&R_PFS->P610PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "             " },  //     61   P610  BUS_CS_WE0/CAN1_CRX/LCD_D05/GPT5_GTIOCB
  {"P708",&R_PFS->P708PFS,  PSEL_00 ,PULLUP_DIS ,OD_DIS ,HI_DRV ,EV_DNC ,IRQ_DIS ,ANAL_DIS ,PORT__IO ,GP_INP , 0 , "             " },  //     16   P708  CAC0_CACREF/CTSU0_TS12/ETH_MII0_ETXD3/ICU0_IRQ11/PDC0_PCKO/SCI1_RXD/SCI1_SCL/SPI0_SSL3/SSI_COMMON0_AUDIO_CLK
};

/*-----------------------------------------------------------------------------------------------------


  \param cgf
  \param sz
-----------------------------------------------------------------------------------------------------*/
static void  Config_pins(const T_IO_pins_configuration *cfg, uint32_t sz)
{
  T_reg_PFS pfs;
  for (uint32_t i=0; i < sz; i++)
  {
    memset(&pfs, 0,  sizeof(pfs));
    pfs.PODR  = cfg[i].init;
    pfs.PDR   = cfg[i].dir;
    pfs.PCR   = cfg[i].PCR;
    pfs.NCODR = cfg[i].ODR;
    pfs.DSCR  = cfg[i].DSCR;
    pfs.ISEL  = cfg[i].ISEL;
    pfs.ASEL  = cfg[i].ASEL;
    pfs.PMR   = cfg[i].PORTM;
    pfs.PSEL  = cfg[i].PSEL;
    memcpy((void *)cfg[i].PFS,&pfs, 4);
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void MC50_board_pins_init(void)
{
  Config_pins(MC50_pins_cfg, sizeof(MC50_pins_cfg) / sizeof(MC50_pins_cfg[0]));
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  MC50_get_pin_count(void)
{
  return sizeof(MC50_pins_cfg) / sizeof(MC50_pins_cfg[0]);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  Get_smpl_enc_a (void)
{
  return R_PFS->P105PFS_b.PIDR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  Get_smpl_enc_b (void)
{
  return R_PFS->P106PFS_b.PIDR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  Get_smpl_enc_sw(void)
{
  return R_PFS->P107PFS_b.PIDR;
}

/*-----------------------------------------------------------------------------------------------------


  \param pin_num
  \param dstr
  \param str_sz
-----------------------------------------------------------------------------------------------------*/
void MC50_get_pin_conf_str(uint32_t pin_num, char *dstr)
{
  char      str[32];
  uint32_t  pin = pin_num;
  uint32_t  tsz = MC50_get_pin_count();
  T_reg_PFS pfs;

  dstr[0] = 0;

  if (pin_num >= tsz)
  {
    return;
  }


  sprintf(str, "%08X ,", MC50_pins_cfg[pin].PFS);
  strcat(dstr, str);

  strcat(dstr, MC50_pins_cfg[pin].name);
  strcat(dstr, " ,");


  memcpy(&pfs, (void *)MC50_pins_cfg[pin].PFS, 4);



  sprintf(str, "PSEL_%02d ,", pfs.PSEL);
  strcat(dstr, str);

  if (pfs.PCR == 0)
  {
    strcat(dstr, "PULLUP_DIS ,");
  }
  else
  {
    strcat(dstr, "PULLUP__EN ,");
  }

  if (pfs.NCODR == 0)
  {
    strcat(dstr, "OD_DIS ,");
  }
  else
  {
    strcat(dstr, "OD__EN ,");
  }


  switch (pfs.DSCR)
  {
  case 0:
    strcat(dstr, "LO_DRV ,");
    break;
  case 1:
    strcat(dstr, "MI_DRV ,");
    break;
  case 2:
    strcat(dstr, "NO_DRV ,");
    break;
  case 3:
    strcat(dstr, "HI_DRV ,");
    break;
  }

  switch (pfs.EOFR)
  {
  case 0:
    strcat(dstr, "EV_DNC ,");
    break;
  case 1:
    strcat(dstr, "EV_DRE ,");
    break;
  case 2:
    strcat(dstr, "EV_DFE ,");
    break;
  case 3:
    strcat(dstr, "EV_DBE ,");
    break;
  }


  if (pfs.ISEL == 0)
  {
    strcat(dstr, "IRQ_DIS ,");
  }
  else
  {
    strcat(dstr, "IRQ__EN ,");
  }

  if (pfs.ASEL == 0)
  {
    strcat(dstr, "ANAL_DIS ,");
  }
  else
  {
    strcat(dstr, "ANAL__EN ,");
  }

  if (pfs.PMR == 0)
  {
    strcat(dstr, "PORT__IO ,");
  }
  else
  {
    strcat(dstr, "PORT_PER ,");
  }

  if (pfs.PDR == 0)
  {
    strcat(dstr, "GP_INP ,");
  }
  else
  {
    strcat(dstr, "GP_OUT ,");
  }

  if (pfs.PODR == 0)
  {
    strcat(dstr, "0 , ");
  }
  else
  {
    strcat(dstr, "1 , ");
  }

  strcat(dstr, MC50_pins_cfg[pin].descr);


}



