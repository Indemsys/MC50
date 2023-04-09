/* generated configuration header file - do not edit */
#ifndef NXD_MDNS_CFG_H_
#define NXD_MDNS_CFG_H_
#if (0)
#define NX_MDNS_DISABLE_SERVER
#endif
#if (!1)
            #define NX_MDNS_DISABLE_CLIENT
            #endif
#if (1)
#define NX_MDNS_ENABLE_ADDRESS_CHECK
#endif
#if (1)
#define NX_MDNS_ENABLE_CLIENT_POOF
#endif
#if (1)
#define NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
#endif
#if (0)
            #define NX_MDNS_ENABLE_IPv6
            #endif
#define NX_MDNS_IPV6_ADDRESS_COUNT                              2
#define NX_MDNS_HOST_NAME_MAX                                   64
#define NX_MDNS_SERVICE_NAME_MAX                                64
#define NX_MDNS_DOMAIN_NAME_MAX                                 16
#define NX_MDNS_CONFLICT_COUNT                                  8
#define NX_MDNS_RR_TTL_HOST                                     120
#define NX_MDNS_RR_TTL_OTHER                                    4500
#define NX_MDNS_PROBING_TIMER_COUNT                             25
#define NX_MDNS_ANNOUNCING_TIMER_COUNT                          25
#define NX_MDNS_GOODBYE_TIMER_COUNT                             25
#define NX_MDNS_QUERY_MIN_TIMER_COUNT                           100
#define NX_MDNS_QUERY_MAX_TIMER_COUNT                           360000
#define NX_MDNS_QUERY_DELAY_MIN                                 2
#define NX_MDNS_QUERY_DELAY_RANGE                               10
#define NX_MDNS_RESPONSE_INTERVAL                               100
#define NX_MDNS_RESPONSE_PROBING_INTERVAL                       25
#define NX_MDNS_RESPONSE_UNIQUE_DELAY                           1
#define NX_MDNS_RESPONSE_SHARED_DELAY_MIN                       2
#define NX_MDNS_RESPONSE_SHARED_DELAY_RANGE                     10
#define NX_MDNS_RESPONSE_TC_DELAY_MIN                           40
#define NX_MDNS_RESPONSE_TC_DELAY_RANGE                         10
#define NX_MDNS_TIMER_COUNT_RANGE                               12
#define NX_MDNS_PROBING_RETRANSMIT_COUNT                        3
#define NX_MDNS_GOODBYE_RETRANSMIT_COUNT                        1
#define NX_MDNS_POOF_MIN_COUNT                                  2
#define NX_MDNS_POOF_TIME_COUNT                                 1000
#define NX_MDNS_RR_DELETE_DELAY_TIMER_COUNT                     100
#endif /* NXD_MDNS_CFG_H_ */
