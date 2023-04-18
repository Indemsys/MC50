#ifndef MC50_SPI0_H
  #define MC50_SPI0_H


void     spi0_rxi_isr(void);
void     spi0_txi_isr(void);
void     spi0_tei_isr(void);
void     spi0_eri_isr(void);
void     spi0_idle_isr(void);



uint32_t SPI0_init(void);

uint32_t SPI0_set_for_byte_transfer_to_display(void);
uint32_t SPI0_set_for_hword_transfer_to_display(void);
uint32_t SPI0_set_for_buffer_transfer_to_display(void);
uint32_t SPI0_send_byte_to_display(uint8_t b);
uint32_t SPI0_send_hword_to_display(uint16_t w);
uint32_t SPI0_send_buff_to_display(uint16_t *buf, uint32_t sz);

// Макросы объявлены здесь
// поскольку при записи их в С файле перестает нормально работать броузинг кода в редакторе
#ifdef MC50_SPI0


//........................................................................................
//  Организация канала передачи для SPI0
//  Используется модуль DMAC канал 3
//........................................................................................
SSP_VECTOR_DEFINE_CHAN(dmac_int_isr  , DMAC , INT  , 3)

SSP_VECTOR_DEFINE_CHAN(spi0_rxi_isr  ,  SPI , RXI  , 0)
//SSP_VECTOR_DEFINE_CHAN(spi0_txi_isr  ,  SPI , TXI  , 0) // Если объявить этот вектор, то после инициализации ICU не будут передаваться запросы на DMA
SSP_VECTOR_DEFINE_CHAN(spi0_tei_isr  ,  SPI , TEI  , 0)
SSP_VECTOR_DEFINE_CHAN(spi0_eri_isr  ,  SPI , ERI  , 0)
SSP_VECTOR_DEFINE_CHAN(spi0_idle_isr ,  SPI , IDLE , 0)
#endif

#endif



