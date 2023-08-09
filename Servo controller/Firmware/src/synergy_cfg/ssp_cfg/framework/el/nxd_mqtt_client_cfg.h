/* generated configuration header file - do not edit */
#ifndef NXD_MQTT_CLIENT_CFG_H_
#define NXD_MQTT_CLIENT_CFG_H_
/* Defined, this option removes the basic MQTT client error checking */
#if (BSP_CFG_PARAM_CHECKING_ENABLE)
            #define NX_DISABLE_ERROR_CHECKING
            #endif

/* Enabling NX Secure Component within MQTT Client Instance */
#if (1)
#ifndef NX_SECURE_ENABLE
#define NX_SECURE_ENABLE
#endif
#endif

/* Defines the time out in the TCP socket disconnect call when disconnecting from the MQTT server in timer ticks */
#define NXD_MQTT_SOCKET_TIMEOUT (0xFFFFFFFF)

/* The maximum topic length (in bytes) the application is going to subscribe to */
#define NXD_MQTT_MAX_TOPIC_NAME_LENGTH (12)

/* The maximum message length (in bytes) the application is going to send or receive */
#define NXD_MQTT_MAX_MESSAGE_LENGTH (32)

/* The MQTT timer rate */
#define NXD_MQTT_KEEPALIVE_TIMER_RATE (1 * TX_TIMER_TICKS_PER_SECOND)

/* Time MQTT client waits for PINGRESP from the broker for after it sends out MQTT PINGREQ */
#define NXD_MQTT_PING_TIMEOUT_DELAY (1 * TX_TIMER_TICKS_PER_SECOND)
#endif /* NXD_MQTT_CLIENT_CFG_H_ */
