#ifndef MC50_PINS_H
  #define MC50_PINS_H


typedef struct
{
    const char *name;
    volatile uint32_t *PFS;

    uint8_t           PSEL;  // The PSEL bits assign the peripheral function

    uint8_t           PCR;   // 0: Disable input pull-up
                             // 1: Enable input pull-up.

    uint8_t           ODR;   // 0 Open Drain output is disabled on the corresponding pin.
                             // 1 Open Drain output is enabled on the corresponding pin

    uint8_t           DSCR;  // Port Drive Capability
                             // 00: Low drive
                             // 01: Middle drive
                             // 10: Setting prohibited
                             // 11: High drive.

    uint8_t           EOFR;  // Event on Falling/Event on Rising
                             // 00: Don’t-care
                             // 01: Detect rising edge
                             // 10: Detect falling edge
                             // 11: Detect both edges

    uint8_t           ISEL;  // IRQ Input Enable
                             // 0: Do not use as IRQn input pin
                             // 1: Use as IRQn input pin.

    uint8_t           ASEL;  // Analog Input Enable
                             // 0: Do not use as analog pin
                             // 1: Use as analog pin.

    uint8_t           PORTM; // Port Mode Control
                             // 0: Use as general I/O pin
                             // 1: Use as I/O port for peripheral functions.

    uint8_t           dir;   // 0 Pin is configured as general purpose input, if configured for the GPIO function
                             // 1 Pin is configured for general purpose output, if configured for the GPIO function
    uint8_t           init;  // Init state
    const char *descr; // Description

} T_IO_pins_configuration;



  #define   PULLUP_DIS    0 // 0: Disable input pull-up
  #define   PULLUP__EN    1 // 1: Enable input pull-up.

  #define   OD_DIS        0 // 0 Open Drain output is disabled on the corresponding pin.
  #define   OD__EN        1 // 1 Open Drain output is enabled on the corresponding pin.

  #define   LO_DRV        0 // 00: Low drive
  #define   MI_DRV        1 // 01: Middle drive
  #define   NO_DRV        2 // 10: Setting prohibited
  #define   HI_DRV        3 // 11: High drive.

  #define   EV_DNC        0 // 00: Don’t-care
  #define   EV_DRE        1 // 01: Detect rising edge
  #define   EV_DFE        2 // 10: Detect falling edge
  #define   EV_DBE        3 // 11: Detect both edges

  #define   IRQ_DIS       0 // 0: Do not use as IRQn input pin
  #define   IRQ__EN       1 // 1: Use as IRQn input pin.

  #define   ANAL_DIS      0 // 0: Do not use as IRQn input pin
  #define   ANAL__EN      1 // 1: Use as IRQn input pin.

  #define   PORT__IO      0 // 0: Use as general I/O pin
  #define   PORT_PER      1 // 1: Use as I/O port for peripheral functions.

// These bits select the peripheral function
  #define   PSEL_00 0
  #define   PSEL_01 1
  #define   PSEL_02 2
  #define   PSEL_03 3
  #define   PSEL_04 4
  #define   PSEL_05 5
  #define   PSEL_06 6
  #define   PSEL_07 7
  #define   PSEL_09 9
  #define   PSEL_10 10
  #define   PSEL_11 11
  #define   PSEL_12 12
  #define   PSEL_16 16
  #define   PSEL_17 17
  #define   PSEL_18 18
  #define   PSEL_19 19
  #define   PSEL_20 20
  #define   PSEL_21 21
  #define   PSEL_22 22
  #define   PSEL_23 23
  #define   PSEL_25 25

  #define   GP_INP        0 // 0 Pin is configured as general purpose input, if configured for the GPIO function
  #define   GP_OUT        1 // 1 Pin is configured for general purpose output, if configured for the GPIO function


typedef struct
{
    uint32_t  PODR       :  1; // Port Output Data
    uint32_t  PIDR       :  1; // Port Input Data
    uint32_t  PDR        :  1; // Port Direction
    uint32_t             :  1; //
    uint32_t  PCR        :  1; // Pull-up Control
    uint32_t             :  1; //
    uint32_t  NCODR      :  1; // N-Channel Open Drain Control
    uint32_t  PCODR      :  1; // P-Channel Open Drain Control
    uint32_t             :  2; //
    uint32_t  DSCR       :  2; // Drive Strength Control Register
    uint32_t  EOFR       :  2; //
    uint32_t  ISEL       :  1; // IRQ input enable
    uint32_t  ASEL       :  1; // Analog Input enable
    uint32_t  PMR        :  1; // Port Mode Control
    uint32_t             :  7; //
    uint32_t  PSEL       :  5; // Port Function SelectThese bits select the peripheral function. For individual pin functions, see the MPC table
}
T_reg_PFS;



extern const T_IO_pins_configuration MC50_pins_cfg[];


void      MC50_board_pins_init(void);
uint32_t  MC50_get_pin_count(void);
void      MC50_get_pin_conf_str(uint32_t pin_num, char *dstr);


uint8_t   Get_smpl_enc_a (void);
uint8_t   Get_smpl_enc_b (void);
uint8_t   Get_smpl_enc_sw(void);


#endif
