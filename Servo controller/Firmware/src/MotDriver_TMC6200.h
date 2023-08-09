#ifndef MOTDRIVER_TMC6200_H
  #define MOTDRIVER_TMC6200_H

  #define TMC6200_AMPLIFICATION_5   0
  #define TMC6200_AMPLIFICATION_10  1
  #define TMC6200_AMPLIFICATION_20  3

typedef __packed struct
{
  uint32_t  disable       : 1; // 1: Disable driver (e.g. for Resetting of short condition)
  uint32_t  singleline    : 1; // 0: Individual signals L+H. 1: H-Input is control signal, L-Input is Enable
  uint32_t  faultdirect   : 1; // 0: Fault output active when at least one bridge isshut down continuously due to overcurrent or overtemperature. 1: Fault output shows each protective action of the overcurrent shutdown
  uint32_t  unused        : 1;
  uint32_t  amplification : 2; // amplification: 0: Current amplification: *5, 1: Current amplification: *10, 2: Current amplification: *10, 3: Current amplification: *20
  uint32_t  amplifier_off : 1; // 0: Current sense amplifiers on
  uint32_t  test_mode     : 1; // Not for user, set to 0 for normal operation!
  uint32_t  unused2       : 24;
}
T_TMC6200_GCONF;

typedef __packed struct
{
  uint32_t  reset         : 1;  // 1: Indicates that the IC has been reset. All registers have been cleared to reset values.
  uint32_t  drv_otpw      : 1;  // 1: Indicates, that the driver temperature has exceeded overtemperature prewarning-level. No action is taken. This flag is latched.
  uint32_t  drv_ot        : 1;  // 1: Indicates, that the driver has been shut down due to overtemperature. This flag can only be cleared when the temperature is below the limit again. It is latched for information
  uint32_t  uv_cp         : 1;  // 1: Indicates an undervoltage on the charge pump. The driver is disabled during undervoltage. This flag is latched for information.
  uint32_t  shortdet_u    : 1;  // 1: U short counter has triggered at least once
  uint32_t  s2gu          : 1;  // 1: Short to GND detected on phase U. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsu         : 1;  // 1: Short to VS detected on phase U. The driver becomes disabled until flag becomes cleared.
  uint32_t  unused        : 1;  //
  uint32_t  shortdet_v    : 1;  // 1: V short counter has triggered at least once
  uint32_t  s2gv          : 1;  // 1: Short to GND detected on phase V. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsv         : 1;  // 1: Short to VS detected on phase V. The driver becomes disabled until flag becomes cleared.
  uint32_t  unused2       : 1;  //
  uint32_t  shortdet_w    : 1;  // 1: short counter has triggered at least once.
  uint32_t  s2gw          : 1;  // 1: Short to GND detected on phase W. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsw         : 1;  // 1: Short to VS detected on phase W. The driver becomes disabled until flag becomes cleared.
  uint32_t  unused3       : 17;

}
T_TMC6200_GSTAT;

typedef __packed struct
{
  uint32_t  ul            : 1;  //
  uint32_t  uh            : 1;  //
  uint32_t  vl            : 1;  //
  uint32_t  vh            : 1;  //
  uint32_t  wl            : 1;  //
  uint32_t  wh            : 1;  //
  uint32_t  drv_en        : 1;  //
  uint32_t  nul           : 1;  //
  uint32_t  otpw          : 1;  //
  uint32_t  ot136         : 1;  //
  uint32_t  ot143         : 1;  //
  uint32_t  ot150         : 1;  //
  uint32_t  ver           : 20; //

}
T_TMC6200_INPUT;


typedef __packed struct
{
  uint32_t BBMCLKS        : 5;
  uint32_t unused         : 11;
  uint32_t otselect       : 2;  // 00: 150°C (default), 01: 143°C, 10: 136°C, 11: 120°C (not recommended, no hysteresis)
  uint32_t drvstrength    : 2;  // 00: weak, 01: weak+TC (medium above OTPW level), 10: medium, 11: strong
  uint32_t unused2        : 12;

}
T_TMC6200_DRV_CONF;


typedef __packed struct
{
  uint32_t S2VS_LEVEL       : 4; // Short to VS detector level for lowside FETs. Checks for voltage drop in LS MOSFET and sense resistor.1 (highest sensitivity) … 15 (lowest sensitivity)
  uint32_t unused0          : 4;
  uint32_t S2G_LEVEL        : 4; // Short to GND detector level for highside FETs. Checks for voltage drop on high side MOSFET 2 (highest sensitivity) … 15 (lowest sensitivity)
  uint32_t unused1          : 4;
  uint32_t SHORTFILTER      : 2; // 0 (lowest, 100ns), 1 (1µs), 2 (2µs) 3 (3µs)
  uint32_t unused2          : 2;
  uint32_t shortdelay       : 1; // 0=750ns: normal, 1=1500ns: high. The short detection delay shall cover the bridge switching time. 0 will work for most applications.
  uint32_t unused3          : 3;
  uint32_t RETRY            : 2; // Number of retries for short detection. 0: Half bridge disabled after first short detection
  uint32_t unused4          : 2;
  uint32_t protect_parallel : 1; // 0: Only the detected half bridge driver becomes shut down upon final short detection. 1: All half bridge drivers become shut down upon final short detection
  uint32_t disable_S2G      : 1; // 0: Short to GND (HS) protection enabled. 1: No short to GND protection
  uint32_t disable_S2VS     : 1; // 0: Short to VS (LS) protection enabled. 1: No short to VS protection
}
T_TMC6200_SHORT_CONF;

