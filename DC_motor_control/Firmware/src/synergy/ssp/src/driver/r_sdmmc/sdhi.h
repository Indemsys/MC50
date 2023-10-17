/***********************************************************************************************************************
 * Copyright [2015-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : sdhi.h
 * Description  : Hardware related helper functions for the SDMMC protocol.
 **********************************************************************************************************************/
#ifndef SDMMC_SDHI_H_
#define SDMMC_SDHI_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "bsp_api.h"
#include "sdcard.h"
#include "r_sdmmc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Create a bitmask of access errors. */
#define SDMMC_CARD_CMD_ERR (1U << 16)               //< Command error
#define SDMMC_CARD_CRC_ERR (1U << 17)               //< CRC error
#define SDMMC_CARD_END_ERR (1U << 18)               //< End bit error
#define SDMMC_CARD_DTO     (1U << 19)               //< Data Timeout
#define SDMMC_CARD_ILW     (1U << 20)               //< Illegal write address
#define SDMMC_CARD_ILR     (1U << 21)               //< Illegal read address
#define SDMMC_CARD_RSPT    (1U << 22)               //< Response timeout
#define SDMMC_CARD_BRE     (1U << 24)               //< Buffer read enable
#define SDMMC_CARD_BWE     (1U << 25)               //< Buffer write enable
#define SDMMC_CARD_ILA_ERR (1U << 31)               //< Illegal access

#define SDMMC_ACCESS_ERROR_MASK  (uint32_t)                                                    \
    (SDMMC_CARD_CMD_ERR | SDMMC_CARD_CRC_ERR | SDMMC_CARD_END_ERR | SDMMC_CARD_DTO | \
     SDMMC_CARD_ILW | SDMMC_CARD_ILR | SDMMC_CARD_RSPT | SDMMC_CARD_ILA_ERR)

/* The clock register can be accessed 8 SD clock cycles after the last command completes. */
#define SDMMC_SDHI_CLKEN_TIMEOUT_SD_CLOCK   (8U)
#define SDMMC_SDHI_DELAY_COMMAND_SEND       (500U)
#define SDMMC_SDHI_DELAY_BUSY_GET           (500U)
#define SDMMC_SDHI_DELAY_RESET              (5000U)
#define SDMMC_SDHI_DELAY_RESPONSE_GET       (10U)

/* SD_INFO1 */
#define SDMMC_SDHI_INFO1_RESPONSE_END       (1U << 0)     //< Response End
#define SDMMC_SDHI_INFO1_ACCESS_END         (1U << 2)     //< Access End
#define SDMMC_SDHI_INFO1_CARD_REMOVED       (1U << 3)     //< Card Removed
#define SDMMC_SDHI_INFO1_CARD_INSERTED      (1U << 4)     //< Card Inserted
#define SDMMC_SDHI_INFO1_CARD_WRITE_PROTECT (1U << 7)     //< Card is Write Protected
#define SDMMC_SDHI_INFO1_CARD_DAT3_REMOVED  (1U << 8)     //< Card Removed
#define SDMMC_SDHI_INFO1_CARD_DAT3_INSERTED (1U << 9)     //< Card Inserted
#define SDMMC_SDHI_INFO1_CARD_STATE         (1U << 10)    //< Card Insert State
#define SDMMC_SDHI_INFO2_CARD_CMD_ERR       (1U << 0)     //< Command error
#define SDMMC_SDHI_INFO2_CARD_CRC_ERR       (1U << 1)     //< CRC error
#define SDMMC_SDHI_INFO2_CARD_END_ERR       (1U << 2)     //< End bit error
#define SDMMC_SDHI_INFO2_CARD_DTO           (1U << 3)     //< Data Timeout
#define SDMMC_SDHI_INFO2_CARD_ILW           (1U << 4)     //< Illegal write address
#define SDMMC_SDHI_INFO2_CARD_ILR           (1U << 5)     //< Illegal read address
#define SDMMC_SDHI_INFO2_CARD_RSPT          (1U << 6)     //< Response timeout
#define SDMMC_SDHI_INFO2_CARD_BRE           (1U << 8)     //< Buffer read enable
#define SDMMC_SDHI_INFO2_CARD_BWE           (1U << 9)     //< Buffer write enable
#define SDMMC_SDHI_INFO2_CARD_CMDBUSY       (1U << 14)    //< Command busy
#define SDMMC_SDHI_INFO2_CARD_ILA_ERR       (1U << 15)    //< Illegal access
#define SDIO_INFO1_IOIRQ              (1U << 0)     //< SDIO interrupt status
#define SDIO_INFO1_EXPUB52            (1U << 14)    //< SDIO EXPUB52 status
#define SDIO_INFO1_EXWT               (1U << 15)    //< SDIO EXWT status

