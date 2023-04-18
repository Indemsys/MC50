#ifndef NXD_DNS_CLIENT_CFG_H_

  #define NXD_DNS_CLIENT_CFG_H_
  #define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    //#define NX_DNS_CLIENT_CLEAR_QUEUE
    //#define NX_DNS_ENABLE_EXTENDED_RR_TYPES
  #define NX_DNS_CACHE_ENABLE
  #define NX_DNS_IP_GATEWAY_AND_DNS_SERVER
    //#define NX_DNS_PACKET_PAYLOAD_UNALIGNED                     (16 + sizeof(NX_IPV6_HEADER) + sizeof(NX_UDP_HEADER) + NX_DNS_MESSAGE_MAX)
  #define NX_DNS_PACKET_PAYLOAD_UNALIGNED                         (16 + sizeof(NX_IPV4_HEADER) + sizeof(NX_UDP_HEADER) + NX_DNS_MESSAGE_MAX)
  #define NX_DNS_TYPE_OF_SERVICE                                  NX_IP_NORMAL
  #define NX_DNS_FRAGMENT_OPTION                                  NX_DONT_FRAGMENT
  #define NX_DNS_TIME_TO_LIVE                                     128
  #define NX_DNS_MAX_SERVERS                                      5
  #define NX_DNS_MESSAGE_MAX                                      512
  #define NX_DNS_PACKET_POOL_SIZE                                 (16 * (NX_DNS_PACKET_PAYLOAD + sizeof(NX_PACKET)))
  #define NX_DNS_MAX_RETRIES                                      3
  #define NX_DNS_MAX_RETRANS_TIMEOUT                              (64 * NX_IP_PERIODIC_RATE)
  #define NX_DNS_PACKET_ALLOCATE_TIMEOUT                          (1 * NX_IP_PERIODIC_RATE)


#endif
