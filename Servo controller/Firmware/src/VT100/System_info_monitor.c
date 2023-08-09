// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.06.06
// 11:51:26
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------


  \param p_cbl
-----------------------------------------------------------------------------------------------------*/
static void Print_System_info_header(void)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  SYSTEM INFO ===\n\r");
  MPRINTF("[L] - list registered interrupts, [P] - pins state, [M] - DTC list,  ESC - exit\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Print_memry_statistic(void)
{
  GET_MCBL;
  uint32_t avail_bytes;
  uint32_t fragments;

  MPRINTF("....................................................................\n\r");
  App_get_RAM_pool_statistic(&avail_bytes,&fragments);
  MPRINTF("Pool in RAM free size   = %08d  Pool fragments = %08d\n\r", avail_bytes, fragments);
  MPRINTF("....................................................................\n\r");

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void Print_ICU_Event_Link_Setting_Registers(void)
{
  GET_MCBL;
  uint32_t             i;
  uint32_t             evn;
  uint32_t             ir;
  uint32_t             dtce;
  const T_ELC_tbl_rec *rec;
  uint32_t             *p_vec_tbl_addr;
  uint32_t             *p_vec;
  dtc_reg_t            *p_dtc_cbl;

  MPRINTF("\r\n");
  MPRINTF("ICU Event Link_Setting Registers:\r\n");

  p_vec_tbl_addr = (uint32_t *)R_DTC->DTCVBR_b.DTCVBR;
  MPRINTF("DTCCR.RRS    =%d DTC Transfer Information Read Skip Enable\r\n", R_DTC->DTCCR_b.RRS);
  MPRINTF("DTCVBR.DTCVBR=%08X Base address for calculating the DTC vector table address\r\n", (uint32_t)p_vec_tbl_addr);
  MPRINTF("DTCST.DTCST  =%d DTC Module Start\r\n", R_DTC->DTCST_b.DTCST);
  MPRINTF("DTCSTS.ACT   =%d DTC Active Flag\r\n", R_DTC->DTCSTS_b.ACT);
  MPRINTF("DTCSTS.VECN  =%d DTC-Activating Vector Number Monitoring\r\n", R_DTC->DTCSTS_b.VECN);

  MPRINTF("\r\n");

  p_vec = p_vec_tbl_addr;
  for (i=0; i < 96; i++)
  {
    evn = R_ICU->IELSRn_b[i].IELS;
    if (evn != 0)
    {
      rec = Get_ELC_tbl_rec(Find_ELC_tbl_rec_indx_by_num(evn));
      ir = R_ICU->IELSRn_b[i].IR;
      dtce = R_ICU->IELSRn_b[i].DTCE;
      MPRINTF("........................................\r\n");
      MPRINTF("%02d: Event N=%03d, IR=%d, DTCE=%d, DTC addr=%08X :", i,evn , ir, dtce, (uint32_t)*p_vec);
      MPRINTF("%s  , %s\r\n", rec->name,  rec->description);
      if (*p_vec != 0)
      {
        p_dtc_cbl = (dtc_reg_t *)*p_vec;
        MPRINTF("MRA = %02X, MRB = %02X\r\n",p_dtc_cbl->MRA, p_dtc_cbl->MRB);
        MPRINTF("SAR = %08X\r\n",p_dtc_cbl->SAR);
        MPRINTF("DAR = %08X\r\n",p_dtc_cbl->DAR);
        MPRINTF("CRA = %02X, CRB = %02X\r\n",p_dtc_cbl->CRA, p_dtc_cbl->CRB);
      }

    }
    p_vec++;
  }
  MPRINTF("\r\n");
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Print_registered_interrupts(void)
{
  GET_MCBL;
  uint32_t     evn;
  uint32_t     vifsz = 0;
  uint32_t     ip_channel;
  uint32_t     ip_id;
  uint32_t     ip_unit;
  uint32_t     ip_signal;
  int32_t      indx;
  const T_ELC_tbl_rec *rec;


  //uint32_t *base_addr = (uint32_t *)&R_ICU->IELSRn; // Находим адрес первого регистра из массива регистров типа ICU Event Link Setting Register

  extern ssp_vector_info_t *const gp_vector_information;
#pragma section="VECT_INFO"
  vifsz = __section_size("VECT_INFO") / sizeof(ssp_vector_info_t);

  MPRINTF("Registered interrupt vectors in VECTOR_INFORMATION section:\n\r");
  // Проходим по таблице зарегистрированных векторов
  for (uint32_t i = 0U; i < vifsz; i++)
  {
    evn = gp_vector_information[i].event_number;
    ip_channel = gp_vector_information[i].ip_channel;
    ip_id      = gp_vector_information[i].ip_id;
    ip_unit    = gp_vector_information[i].ip_unit;
    ip_signal  = gp_vector_information[i].ip_signal;
    MPRINTF("%02d: Event N=%03d (p=%02d, c=%02d, s=%02d, u=%02d) -: ", i, evn, ip_id, ip_channel, ip_signal, ip_unit);

    indx = Find_ELC_tbl_rec_indx_by_num(evn);

    if (indx < 0)
    {
      MPRINTF("Not found in events table!");
    }
    else
    {
      rec = Get_ELC_tbl_rec(indx);
      MPRINTF("%s (p=%02d, c=%02d, s=%02d, u=%02d) %s", rec->name, rec->ip, rec->channel, rec->sig, rec->unit, rec->description);
    }

    MPRINTF("\r\n");
  }

}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void Print_pins_state(void)
{
  GET_MCBL;
  uint32_t i;
  char tmp_str[200];
  uint32_t sz = MC50_get_pin_count();

  MPRINTF("\r\n", 10);

  for (i=0; i < sz; i++)
  {
    MC50_get_pin_conf_str(i, tmp_str);
    MPRINTF(tmp_str, 10);
    MPRINTF("\r\n", 10);
  }
  MPRINTF("\r\n", 10);
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_MC50_System_info(uint8_t keycode)
{
  uint8_t                  b;
  T_sys_timestump          st_t;
  fmi_unique_id_t          uid;
  fmi_product_info_t       *p_product_info;
  //char str[32];
  GET_MCBL;



  Print_System_info_header();


  g_fmi.p_api->uniqueIdGet(&uid);

  MPRINTF("Chip Unuque ID     = %08X %08X %08X %08X\r\n", uid.unique_id[0], uid.unique_id[1], uid.unique_id[2], uid.unique_id[3]);

  g_fmi.p_api->productInfoGet(&p_product_info);
  MPRINTF("Product header     = %d %d %d %d %d\r\n", p_product_info->header.contents, p_product_info->header.variant, p_product_info->header.count, p_product_info->header.minor, p_product_info->header.major);
  //  memset(str,0, 32);
  //  memcpy(str, p_product_info->product_name,16);
  //  MPRINTF("Product name       = %s");
  //  memset(str,0, 32);
  //  memcpy(str, p_product_info->product_marking,16);
  //  MPRINTF("  Product marking  = %s\r\n");


  MPRINTF("Max frequency      = %d MHz\r\n", p_product_info->max_freq);
  MPRINTF("Chip pin count     = %d\r\n", p_product_info->pin_count);
  MPRINTF("Chip pkg type      = %d\r\n", p_product_info->pkg_type);

  uint32_t sys_div;
  uint32_t sys_freq;
  uint32_t ticks_freq;

  Get_system_timings(&sys_div, &sys_freq, &ticks_freq);
  MPRINTF("System frequency   = %d Hz\r\n", sys_freq);
  MPRINTF("SysTick divider    = %d\r\n"   , sys_div);
  MPRINTF("SysTick frequency  = %d Hz\r\n", ticks_freq);

  Print_memry_statistic();
  {
    int64_t tdiff;
    st_t.cycles = 0;
    st_t.ticks  = 0;
    tdiff = Hw_timestump_diff64_us(&st_t,&g_main_thread_start_timestump);
    MPRINTF("Main thread start delay = %lld uS\r\n", tdiff);
  }

  {
    uint32_t  t1,t2;
    T_sys_timestump ts1,ts2;
    uint32_t  r1,r2;

    Get_system_ticks(&t1);
    Get_hw_timestump(&ts1);
    tx_thread_sleep(ms_to_ticks(500));
    Get_system_ticks(&t2);
    Get_hw_timestump(&ts2);
    r1 = Time_diff_microseconds(t1, t2);
    r2 = (uint32_t)Hw_timestump_diff64_us(&ts1,&ts2);
    MPRINTF("500 ms time delay calibrating result           = %d %d\r\n", r1, r2);

    Get_hw_timestump(&ts1);
    DELAY_ms(500);
    Get_hw_timestump(&ts2);
    r2 = (uint32_t)Hw_timestump_diff64_us(&ts1,&ts2);
    MPRINTF("500 ms time software delay calibrating result  = %d\r\n",r2);
  }

  {
    // Проверяем заполнение памяти
    uint8_t  *str_addr = (uint8_t *)0x00000000;
    uint8_t  *addr;
    uint8_t  *end_addr = (uint8_t *)0x002FFFFF;
    uint8_t  *end = 0;
    uint32_t  empt_bl_sz = 0;

    addr = str_addr;
    do
    {
      if (*addr != 0xFF)
      {
        if (empt_bl_sz > 256)
        {
          MPRINTF("Empty flash memory from %08X to %08X\r\n", (uint32_t)end - empt_bl_sz , (uint32_t)end);
        }
        empt_bl_sz = 0;
      }
      else
      {
        end = addr;
        empt_bl_sz++;
      }
      addr++;

    } while (addr <= end_addr);

    if (empt_bl_sz > 256)
    {
      MPRINTF("Empty flash memory from %08X to %08X\r\n", (uint32_t)end - empt_bl_sz , (uint32_t)end);
    }
  }


  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(1000)) == RES_OK)
    {

      switch (b)
      {
      case 'L':
      case 'l':
        Print_registered_interrupts();
        MPRINTF("\r\nPress any key to continue.\r\n");
        WAIT_CHAR(&b,  ms_to_ticks(100000));
        Print_System_info_header();
        break;
      case 'P':
      case 'p':
        Print_pins_state();
        MPRINTF("\r\nPress any key to continue.\r\n");
        WAIT_CHAR(&b,  ms_to_ticks(100000));
        Print_System_info_header();
        break;
      case 'm':
      case 'M':
        Print_ICU_Event_Link_Setting_Registers();
        MPRINTF("\r\nPress any key to continue.\r\n");
        WAIT_CHAR(&b,  ms_to_ticks(100000));
        Print_System_info_header();
        break;
      case VT100_ESC:
        return;
      }
    }
  } while (1);

}

