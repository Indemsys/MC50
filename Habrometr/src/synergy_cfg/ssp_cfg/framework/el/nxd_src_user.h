/* generated configuration header file - do not edit */
#ifndef NXD_SRC_USER_H_
  #define NXD_SRC_USER_H_
  #if (!1)
    #define NX_DISABLE_ERROR_CHECKING
  #endif
  #if (0)
    #define NX_ENABLE_IP_STATIC_ROUTING
  #endif
  #if (+0)
    #define NX_PHYSICAL_HEADER ()
  #endif
  #if (+0)
    #define NX_PHYSICAL_TRAILER ()
  #endif
  #if (0)
    #define NX_ARP_DEFEND_BY_REPLY
  #endif
  #if (0)
    #define NX_ARP_MAC_CHANGE_NOTIFICATION_ENABLE
  #endif
  #if (+0)
    #define NX_ARP_EXPIRATION_RATE ()
  #endif
  #if (+0)
    #define NX_ARP_UPDATE_RATE ()
  #endif
  #if (+0)
    #define NX_TCP_ACK_TIMER_RATE ()
  #endif
  #if (+0)
    #define NX_TCP_FAST_TIMER_RATE ()
  #endif
  #if (+0)
    #define NX_TCP_TRANSMIT_TIMER_RATE ()
  #endif
  #if (+0)
    #define NX_TCP_KEEPALIVE_INITIAL ()
  #endif
  #if (+0)
    #define NX_TCP_KEEPALIVE_RETRY ()
  #endif
  #if (+0)
    #define NX_ARP_MAXIMUM_RETRIES ()
  #endif
  #if (+0)
    #define NX_ARP_MAX_QUEUE_DEPTH ()
  #endif
  #if (!1)
    #define NX_ARP_DISABLE_AUTO_ARP_ENTRY
  #endif
  #if (0)
    #define NX_MAX_MULTICAST_GROUPS ()
  #endif
  #if (+0)
    #define NX_MAX_LISTEN_REQUESTS ()
  #endif
  #if (0)
    #define NX_TCP_ENABLE_KEEPALIVE
  #endif
  #if (1)
    #define NX_TCP_ENABLE_WINDOW_SCALING
  #endif
  #if (1)
    #define NX_TCP_IMMEDIATE_ACK
  #endif
  #if (+0)
    #define NX_TCP_ACK_EVERY_N_PACKETS ()
  #endif

/* Automatically define NX_TCP_ACK_EVERY_N_PACKETS to 1 if NX_TCP_IMMEDIATE_ACK is defined.
 This is needed for backward compatibility. */
  #if (defined(NX_TCP_IMMEDIATE_ACK) && !defined(NX_TCP_ACK_EVERY_N_PACKETS))
    #define NX_TCP_ACK_EVERY_N_PACKETS 1
  #endif

  #if (+0)
    #define NX_TCP_MAXIMUM_RETRIES ()
  #endif
  #if (+0)
    #define NX_TCP_MAXIMUM_TX_QUEUE ()
  #endif
  #if (+0)
    #define NX_TCP_RETRY_SHIFT ()
  #endif
  #if (+0)
    #define NX_TCP_KEEPALIVE_RETRIES ()
  #endif
  #if (0)
    #define NX_DRIVER_DEFERRED_PROCESSING
  #endif
  #if (!1)
    #define NX_DISABLE_LOOPBACK_INTERFACE
  #endif
  #if (2+0)
    #define NX_MAX_PHYSICAL_INTERFACES (1)
  #endif
  #if (!1)
    #define NX_DISABLE_FRAGMENTATION
  #endif
  #if (!1)
    #define NX_DISABLE_IP_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_IP_TX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_TCP_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_TCP_TX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_UDP_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_UDP_TX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_RESET_DISCONNECT
  #endif
  #if (!1)
    #define NX_DISABLE_RX_SIZE_CHECKING
  #endif
  #if (!1)
    #define NX_DISABLE_ARP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_IP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_ICMP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_IGMPV2
  #endif
  #if (!1)
    #define NX_DISABLE_IGMP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_PACKET_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_RARP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_TCP_INFO
  #endif
  #if (!1)
    #define NX_DISABLE_UDP_INFO
  #endif
  #if (1)
    #define NX_ENABLE_EXTENDED_NOTIFY_SUPPORT
  #endif
  #if (1)
    #define NX_NAT_ENABLE
  #endif
  #if (+0)
    #define NX_PACKET_HEADER_PAD
    #define NX_PACKET_HEADER_PAD_SIZE ()
  #endif
  #if (+0)
    #define NX_TCP_MSS_CHECKING_ENABLED
    #define NX_TCP_MSS_MINIMUM ()
  #endif
  #if (0)
    #define NX_ENABLE_SOURCE_ADDRESS_CHECK
  #endif
  #if (+0)
    #define NX_ARP_DEFEND_INTERVAL ()
  #endif
  #if (+0)
    #define NX_TCP_MAX_OUT_OF_ORDER_PACKETS ()
  #endif
  #define NX_DISABLE_INCLUDE_SOURCE_CODE /* Disable C include C */

