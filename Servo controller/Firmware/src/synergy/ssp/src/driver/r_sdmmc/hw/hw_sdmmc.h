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
 * File Name    : hw_sdmmc.c
 * Description  : Hardware related  LLD functions for the SDMMC HAL
 **********************************************************************************************************************/
#ifndef HW_SDMMC_H
#define HW_SDMMC_H

#include "bsp_api.h"
#include "bsp_clock_cfg.h"
#include "../sdcard.h"
#include "../sdhi.h"
#include "r_sdmmc_cfg.h"
#include "r_ioport.h"
#include "r_elc_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SDMMC_EVENTS_PER_CHANNEL   (4U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 *   This function sets normally untouched I/O register bits to the default state.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_RegisterDefaultSet (R_SDHI0_Type * p_sdhi_reg)
{
    /* Execute software reset or check SD_INFO2.SD_CLK_CTRLEN prior to calling this function. */
    p_sdhi_reg->SD_CLK_CTRL              = 0x20U;                  /* Automatic clock control disabled. */
    p_sdhi_reg->SDIO_MODE                = 0x00U;                  /* Not in SDIO mode initially. */
    p_sdhi_reg->SD_DMAEN                 = 0x00U;                  /* Not in DMA mode initially. */
    p_sdhi_reg->SDIF_MODE                = 0x00U;                  /* CRC check is valid. */
    p_sdhi_reg->EXT_SWAP                 = 0x00U;                  /* SD_BUF normal read and write operation. */
}


/*******************************************************************************************************************//**
 *   This function sends the SD command.
 *   @param[in] channel - channel of SDHI
 *   @param[in] command - command to send
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_CommandSend (R_SDHI0_Type * p_sdhi_reg, uint16_t command)
{
    p_sdhi_reg->SD_CMD = command; /* write command to command register */
}

/*******************************************************************************************************************//**
 *   This function returns the state of the response end.
 *   @param[in] channel - channel of SDHI
 *   @retval state of flag.
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_ResponseEndGet (R_SDHI0_Type * p_sdhi_reg)
{
    return p_sdhi_reg->SD_INFO1_b.RSPEND;
}

/*******************************************************************************************************************//**
 *   This function enables SDHI DMA mode.
 *   @param[in] channel - channel of SDHI
 *   @retval read data
 **********************************************************************************************************************/
static inline void HW_SDMMC_DMAModeEnable (R_SDHI0_Type * p_sdhi_reg, bool enable)
{

    if (enable)
    {
        /* When the SD_DMAEN.DMAEN bit is 1, set the SD_INFO2_MASK.BWEM bit to 1 and the SD_INFO2_MASK.BREM bit to 1. */
        p_sdhi_reg->SD_INFO2_MASK |= 0x300U;
        p_sdhi_reg->SD_DMAEN |= (uint32_t) 0x02;
    }
    else
    {
        p_sdhi_reg->SD_DMAEN &= (uint32_t) ~0x02;
    }

}


/*******************************************************************************************************************//**
 *   This function returns data read from the media card.
 *   @param[in] channel - channel of SDHI
 *   @retval read data
 **********************************************************************************************************************/
static inline uint32_t HW_SDMMC_DataGet (R_SDHI0_Type * p_sdhi_reg)
{

    return (p_sdhi_reg->SD_BUF0_b.SD_BUF);

}

