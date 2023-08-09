/* generated configuration header file - do not edit */
#ifndef NX_SECURE_USER_H_
  #define NX_SECURE_USER_H_
/* NX_CRYPTO_AES_OUTPUT_BUFFER specifies the size of internal data buffer used for Encryption/Decryption processing.*/
  #ifndef NX_CRYPTO_AES_OUTPUT_BUFFER_SIZE
    #define NX_CRYPTO_AES_OUTPUT_BUFFER_SIZE              (2048U)
  #endif

/* NX_SECURE_TLS_ENABLE_TLS_1_3 defines whether or not to disable TLS 1.3 protocol support.
 BY default TLS 1.3 is not enabled. */
  #if (0)
    #define NX_SECURE_TLS_ENABLE_TLS_1_3
  #endif

/* NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE defines whether or not to disables
 protocol version downgrade for TLS client. BY default protocol version downgrade is supported. */
  #if (0)
    #define NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE
  #endif

/* Defines Maximum RSA Modulus Bits*/
  #ifndef NX_CRYPTO_MAX_RSA_MODULUS_SIZE
    #define NX_CRYPTO_MAX_RSA_MODULUS_SIZE                (4096U)
  #endif

/* Enables the optional X.509 Distinguished
 Name fields, at the expense of extra memory use for X.509 certificates.*/
  #if (0)
    #define NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
  #endif

/* Enables strict distinguished name comparison for X.509 certificates for certificate searching and verification*/
  #if (0)
    #define NX_SECURE_X509_STRICT_NAME_COMPARE
  #endif

/* NX_SECURE_X509_DISABLE_CRL disables X509 Certificate Revocation List check.
 By default this feature is enabled. */
  #if(0)
    #define NX_SECURE_X509_DISABLE_CRL
  #endif

/* NX_SECURE_DISABLE_X509 disables X509 feature. By default this feature is enabled. */
  #if (0)
    #define NX_SECURE_DISABLE_X509
  #endif

/* NX_SECURE_ENABLE_PSK_CIPHERSUITES enables Pre-Shared Key.  By default
 this feature is not enabled. */
  #if (0)
    #define NX_SECURE_ENABLE_PSK_CIPHERSUITES
  #endif

/* NX_SECURE_ENABLE_ECC_CIPHERSUITES enables ECC ciphersuites.  By default
 this feature is not enabled. */
  #if (0)
    #define NX_SECURE_ENABLE_ECC_CIPHERSUITE
  #else
    #define NX_SECURE_DISABLE_ECC_CIPHERSUITE
  #endif

/* NX_SECURE_ENABLE_AEAD_CIPHER enables AEAD ciphersuites.
 For AEAD ciphersuites other than AES-CCM or AES-GCM, additional definition of
 NX_SECURE_AEAD_CIPHER_CHECK must be defined. By default this feature is not enabled. */
  #if (0)
    #define NX_SECURE_ENABLE_AEAD_CIPHER
  #endif

/* NX_SECURE_AEAD_CIPHER_CHECK AEAD ciphersuites other than AES-CCM or AES-GCM.
 It works only when NX_SECURE_ENABLE_AEAD_CIPHER is defined.
 By default this feature is not enabled. */
  #if (0)
    #define NX_SECURE_AEAD_CIPHER_CHECK(a) ((a) == (NX_FALSE))
  #endif

/* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE enables ECJPAKE ciphersuites for DTLS.
 By default this feature is not enabled. */
  #if (0)
    #define NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
  #endif

/* NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE enables SCSV ciphersuite in ClientHello message.
 By default this feature is not enabled. */
  #if(0)
    #define NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
  #endif

/* NX_SECURE_KEY_CLEAR enables key related materials cleanup when they are not used anymore.
 By default this feature is not enabled. */
  #if (0)
    #define NX_SECURE_KEY_CLEAR
  #endif

/* NX_SECURE_ENABLE_DTLS enables DTLS feature. By default this feature is not enabled. */
  #if (0)
    #define NX_SECURE_ENABLE_DTLS
  #endif

/* NX_SECURE_DTLS_COOKIE_LENGTH defines the length of DTLS cookie.
 The default value is 32. */
  #define NX_SECURE_DTLS_COOKIE_LENGTH (32)

/* NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES defineds the maximum retransmit retries
 for DTLS handshake packet. The default value is 10. */
  #define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES (10)

/* NX_SECURE_DTLS_RETRANSMIT_TIMEOUT defines the initial DTLS retransmit rate.
 The default value is 1 second. */
  #define NX_SECURE_DTLS_RETRANSMIT_TIMEOUT ((1)*NX_IP_PERIODIC_RATE)

/* NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT defines the maximum DTLS retransmit rate.
 The default value is 60 seconds. */
  #define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT ((60) * NX_IP_PERIODIC_RATE)

/* NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT defins how the retransmit timeout period changes between successive retries.
 If this value is 0, the initial retransmit timeout is the same as subsequent retransmit timeouts. If this
 value is 1, each successive retransmit is twice as long. The default value is 1.  */
  #define NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT (1)

/* NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY enables client certificate verification.
 By default this feature is not enabled. */
  #if(0)
    #define NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
  #endif

/* NX_SECURE_TLS_MAX_PSK_ID_SIZE defines the maximum size of PSK ID.
 By default it is 20. */
  #define NX_SECURE_TLS_MAX_PSK_ID_SIZE (20)

/* NX_SECURE_TLS_MAX_PSK_KEYS defines the maximum PSK keys.
 By default it is 5. */
  #define NX_SECURE_TLS_MAX_PSK_KEYS (5)

/* NX_SECURE_TLS_MAX_PSK_SIZE defines the maximum size of PSK.
 By default it is 20. */
  #define NX_SECURE_TLS_MAX_PSK_SIZE (20)

/* NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE defines a minimum reasonable size for a TLS
 X509 certificate. This is used in checking for * errors in allocating certificate space.
 The size is determined by assuming a 512-bit RSA key, MD5 hash, and a rough estimate of
 other data. It is theoretically possible for a real certificate to be smaller,
 but in that case, bypass the error checking by re-defining this macro.
 Approximately: 64(RSA) + 16(MD5) + 176(ASN.1 + text data, common name, etc)
 The default value is 256. */
  #define NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE (256)

/* NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE defines the minimum size for the TLS message buffer.
 It is determined by a number of factors, but primarily the expected size of the TLS handshake
 Certificate message (sent by the TLS server) that may contain multiple certificates of 1-2KB each.
 The upper limit is determined by the length field in the TLS header (16 bit), and is 64KB.
 The default value is 4000. */
  #define NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE (4000)

/* NX_SECURE_TLS_PREMASTER_SIZE defines the sie of pre-master secret.
 The default value is 48. */
  #define NX_SECURE_TLS_PREMASTER_SIZE (48)

/* NX_SECURE_TLS_SNI_EXTENSION_DISABLED disables Server Name Indication (SNI) extension.
 By default this feature is enabled */
  #if (0)
    #define NX_SECURE_TLS_SNI_EXTENSION_DISABLED
  #endif

/* This option removes all TLS stack code related to TLS Server mode, reducing code and data usage */
  #if (0)
    #define NX_SECURE_TLS_SERVER_DISABLED
  #endif

/* This option removes all TLS stack code related to TLS Client mode, reducing code and data usage */
  #if (0)
    #define NX_SECURE_TLS_CLIENT_DISABLED
  #endif

/* This option allows TLS to accept self-signed certificates from a remote host.
 Otherwise the certificate must be issued by a certificate in the trusted certificate store */
  #if (1)
    #define NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
  #endif

/* This option specifies whether to use Software Crypto Engines or not */
  #if (0)
    #define NX_CRYPTO_ENGINE_SW
  #endif

/* NX_SECURE_MEMCMP defines the memory compare function.
 By default it is mapped to C library function. */
  #define NX_SECURE_MEMCMP memcmp

/* NX_SECURE_MEMCPY defines the memory copy function.
 By default it is mapped to C library function. */
  #define NX_SECURE_MEMCPY memcpy

/* NX_SECURE_MEMMOVE defines the memory move function.
 By default it is mapped to C library function. */
  #define NX_SECURE_MEMMOVE memmove

/* NX_SECURE_MEMSET defines the memory set function.
 By default it is mapped to C library function. */
  #define NX_SECURE_MEMSET memset
#endif /* NX_SECURE_USER_H_ */
