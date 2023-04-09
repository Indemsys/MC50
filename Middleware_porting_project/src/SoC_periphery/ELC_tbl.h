#ifndef MC50_ELC_TBL_H
  #define MC50_ELC_TBL_H

typedef struct
{
    elc_event_t evt;
    uint16_t    num;
    char const *name;
    ssp_ip_t    ip;
    uint8_t     channel;
    uint8_t     sig;
    uint8_t     unit;
    int8_t      en_nvic;
    int8_t      en_dtc;
    int8_t      en_dmac;
    int8_t      c_sn;
    int8_t      c_ssb;
    int8_t      c_dssb;
    char const *description;
} T_ELC_tbl_rec;

uint32_t       Get_ELC_tbl_size(void);
const          T_ELC_tbl_rec* Get_ELC_tbl_rec(uint32_t indx);
int32_t        Find_ELC_tbl_rec_indx_by_num(uint16_t num);
ssp_err_t      Find_IRQ_number_by_attrs(uint32_t ssp_ip, uint32_t channel, uint32_t unit, uint32_t signaln,  IRQn_Type *p_irq, elc_event_t *p_event);
int32_t        Find_IRQ_number_by_evt(elc_event_t evt);

#endif



