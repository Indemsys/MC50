#ifndef TRANSFER_CHANNELS_H
  #define TRANSFER_CHANNELS_H

  #define ADDR_FIXED          TRANSFER_ADDR_MODE_FIXED
  #define ADDR__INCR          TRANSFER_ADDR_MODE_INCREMENTED
  #define ADDR__DECR          TRANSFER_ADDR_MODE_DECREMENTED

  #define SRC_REPEATED        TRANSFER_REPEAT_AREA_SOURCE
  #define DST_REPEATED        TRANSFER_REPEAT_AREA_DESTINATION

  #define DISEL_0_IR_RES      TRANSFER_IRQ_END              // DISEL = 0  Generate interrupt request to CPU when specified data transfer is complete
  #define DISEL_1_SETIRQ      TRANSFER_IRQ_EACH             // DISEL = 1  Generate interrupt request to CPU each time DTC data transfer is performed.

  #define CHAIN___DISABLED    TRANSFER_CHAIN_MODE_DISABLED  // CHNS = 0  CHNE = 0
  #define CHAIN_NO_LOOPING    TRANSFER_CHAIN_MODE_EACH      // CHNS = 0  CHNE = 1
  #define CHAIN_AFTER_LOOP    TRANSFER_CHAIN_MODE_END       // CHNS = 1  CHNE = 1

  #define UNIT_1_BYTE         TRANSFER_SIZE_1_BYTE
  #define UNIT_2_BYTE         TRANSFER_SIZE_2_BYTE
  #define UNIT_4_BYTE         TRANSFER_SIZE_4_BYTE

  #define MODE_NORMAL         TRANSFER_MODE_NORMAL          // 00
  #define MODE_REPEAT         TRANSFER_MODE_REPEAT          // 01
  #define MODE__BLOCK         TRANSFER_MODE_BLOCK           // 10


extern spi_api_t const             g_spi_on_sci;
extern i2c_api_master_t const      g_i2c_master_on_riic;


extern const transfer_instance_t   DTC_SPI5_RX_transfer_instance;
extern const transfer_instance_t   DTC_SPI5_TX_transfer_instance;
extern const spi_cfg_t             SPI5_configuration;
extern const spi_instance_t        SPI5_interface_instance;
extern sci_spi_instance_ctrl_t     SPI5_control_block;
extern const sci_spi_extended_cfg  SPI5_extended_configuration;
extern sf_spi_bus_t                SPI5_bus;

extern const transfer_instance_t   DMA_CH2_transfer_instance;

extern const transfer_instance_t   DMA_CH3_transfer_instance;
extern dmac_instance_ctrl_t        DMA_CH3_control_block;


extern const transfer_instance_t   DTC_I2C2_RX_transfer_instance;
extern const transfer_instance_t   DTC_I2C2_TX_transfer_instance;
extern const i2c_cfg_t             I2C2_configuration;
extern const i2c_master_instance_t I2C2_interface_instance;

extern riic_instance_ctrl_t        I2C2_control_block;
extern const riic_extended_cfg     I2C2_extended_configuration;
extern sf_i2c_bus_t                I2C2_bus;



#endif // TRANSFER_CHANNELS_H



