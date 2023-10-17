/* generated configuration header file - do not edit */
#ifndef NXD_HTTP_SERVER_CFG_H_
#define NXD_HTTP_SERVER_CFG_H_
#if (!1)
            #define NX_HTTP_NO_FILEX
            #endif
#if (0)
            #define NX_HTTP_MULTIPART_ENABLE
            #endif
#define NX_HTTP_SERVER_PRIORITY                     16
#define NX_HTTP_SERVER_WINDOW_SIZE                  2048
#define NX_HTTP_SERVER_TIMEOUT                      (10 * NX_IP_PERIODIC_RATE)
#define NX_HTTP_SERVER_TIMEOUT_ACCEPT               (10 * NX_IP_PERIODIC_RATE)
#define NX_HTTP_SERVER_TIMEOUT_DISCONNECT           (10 * NX_IP_PERIODIC_RATE)
#define NX_HTTP_SERVER_TIMEOUT_RECEIVE              (10 * NX_IP_PERIODIC_RATE)
#define NX_HTTP_SERVER_TIMEOUT_SEND                 (10 * NX_IP_PERIODIC_RATE)
#define NX_HTTP_MAX_HEADER_FIELD                    256
#define NX_HTTP_SERVER_MAX_PENDING                  5
#define NX_HTTP_MAX_NAME                            20
#define NX_HTTP_MAX_PASSWORD                        20
#define NX_HTTP_SERVER_MIN_PACKET_SIZE              600
#define NX_HTTP_SERVER_THREAD_TIME_SLICE            2
#define NX_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH         20
#define NX_HTTP_SERVER_RETRY_SECONDS                2
#define NX_HTTP_SERVER_RETRY_MAX                    10
#define NX_HTTP_SERVER_RETRY_SHIFT                  1
#endif /* NXD_HTTP_SERVER_CFG_H_ */