typedef struct
{
    union
    {
        T_TMC6200_GCONF     gconf;
        uint32_t            u_gconf;
    };
    union
    {
        T_TMC6200_GSTAT     gstat;
        uint32_t            u_gstat;
    };
    union
    {
        T_TMC6200_INPUT     inputs;
        uint32_t            u_inputs;
    };
    union
    {
        T_TMC6200_DRV_CONF  drv_conf;
        uint32_t            u_drv_conf;
    };
    union
    {
        T_TMC6200_SHORT_CONF short_conf;
        uint32_t             u_short_conf;
    };
}
T_TMC6200_cbl;

  #define TMC6200_GCONF_ADDR   0
  #define TMC6200_GSTAT_ADDR   1
  #define TMC6200_INPUT_ADDR   4
  #define TMC6200_DRVCF_ADDR   10
  #define TMC6200_SHORT_CONF   9


typedef  struct
{
    union
    {
        struct
        {
            uint32_t  UF         :  1;  // Input Phase Soft Setting WFThis bit sets the input phase by the software settings.This bit setting is valid when the OPSCR.FB bit = 1.
            uint32_t  VF         :  1;  // Input Phase Soft Setting VFThis bit sets the input phase by the software settings.This bit setting is valid when the OPSCR.FB bit = 1.
            uint32_t  WF         :  1;  // Input Phase Soft Setting UFThis bit sets the input phase by the software settings.This bit setting is valid when the OPSCR.FB bit = 1.
            uint32_t             :  1;  //
            uint32_t  U          :  1;  // Input U-Phase MonitorThis bit monitors the state of the input phase.OPSCR.FB=0:External input monitoring by PCLKOPSCR.FB=1:Software settings (UF/VF/WF)
            uint32_t  V          :  1;  // Input V-Phase MonitorThis bit monitors the state of the input phase.OPSCR.FB=0:External input monitoring by PCLKOPSCR.FB=1:Software settings (UF/VF/WF)
            uint32_t  W          :  1;  // Input W-Phase MonitorThis bit monitors the state of the input phase.OPSCR.FB=0:External input monitoring by PCLKOPSCR.FB=1:Software settings (UF/VF/WF)
            uint32_t             :  1;  //
            uint32_t  EN         :  1;  // Enable-Phase Output Control
            uint32_t             :  7;  //
            uint32_t  FB         :  1;  // External Feedback Signal EnableThis bit selects the input phase from the software settings and external input.
            uint32_t  P          :  1;  // Positive-Phase Output (P) Control
            uint32_t  N          :  1;  // Negative-Phase Output (N) Control
            uint32_t  INV        :  1;  // Invert-Phase Output Control
            uint32_t  RV         :  1;  // Output phase rotation direction reversal
            uint32_t  ALIGN      :  1;  // Input phase alignment
            uint32_t             :  2;  //
            uint32_t  GRP        :  2;  // Output disabled source selection
            uint32_t  GODF       :  1;  // Group output disable function
            uint32_t             :  2;  //
            uint32_t  NFEN       :  1;  // External Input Noise Filter Enable
            uint32_t  NFCS       :  2;  // External Input Noise Filter Clock selectionNoise filter sampling clock setting of the external input.
        };
        uint32_t w;
    };
} T_OPSCR;

typedef struct
{

  uint32_t  reset       ;  // 1: Indicates that the IC has been reset. All registers have been cleared to reset values.
  uint32_t  drv_otpw    ;  // 1: Indicates, that the driver temperature has exceeded overtemperature prewarning-level. No action is taken. This flag is latched.
  uint32_t  drv_ot      ;  // 1: Indicates, that the driver has been shut down due to overtemperature. This flag can only be cleared when the temperature is below the limit again. It is latched for information
  uint32_t  uv_cp       ;  // 1: Indicates an undervoltage on the charge pump. The driver is disabled during undervoltage. This flag is latched for information.
  uint32_t  shortdet_u  ;  // 1: U short counter has triggered at least once
  uint32_t  s2gu        ;  // 1: Short to GND detected on phase U. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsu       ;  // 1: Short to VS detected on phase U. The driver becomes disabled until flag becomes cleared.
  uint32_t  shortdet_v  ;  // 1: V short counter has triggered at least once
  uint32_t  s2gv        ;  // 1: Short to GND detected on phase V. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsv       ;  // 1: Short to VS detected on phase V. The driver becomes disabled until flag becomes cleared.
  uint32_t  shortdet_w  ;  // 1: short counter has triggered at least once.
  uint32_t  s2gw        ;  // 1: Short to GND detected on phase W. The driver becomes disabled until flag becomes cleared.
  uint32_t  s2vsw       ;  // 1: Short to VS detected on phase W. The driver becomes disabled until flag becomes cleared.
}
T_TMC6200_counters;



extern T_TMC6200_cbl      tmc6200_cbl;
extern uint32_t           tmc6200_comm_err_cnt;
extern T_TMC6200_counters tmc6200_cnts;


void     TMC6200_init(void);
uint32_t TMC6200_read(uint8_t regaddr, uint32_t *word);
uint32_t TMC6200_write(uint8_t regaddr, uint32_t word);
uint32_t Thread_TMC6200_create(void);
uint32_t TMC6200_set_clearing_fault(void);
uint32_t TMC6200_set_clearing_done(void);
uint32_t TMC6200_wait_clearing_done(void);
void     TMC6200_init_fault_interrut(void);

#endif // MOTDRIVER_TMC6200_H



