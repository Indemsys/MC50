/* generated configuration header file - do not edit */
#ifndef NXD_WEB_HTTP_CLIENT_CFG_H_
#define NXD_WEB_HTTP_CLIENT_CFG_H_
#define NX_WEB_HTTP_CLIENT_MIN_PACKET_SIZE              300

/* Defined, this option removes the basic Web HTTP client error checking */
#if (!(BSP_CFG_PARAM_CHECKING_ENABLE))
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Enabling NX Secure Component within Web HTTP Client Instance */
#if (1)
#ifndef NX_WEB_HTTPS_ENABLE
#define NX_WEB_HTTPS_ENABLE
#endif
#endif
#endif /* NXD_WEB_HTTP_CLIENT_CFG_H_ */