/*******************************************************************************************************************//**
 *   This function sets up data to be written to the media card.
 *   @param[in] channel - channel of SDHI
 *   @param[in] Pointer to data to be written.
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_DataWrite(R_SDHI0_Type * p_sdhi_reg, uint32_t *data)
{

    p_sdhi_reg->SD_BUF0_b.SD_BUF = *data;

}

/*******************************************************************************************************************//**
 *   This function returns the state of the card detect pin.
 *   @param[in] channel - channel of SDHI
 *   @retval - State of the card detect pin.
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_CardInsertedGet (R_SDHI0_Type * p_sdhi_reg)
{

    return ((p_sdhi_reg->SD_INFO1_b.CDS ));

}

/*******************************************************************************************************************//**
 *   This function returns the state of the write protect pin.
 *   @param[in] channel - channel of SDHI
 *   @retval State of the write protect pin.
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_WriteProtectGet (R_SDHI0_Type * p_sdhi_reg)
{

    return (!(p_sdhi_reg->SD_INFO1_b.WPS));

}

/*******************************************************************************************************************//**
 *   This function returns the illegal read data error if it exists; if the data register was read when it was empty.
 *   @param[in] channel - channel of SDHI
 *   @retval State of the ILR flag.
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_ReadDataError (R_SDHI0_Type * p_sdhi_reg)
{
    return (p_sdhi_reg->SD_INFO2_b.ILR);
}

/*******************************************************************************************************************//**
 *   This function returns the SDHI hardware version.
 *   @param[in] none
 *   @retval Unmasked error flags.
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_VersionGet (R_SDHI0_Type * p_sdhi_reg)
{
    return (p_sdhi_reg->VERSION);
}

/*******************************************************************************************************************//**
 *   This function returns the value of the info1 register..
 *   @param[in] channel - channel of SDHI
 *   @retval Info1 register contents
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_Info1Get (R_SDHI0_Type * p_sdhi_reg)
{
    return p_sdhi_reg->SD_INFO1;
}

/*******************************************************************************************************************//**
 *   This function returns the value of the info2 register.
 *   @param[in] channel - channel of SDHI
 *   @retval Info2 register contents
 **********************************************************************************************************************/


static inline uint32_t HW_SDMMC_Info2Get (R_SDHI0_Type * p_sdhi_reg)
{
    return p_sdhi_reg->SD_INFO2;
}

/*******************************************************************************************************************//**
 *   This function returns the value of the SDIO info1 register.
 *   @param[in] channel - channel of SDHI
 *   @retval SDIO Info1 register contents
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_SDIO_Info1Get (R_SDHI0_Type * p_sdhi_reg)
{
    return p_sdhi_reg->SDIO_INFO1;
}

/*******************************************************************************************************************//**
 *   This function sets the value of the info1 register.
 *   @param[in] channel - channel of SDHI
 *   @param[in] set_value - value to set to the Info1 register
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_Info1Set (R_SDHI0_Type * p_sdhi_reg, uint32_t set_value)
{
    //reset only the bit as in set_value
    p_sdhi_reg->SD_INFO1 = (~set_value);
}

/*******************************************************************************************************************//**
 *   This function sets the value of the info2 register.
 *   @param[in] channel - channel of SDHI
 *   @param[out] set_value - value to set to the Info2 register
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_Info2Set (R_SDHI0_Type * p_sdhi_reg, uint32_t set_value)
{
    //reset only the bit as in set_value
    p_sdhi_reg->SD_INFO2 = (~set_value);
}

/*******************************************************************************************************************//**
 *   This function returns the value of the info2 register BRE bit.
 *   @param[in] channel - channel of SDHI
 *   @retval bit status
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_ReadBufferReadyGet(R_SDHI0_Type * p_sdhi_reg)
{
    return (p_sdhi_reg->SD_INFO2_b.BRE);
}


/*******************************************************************************************************************//**
 *   This function sets the value of the sdio info1 register.
 *   @param[in] channel - channel of SDHI
 *   @param[in] set_value - value to set to the sdio Info1 register
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_SDIO_Info1Set (R_SDHI0_Type * p_sdhi_reg, uint32_t set_value)
{
    p_sdhi_reg->SDIO_INFO1 = set_value;
}

/*******************************************************************************************************************//**
 *   This function sets the value of the sdio mode register.
 *   @param[in] channel - channel of SDHI
 *   @param[in] set_value - value to set to the sdio Info1 register
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_SdioIrqEnable (R_SDHI0_Type * p_sdhi_reg, bool enable)
{
    if (enable)
    {
        p_sdhi_reg->SDIO_MODE_b.IOMOD = 1U;
        p_sdhi_reg->SDIO_INFO1_MASK = 0x6U;
    }
    else
    {
        p_sdhi_reg->SDIO_MODE_b.IOMOD = 0U;
        p_sdhi_reg->SDIO_INFO1_MASK_b.IOIRQM = 1U;
    }
}

/*******************************************************************************************************************//**
 *   This function returns the state of the command busy flag.
 *   @param[in] channel - channel of SDHI
 *   @retval false - command not busy
 *   @retval true - command busy
 **********************************************************************************************************************/

