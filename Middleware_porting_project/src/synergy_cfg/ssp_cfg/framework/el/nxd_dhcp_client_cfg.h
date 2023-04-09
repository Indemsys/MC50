/* generated configuration header file - do not edit */
#ifndef NXD_DHCP_CLIENT_CFG_H_
#define NXD_DHCP_CLIENT_CFG_H_
#if (0)
            #define NX_DHCP_ENABLE_BOOTP
            #endif
#if 0
            #define NX_DHCP_CLIENT_RESTORE_STATE
            #endif
#if (0)
            #define NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL
            #endif
#if (0)
            #define NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION
            #endif
#if (0)
            #define NX_DHCP_CLIENT_SEND_ARP_PROBE
            #define NX_DHCP_ARP_PROBE_WAIT                       (1 * NX_IP_PERIODIC_RATE)
            #define NX_DHCP_ARP_PROBE_MIN                        (1 * NX_IP_PERIODIC_RATE)
            #define NX_DHCP_ARP_PROBE_MAX                        (2 * NX_IP_PERIODIC_RATE)
            #define NX_DHCP_ARP_PROBE_NUM                        2
            #endif
#define NX_DHCP_THREAD_PRIORITY                      3
#define NX_DHCP_THREAD_STACK_SIZE                    1024
#define NX_DHCP_TIME_INTERVAL                        (1 * NX_IP_PERIODIC_RATE)
#define NX_DHCP_MAX_RETRANS_TIMEOUT                  (64 * NX_IP_PERIODIC_RATE)
#define NX_DHCP_MIN_RENEW_TIMEOUT                    (60 * NX_IP_PERIODIC_RATE)
#define NX_DHCP_MIN_RETRANS_TIMEOUT                  (4 * NX_IP_PERIODIC_RATE)
#define NX_DHCP_PACKET_PAYLOAD                       592
#define NX_DHCP_PACKET_POOL_SIZE                     (5 *  ( NX_DHCP_PACKET_PAYLOAD + sizeof(NX_PACKET)))
#define NX_DHCP_OPTIONS_BUFFER_SIZE                  312
#define NX_DHCP_CLIENT_MAX_RECORDS                   1
#define NX_DHCP_RESTART_WAIT                         (10 * NX_IP_PERIODIC_RATE)
#endif /* NXD_DHCP_CLIENT_CFG_H_ */