// IPv6 Options
  #if (!1) /* NetX Duo IPV6 support */
    #define NX_DISABLE_IPV6
  #endif
  #if (0)
    #define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
  #endif
  #if (0)
    #define NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
  #endif
  #if (+0)
    #define NX_PATH_MTU_INCREASE_WAIT_INTERVAL
  #endif
  #if (+0)
    #define NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE
  #endif
  #if (+0)
    #define NX_IPV6_DESTINATION_TABLE_SIZE
  #endif
  #if (+0)
    #define NX_IP_MAX_REASSEMBLY_TIME
  #endif
  #if (+0)
    #define NX_IPV4_MAX_REASSEMBLY_TIME
  #endif
  #if (+0)
    #define NX_IPV6_MAX_REASSEMBLY_TIME
  #endif
  #if (0)
    #define NX_ENABLE_IPV6_MULTICAST
  #endif
  #if (+0)
    #define NX_IPV6_PREFIX_LIST_TABLE_SIZE
  #endif
  #if (0)
    #define NX_IPV6_STATELESS_AUTOCONFIG_CONTROL
  #endif
  #if (+0)
    #define NX_MAX_IPV6_ADDRESSES
  #endif
// Neighbor Cache Configuration Options
  #if (+0)
    #define NX_DELAY_FIRST_PROBE_TIME
  #endif
  #if (!1)
    #define NX_DISABLE_IPV6_DAD
  #endif
  #if (+0)
    #define NX_IPV6_DAD_TRANSMITS
  #endif
  #if (!1)
    #define NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
  #endif
  #if (+0)
    #define NX_IPV6_NEIGHBOR_CACHE_SIZE
  #endif
  #if (+0)
    #define NX_MAX_MULTICAST_SOLICIT
  #endif
  #if (+0)
    #define NX_MAX_UNICAST_SOLICIT
  #endif
  #if (+0)
    #define NX_ND_MAX_QUEUE_DEPTH
  #endif
  #if (+0)
    #define NX_REACHABLE_TIME
  #endif
  #if (+0)
    #define NX_RETRANS_TIMER
  #endif
// Miscellaneous ICMPv6 Configuration Options
  #if (!1)
    #define NX_DISABLE_ICMPV6_ERROR_MESSAGE
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV6_REDIRECT_PROCESS
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
  #endif
  #if (+0)
    #define NX_ICMPV6_MAX_RTR_SOLICITATIONS
  #endif
  #if (+0)
    #define NX_ICMPV6_RTR_SOLICITATION_INTERVAL
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV4_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV6_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_ICMP_RX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV4_TX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_ICMPV6_TX_CHECKSUM
  #endif
  #if (!1)
    #define NX_DISABLE_ICMP_TX_CHECKSUM
  #endif
  #if (+0)
    #define NX_PACKET_ALIGNMENT
  #endif
  #if (0)
    #define NX_ENABLE_INTERFACE_CAPABILITY
  #endif
  #if (0)
    #define NX_ENABLE_TCPIP_OFFLOAD
  #endif
#endif /* NXD_SRC_USER_H_ */
