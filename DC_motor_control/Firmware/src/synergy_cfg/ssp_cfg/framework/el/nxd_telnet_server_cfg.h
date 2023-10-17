/* generated configuration header file - do not edit */
#ifndef NXD_TELNET_SERVER_CFG_H_
#define NXD_TELNET_SERVER_CFG_H_
#if (!1)
            #define NX_TELNET_SERVER_OPTION_DISABLE
            #endif
#if (1)
            #define NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
            #endif
#define NX_TELNET_SERVER_PRIORITY                           (16)
#define NX_TELNET_MAX_CLIENTS                               (1)
#define NX_TELNET_SERVER_WINDOW_SIZE                        (2048)
#define NX_TELNET_SERVER_TIMEOUT                            (10 * NX_IP_PERIODIC_RATE)
#define NX_TELNET_ACTIVITY_TIMEOUT                          (600)
#define NX_TELNET_TIMEOUT_PERIOD                            (60)
#define NX_TELNET_SERVER_PACKET_PAYLOAD                     (300)
#define NX_TELNET_SERVER_PACKET_POOL_SIZE                   (2048)
#endif /* NXD_TELNET_SERVER_CFG_H_ */
