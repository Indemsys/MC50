/* generated configuration header file - do not edit */
#ifndef NXD_PPP_CFG_H_
#define NXD_PPP_CFG_H_
#if(!1)
            #define NX_PPP_DISABLE_CHAP
            #endif
#if(!1)
            #define NX_PPP_DISABLE_PAP
            #endif
#if(!1)
            #define NX_PPP_DNS_OPTION_DISABLE
            #endif
#define NX_PPP_DNS_ADDRESS_MAX_RETRIES          (2)
#define NX_PPP_BASE_TIMEOUT                     (1 * NX_IP_PERIODIC_RATE )
#define NX_PPP_TIMEOUT                          (4 * NX_IP_PERIODIC_RATE )
#define NX_PPP_RECEIVE_TIMEOUTS                 (4)
#define NX_PPP_PROTOCOL_TIMEOUT                 (4)
#define NX_PPP_MAX_LCP_PROTOCOL_RETRIES         (20)
#define NX_PPP_MAX_PAP_PROTOCOL_RETRIES         (20)
#define NX_PPP_MAX_CHAP_PROTOCOL_RETRIES        (20)
#define NX_PPP_MAX_IPCP_PROTOCOL_RETRIES        (20)
#endif /* NXD_PPP_CFG_H_ */
