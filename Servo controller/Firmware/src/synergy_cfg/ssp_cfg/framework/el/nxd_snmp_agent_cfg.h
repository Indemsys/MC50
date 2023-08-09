/* generated configuration header file - do not edit */
#ifndef NXD_SNMP_AGENT_CFG_H_
#define NXD_SNMP_AGENT_CFG_H_
/* Module specific overrides */
#define NX_SNMP_AGENT_THREAD_STACK_SIZE           (4096)
#define NX_SNMP_AGENT_PRIORITY                    (16)
#define NX_SNMP_TYPE_OF_SERVICE                   NX_IP_NORMAL
#define NX_SNMP_FRAGMENT_OPTION                   NX_DONT_FRAGMENT
#define NX_SNMP_TIME_TO_LIVE                      (128)
#define NX_SNMP_AGENT_TIMEOUT                     (100)
#define NX_SNMP_MAX_OCTET_STRING                  (255)
#define NX_SNMP_MAX_CONTEXT_STRING                (32)
#define NX_SNMP_MAX_USER_NAME                     (64)
#define NX_SNMP_MAX_SECURITY_KEY                  (64)
#define NX_SNMP_PACKET_SIZE                       (560)
#define NX_SNMP_AGENT_PORT                        (161)
#define NX_SNMP_MANAGER_TRAP_PORT                 (162)
#define NX_SNMP_MAX_TRAP_KEY                      (64)
#if (!1)
      #define NX_SNMP_DISABLE_V1
      #endif
#if (!1)
      #define NX_SNMP_DISABLE_V2
      #endif
#if (!1)
      #define NX_SNMP_DISABLE_V3
      #endif
#endif /* NXD_SNMP_AGENT_CFG_H_ */