static inline bool HW_SDMMC_CommandBusyGet (R_SDHI0_Type * p_sdhi_reg)
{
    return (bool) p_sdhi_reg->SD_INFO2_b.CBSY;
}

/*******************************************************************************************************************//**
 *   This function returns the state of the card DAT0, indicating busy if held low for an extended period.
 *   @param[in] channel - channel of SDHI
 *   @retval true - card busy
 *   @retval false - card not busy
 **********************************************************************************************************************/

static inline bool HW_SDMMC_CardBusyGet (R_SDHI0_Type * p_sdhi_reg)
{
    return (!p_sdhi_reg->SD_INFO2_b.DAT0);
}

/*******************************************************************************************************************//**
 *   This function clears status registers info1 and info2.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_StatusClear (R_SDHI0_Type * p_sdhi_reg)
{
    p_sdhi_reg->SD_INFO1 = 0U;
    p_sdhi_reg->SD_INFO2 = 0U;
}

/*******************************************************************************************************************//**
 *   This function enables the clock.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ClockEnable (R_SDHI0_Type * p_sdhi_reg)
{
    p_sdhi_reg->SD_CLK_CTRL_b.SCLKEN = 1U;
}

/*******************************************************************************************************************//**
 *   This function disables the clock.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ClockDisable (R_SDHI0_Type * p_sdhi_reg)
{
    p_sdhi_reg->SD_CLK_CTRL_b.SCLKEN = 0U;

}

/*******************************************************************************************************************//**
 *   This function returns the state of the clockdivenable register bit.
 *   @param[in] channel - channel of SDHI
 *   @retval true - bit is high
 *   @retval false - bit is low
 **********************************************************************************************************************/

static inline uint32_t HW_SDMMC_ClockDivEnableGet(R_SDHI0_Type * p_sdhi_reg)
{

    return (p_sdhi_reg->SD_INFO2_b.SCLKDIVEN);

}

/*******************************************************************************************************************//**
 *   This function sets the argument registers for the command.
 *   @param[in] channel - channel of SDHI
 *   @param[in] argument - SD command argument
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_SetArguments (R_SDHI0_Type * p_sdhi_reg, uint32_t argument)
{
    p_sdhi_reg->SD_ARG = (argument & 0x0ffff);
    p_sdhi_reg->SD_ARG1 = ((argument >> 16) & 0x0ffff);
}

/*******************************************************************************************************************//**
 *   This function sets the block size.
 *   @param[in] channel - channel of SDHI
 *   @param[in] size - size of blocks
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_BlockSizeSet (R_SDHI0_Type * p_sdhi_reg, uint16_t size)
{
    p_sdhi_reg->SD_SIZE_b.LEN = (uint32_t) size & 0x3FFU;
}

/*******************************************************************************************************************//**
 *   This function enables and disables block counting for multiple block transfers.
 *   @param[in] channel - channel of SDHI
 *   @param[in] enable  - enable/disable data stop
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_DataStop (R_SDHI0_Type * p_sdhi_reg, uint32_t enable)
{
    p_sdhi_reg->SD_STOP_b.STP = (enable & 1U);
}

/*******************************************************************************************************************//**
 *   This function enables and disables stop bit for block counting of multiple block transfers.
 *   @param[in] channel - channel of SDHI
 *   @param[in] enable - enable/disable block count
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_BlockCountEnable (R_SDHI0_Type * p_sdhi_reg, uint32_t enable)
{
    p_sdhi_reg->SD_STOP_b.SEC = (enable & 1U);
}

/*******************************************************************************************************************//**
 *   This function sets the block count for multiple block transfers.
 *   @param[in] channel - channel of SDHI
 *   @param[in] count - number of blocks to transfer
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_BlockCountSet (R_SDHI0_Type * p_sdhi_reg, uint32_t count)
{
    p_sdhi_reg->SD_SECCNT = count;
}

/*******************************************************************************************************************//**
 *   This function gets the R2 response.
 *   @param[in] channel - channel of SDHI
 *   @param[in,out] - pointer to csd register data
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ResponseR2Get (R_SDHI0_Type * p_sdhi_reg, sdmmc_priv_csd_reg_t * csd)
{

    /* SDResponseR2 are bits from 8-127, first 8 MSBs are reserved */
    csd->reg.sdrsp10 = p_sdhi_reg->SD_RSP10;
    csd->reg.sdrsp32 = p_sdhi_reg->SD_RSP32;
    csd->reg.sdrsp54 = p_sdhi_reg->SD_RSP54;
    csd->reg.sdrsp76 = p_sdhi_reg->SD_RSP76;

}

