#ifndef CAN0_H
  #define CAN0_H

  #define CAN_SPEED_555555     0
  #define CAN_SPEED_100000     1



  #define RX_MESSAGES_RING_SZ  128
  #define TX_MESSAGES_RING_SZ  128


typedef struct
{
  uint32_t can_id;
  uint32_t len;
  uint8_t  data[8];

} T_can_msg;



typedef struct
{
    uint32_t bus_error_cnt;
    uint32_t error_warning_cnt;     // EWIF flag (Error-Warning Detect Flag). The EWIF flag is set to 1 when the value of the receive error counter (REC) or transmit error counter (TEC) exceeds 95
    uint32_t error_passive_cnt;     // EPIF flag (Error-Passive Detect Flag). The EPIF flag is set to 1 when the CAN error state becomes error-passive, when the receive error counter (REC) or transmit error counter (TEC) value exceeds 127
    uint32_t bus_off_entry_cnt;     // BOEIF flag (Bus-Off Entry Detect Flag). The BOEIF flag is set to 1 when the CAN error state becomes bus-off, when the transmit error counter (TEC) value exceeds 255
    uint32_t bus_off_recovery_cnt;  // BORIF flag (Bus-Off Recovery Detect Flag). The BORIF flag is set to 1 when the CAN module recovers from the bus-off state normally by detecting 11 consecutive recessive bits 128 times in some conditions
    uint32_t receive_overrun_cnt;
    uint32_t overload_frame_cnt;
    uint32_t bus_lock_cnt;

    uint32_t stuff_error_cnt;
    uint32_t form_error_cnt;
    uint32_t ack_error_cnt;
    uint32_t crc_error_cnt;
    uint32_t bit_error_recessive_cnt;
    uint32_t bit_error_dominant_cnt;
    uint32_t ack_delimiter_cnt;

} T_can_error_counters;


typedef union
{
    uint16_t str;
    struct
    {
        uint16_t  NDST  :  1;   // NEWDATA Status Flag
        uint16_t  SDST  :  1;   // SENTDATA Status Flag
        uint16_t  RFST  :  1;   // Receive FIFO Status Flag
        uint16_t  TFST  :  1;   // Transmit FIFO Status Flag
        uint16_t  NMLST :  1;   // Normal Mailbox Message Lost Status Flag
        uint16_t  FMLST :  1;   // FIFO Mailbox Message Lost Status Flag
        uint16_t  TABST :  1;   // Transmission Abort Status Flag
        uint16_t  EST   :  1;   // Error Status Flag
        uint16_t  RSTST :  1;   // CAN Reset Status Flag
        uint16_t  HLTST :  1;   // CAN Halt Status Flag
        uint16_t  SLPST :  1;   // CAN Sleep Status Flag
        uint16_t  EPST  :  1;   // Error-Passive Status Flag
        uint16_t  BOST  :  1;   // Bus-Off Status Flag
        uint16_t  TRMST :  1;   // Transmit Status Flag (transmitter)
        uint16_t  RECST :  1;   // Receive Status Flag (receiver)
    } str_b;
} T_can_str;



uint32_t CAN_init(uint8_t speed_code);
uint32_t CAN_reinit(uint8_t speed_code);
uint32_t Thread_CAN_create(void);
uint32_t CAN0_post_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len);
uint8_t  Is_CAN0_in_error_state(void);
uint8_t  Is_CAN0_task_ready(void);

#endif // SB200M2BU1_CAN_H



