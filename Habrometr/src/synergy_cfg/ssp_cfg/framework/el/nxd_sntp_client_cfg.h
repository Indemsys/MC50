#ifndef NXD_SNTP_CLIENT_CFG_H_

  #define NXD_SNTP_CLIENT_CFG_H_
  #define NX_SNTP_CLIENT_THREAD_STACK_SIZE           (1024)
  #define NX_SNTP_CLIENT_THREAD_TIME_SLICE           (TX_NO_TIME_SLICE)
  #define NX_SNTP_CLIENT_THREAD_PRIORITY             (2)
  #define NX_SNTP_CLIENT_PREEMPTION_THRESHOLD         NX_SNTP_CLIENT_THREAD_PRIORITY
  #define NX_SNTP_CLIENT_UDP_SOCKET_NAME             "SNTP Client socket"
  #define NX_SNTP_CLIENT_UDP_PORT                    (123)
  #define NX_SNTP_SERVER_UDP_PORT                    (123)
  #define NX_SNTP_CLIENT_TIME_TO_LIVE                (128)
  #define NX_SNTP_CLIENT_MAX_QUEUE_DEPTH             (5)
  #define NX_SNTP_CLIENT_PACKET_TIMEOUT              (1 * NX_IP_PERIODIC_RATE)
  #define NX_SNTP_CLIENT_NTP_VERSION                 (3)
  #define NX_SNTP_CLIENT_MIN_NTP_VERSION             (3)
  #define NX_SNTP_CLIENT_MIN_SERVER_STRATUM          (2)
  #define NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT         (10)
  #define NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT         (10800000)
  #define NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP   (NX_TRUE)
  #define NX_SNTP_CLIENT_MAX_TIME_LAPSE              (60)
  #define NX_SNTP_UPDATE_TIMEOUT_INTERVAL            (1)
  #define NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL       (30)
  #define NX_SNTP_CLIENT_EXP_BACKOFF_RATE            (2)
  #define NX_SNTP_CLIENT_RTT_REQUIRED                (NX_FALSE)
  #define NX_SNTP_CLIENT_MAX_ROOT_DISPERSION         (50000)
  #define NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT        (3)
  #define NX_SNTP_CLIENT_RANDOMIZE_ON_STARTUP        (NX_FALSE)
  #define NX_SNTP_CLIENT_SLEEP_INTERVAL              (1)
  #define NX_SNTP_CURRENT_YEAR                       (2016)

#endif