/*******************************************************************************************************************//**
 *   This function gets the response.
 *   @param[in] channel - channel of SDHI
 *   @retval card status - status of card
 **********************************************************************************************************************/

static inline void HW_SDMMC_ResponseGet (R_SDHI0_Type * p_sdhi_reg, sdmmc_priv_card_status_t * response)
{
    response->status = p_sdhi_reg->SD_RSP10;
}

/*******************************************************************************************************************//**
 *   This function gets the response to a multiple block command..
 *   @param[in] channel - channel of SDHI
 *   @retval card status - status of card
 **********************************************************************************************************************/

static inline void HW_SDMMC_Response54Get (R_SDHI0_Type * p_sdhi_reg, sdmmc_priv_card_status_t * response)
{
    response->status = p_sdhi_reg->SD_RSP54;
}

/*******************************************************************************************************************//**
 *   This function sets the bus width.
 *   @param[in] channel - channel of SDHI
 *   @param[in] bus_width - width of the SD data bus
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_BusWidthSet (R_SDHI0_Type * p_sdhi_reg, uint8_t bus_width)
{
    switch (bus_width)
    {
    case 1U:
        p_sdhi_reg->SD_OPTION_b.WIDTH = 0x1U;
        p_sdhi_reg->SD_OPTION_b.WIDTH8 = 0x0U;
        break;
    case 4U:
        p_sdhi_reg->SD_OPTION_b.WIDTH = 0x0U;
        p_sdhi_reg->SD_OPTION_b.WIDTH8 = 0x0U;
        break;
    case 8U:
        p_sdhi_reg->SD_OPTION_b.WIDTH = 0x0U;
        p_sdhi_reg->SD_OPTION_b.WIDTH8 = 0x1U;
        break;
    default:
        p_sdhi_reg->SD_OPTION_b.WIDTH = 0x0U;
        p_sdhi_reg->SD_OPTION_b.WIDTH8 = 0x0U;
        break;
    }
}

/*******************************************************************************************************************//**
 *   This function sets the clock rate.
 *   @param[in] channel - channel of SDHI
 *   @param[in] clock divider
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ClockRateSet (R_SDHI0_Type * p_sdhi_reg, uint8_t clock_divider)
{
    p_sdhi_reg->SD_CLK_CTRL_b.DIV = clock_divider;
}

static inline uint8_t HW_SDMMC_ClockDivGet (R_SDHI0_Type * p_sdhi_reg)
{
    return (uint8_t) p_sdhi_reg->SD_CLK_CTRL_b.DIV;
}

/*******************************************************************************************************************//**
 *   This function asserts reset of SDHI.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ResetAssert (R_SDHI0_Type * p_sdhi_reg)
{
    /* Reset. */
    p_sdhi_reg->SOFT_RST_b.SDRST = 0x0U;
}

/*******************************************************************************************************************//**
 *   This function releases reset of SDHI.
 *   @param[in] channel - channel of SDHI
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_ResetRelease (R_SDHI0_Type * p_sdhi_reg)
{
    /* Release reset.  */
    p_sdhi_reg->SOFT_RST_b.SDRST = 0x1U;
}

static uint32_t g_sdmmc_default_info1_mask[2] =
{
    ~ (0U),                                                                       // SDMMC_CARD_DETECT_NONE
    ~ (SDMMC_SDHI_INFO1_CARD_REMOVED | SDMMC_SDHI_INFO1_CARD_INSERTED),           // SDMMC_CARD_DETECT_CD
};
static inline void HW_SDMMC_InterruptMaskInfo1Set(R_SDHI0_Type * p_sdhi_reg, sdmmc_card_detect_t card_detect)
{
    p_sdhi_reg->SD_INFO1_MASK = g_sdmmc_default_info1_mask[card_detect];
}

