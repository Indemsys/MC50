/* generated configuration header file - do not edit */
#ifndef SF_CRYPTO_HASH_CFG_H_
#define SF_CRYPTO_HASH_CFG_H_
#define SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE (BSP_CFG_PARAM_CHECKING_ENABLE)

/* The use of BSP_CFG_MCU_PART_SERIES below is not expected in a SSP framework module because
 * framework modules should not have any MCU part dependencies. However, the MCU part dependent
 * macro is temporarily used here as an exception in this version of SSP. The implementation is
 * a subject to change and will be updated in the future version of SSP.
 */
#if (BSP_CFG_MCU_PART_SERIES == 1) || (BSP_CFG_MCU_PART_SERIES == 3)
            #define SF_CRYPTO_HASH_CFG_HAL_NOT_AVAILABLE
            #endif
#endif /* SF_CRYPTO_HASH_CFG_H_ */
