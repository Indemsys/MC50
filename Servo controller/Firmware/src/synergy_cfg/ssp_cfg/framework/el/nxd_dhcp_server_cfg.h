/* generated configuration header file - do not edit */
#ifndef NXD_DHCP_SERVER_CFG_H_
#define NXD_DHCP_SERVER_CFG_H_
#define NX_DHCP_SERVER_THREAD_PRIORITY             (1)
#define NX_DHCP_PACKET_ALLOCATE_TIMEOUT            (2 * NX_IP_PERIODIC_RATE)
#define NX_DHCP_FAST_PERIODIC_TIME_INTERVAL        (10)
#define NX_DHCP_CLIENT_SESSION_TIMEOUT             (10 * NX_DHCP_FAST_PERIODIC_TIME_INTERVAL)
#define NX_DHCP_DEFAULT_LEASE_TIME                 (0xFFFFFFFF)
#define NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL        (1000)
#define NX_DHCP_CLIENT_OPTIONS_MAX                 (12)
#define NX_DHCP_OPTIONAL_SERVER_OPTION_LIST        "1 3 6"
#define NX_DHCP_OPTIONAL_SERVER_OPTION_SIZE        (3)
#define NX_DHCP_SERVER_HOSTNAME_MAX                (32)
#define NX_DHCP_CLIENT_HOSTNAME_MAX                (32)
#define NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE           20
#define NX_DHCP_CLIENT_RECORD_TABLE_SIZE           50
#define NX_BOOT_BUFFER_SIZE                        548
#endif /* NXD_DHCP_SERVER_CFG_H_ */