static inline void HW_SDMMC_ResponseEndEnable(R_SDHI0_Type * p_sdhi_reg)
{
    /* Disable access end interrupt and enable response end interrupt. */
    uint32_t mask = p_sdhi_reg->SD_INFO1_MASK;
    mask &= (~SDMMC_SDHI_INFO1_RESPONSE_END);
    mask |= SDMMC_SDHI_INFO1_ACCESS_END;
    p_sdhi_reg->SD_INFO1_MASK = mask;
}

static inline void HW_SDMMC_AccessEndEnable(R_SDHI0_Type * p_sdhi_reg)
{
    /* Disable response end interrupt (set the bit) and enable access end interrupt (clear the bit). */
    uint32_t mask = p_sdhi_reg->SD_INFO1_MASK;
    mask &= (~SDMMC_SDHI_INFO1_ACCESS_END);
    mask |= SDMMC_SDHI_INFO1_RESPONSE_END;
    p_sdhi_reg->SD_INFO1_MASK = mask;
}

/*******************************************************************************************************************//**
 *   This function sets Info2 mask.
 *   @param[in] channel - channel of SDHI
 *   @param[in] mask to set
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_InterruptMaskInfo2Set(R_SDHI0_Type * p_sdhi_reg, uint32_t mask)
{
    p_sdhi_reg->SD_INFO2_MASK = mask;
}

static inline uint32_t HW_SDMMC_InterruptMaskInfo1Get(R_SDHI0_Type * p_sdhi_reg)
{
    return p_sdhi_reg->SD_INFO1_MASK;
}

/*******************************************************************************************************************//**
 *   This function sets SDIO Info1 mask.
 *   @param[in] channel - channel of SDHI
 *   @param[in] mask to set
 *   @retval none
 **********************************************************************************************************************/
static inline void HW_SDMMC_InterruptMaskSdioInfo1Set (R_SDHI0_Type * p_sdhi_reg, uint32_t mask)
{
    p_sdhi_reg->SDIO_INFO1_MASK = mask;
}

/*******************************************************************************************************************//**
 *   This function sets the card detect timer..
 *   @param[in] channel - channel of SDHI
 *   @param[in] count
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_CardDetectTimeCounterSet(R_SDHI0_Type * p_sdhi_reg, uint8_t count)
{
    p_sdhi_reg->SD_OPTION_b.CTOP = count & 0x0FU;
}

/*******************************************************************************************************************//**
 *   This function sets the timeout counter.
 *   @param[in] channel - channel of SDHI
 *   @param[in] count
 *   @retval none
 **********************************************************************************************************************/

static inline void HW_SDMMC_TimeoutCounterSet(R_SDHI0_Type * p_sdhi_reg, uint8_t count)
{
    p_sdhi_reg->SD_OPTION_b.TOP  = count & 0x0FU;
}

/*******************************************************************************************************************//**
 *   This function returns data buffer address from the media card.
 *   @param[in] channel - channel of SDHI
 *   @retval Data buffer register address.
 **********************************************************************************************************************/
static inline uint32_t * HW_SDMMC_DataBufferAddressGet (R_SDHI0_Type * p_sdhi_reg)
{

    return (uint32_t *) (&p_sdhi_reg->SD_BUF0);

}

/*******************************************************************************************************************//**
 *   This function returns the ELC event for DMA by channel.
 *   @param[in] channel - channel of SDHI
 *   @retval elc event
 **********************************************************************************************************************/
static inline elc_event_t HW_SDMMC_DmaEventGet (uint8_t channel)
{
    elc_event_t event = ELC_EVENT_SDHIMMC0_DMA_REQ;

    if (1U == channel)
    {
        event = (elc_event_t) ((uint32_t) event + SDMMC_EVENTS_PER_CHANNEL);
    }

    return event;
}

#endif // HW_SDMMC_H