#define SDMMC_SDHI_INFO2_MASK (uint32_t)                                                                                   \
    ((SDMMC_SDHI_INFO2_CARD_CMD_ERR | SDMMC_SDHI_INFO2_CARD_CRC_ERR  | SDMMC_SDHI_INFO2_CARD_END_ERR |                                \
       SDMMC_SDHI_INFO2_CARD_DTO | SDMMC_SDHI_INFO2_CARD_ILW | SDMMC_SDHI_INFO2_CARD_ILR | SDMMC_SDHI_INFO2_CARD_BRE | SDMMC_SDHI_INFO2_CARD_BWE | \
       SDMMC_SDHI_INFO2_CARD_RSPT | SDMMC_SDHI_INFO2_CARD_ILA_ERR))

#define SDMMC_SDHI_INFO1_ACCESS_MASK  (uint32_t) ((SDMMC_SDHI_INFO1_RESPONSE_END | SDMMC_SDHI_INFO1_ACCESS_END))
#define SDMMC_SDHI_INFO1_CARD_MASK    (uint32_t) ((SDMMC_SDHI_INFO1_CARD_REMOVED | SDMMC_SDHI_INFO1_CARD_INSERTED | \
                                                   SDMMC_SDHI_INFO1_CARD_DAT3_REMOVED | SDMMC_SDHI_INFO1_CARD_DAT3_INSERTED))
#define SDMMC_SDHI_INFO1_CARD_REMOVED_MASK    (uint32_t) ((SDMMC_SDHI_INFO1_CARD_REMOVED | SDMMC_SDHI_INFO1_CARD_DAT3_REMOVED))
#define SDMMC_SDHI_INFO1_CARD_INSERTED_MASK   (uint32_t) ((SDMMC_SDHI_INFO1_CARD_INSERTED | SDMMC_SDHI_INFO1_CARD_DAT3_INSERTED))


/* Clear all masks to enable interrupts by all sources.
 * Do not set BREM or BWEM when using DMA/DTC. This driver always uses DMA or DTC. */
#define SDMMC_SDHI_INFO2_MASK_CMD_SEND (0x00007C80U)

/* The relationship of the SD Clock Control Register SD_CLK_CTRL CLKSEL to the division of PCLKA 
 * b7            b0
 * 0 0 0 0 0 0 0 0: PCLKA/2
 * 0 0 0 0 0 0 0 1: PCLKA/4
 * 0 0 0 0 0 0 1 0: PCLKA/8
 * 0 0 0 0 0 1 0 0: PCLKA/16
 * 0 0 0 0 1 0 0 0: PCLKA/32
 * 0 0 0 1 0 0 0 0: PCLKA/64
 * 0 0 1 0 0 0 0 0: PCLKA/128
 * 0 1 0 0 0 0 0 0: PCLKA/256
 * 1 0 0 0 0 0 0 0: PCLKA/512.
 * Other settings are prohibited.
 */
#define SDMMC_MAX_CLOCK_DIVISION_SHIFT   (9U)            /* 512 (2^9) is max clock division supported */
#define SDMMC_MIN_CLOCK_DIVISION_SHIFT   (1U)            /* 2 (2^1) is minimum division supported */

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

typedef enum e_sdhi_timeout
{
    SDMMC_SDHI_TIMEOUT_2_13,
    SDMMC_SDHI_TIMEOUT_2_14,
    SDMMC_SDHI_TIMEOUT_2_15,
    SDMMC_SDHI_TIMEOUT_2_16,
    SDMMC_SDHI_TIMEOUT_2_17,
    SDMMC_SDHI_TIMEOUT_2_18,
    SDMMC_SDHI_TIMEOUT_2_19,
    SDMMC_SDHI_TIMEOUT_2_20,
    SDMMC_SDHI_TIMEOUT_2_21,
    SDMMC_SDHI_TIMEOUT_2_22,
    SDMMC_SDHI_TIMEOUT_2_23,
    SDMMC_SDHI_TIMEOUT_2_24,
    SDMMC_SDHI_TIMEOUT_2_25,
    SDMMC_SDHI_TIMEOUT_2_26,
    SDMMC_SDHI_TIMEOUT_2_27
} sdmmc_sdhi_timeout_t;

typedef enum e_sdhi_cd_timeout
{
    SDMMC_SDHI_CD_TIMEOUT_2_10,
    SDMMC_SDHI_CD_TIMEOUT_2_11,
    SDMMC_SDHI_CD_TIMEOUT_2_12,
    SDMMC_SDHI_CD_TIMEOUT_2_13,
    SDMMC_SDHI_CD_TIMEOUT_2_14,
    SDMMC_SDHI_CD_TIMEOUT_2_15,
    SDMMC_SDHI_CD_TIMEOUT_2_16,
    SDMMC_SDHI_CD_TIMEOUT_2_17,
    SDMMC_SDHI_CD_TIMEOUT_2_18,
    SDMMC_SDHI_CD_TIMEOUT_2_19,
    SDMMC_SDHI_CD_TIMEOUT_2_20,
    SDMMC_SDHI_CD_TIMEOUT_2_21,
    SDMMC_SDHI_CD_TIMEOUT_2_22,
    SDMMC_SDHI_CD_TIMEOUT_2_23,
    SDMMC_SDHI_CD_TIMEOUT_2_24
} sdmmc_sdhi_cd_timeout_t;

/***********************************************************************************************************************
 * Function prototypes
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER


#endif  /* _SDHI_H */
