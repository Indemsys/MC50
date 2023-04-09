/***********************************************************************************************************************
 * Copyright [2015-2018] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_jpeg_common.c
 * Description  : JPEG Codec common ISR file shared between r_jpeg_decode and r_jpeg_encode.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_jpeg_common.h"

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

void jpeg_jdti_isr();

void jpeg_jedi_isr();

/*******************************************************************************************************************//**
 * @brief JPEG internal function: Data Transfer Interrupt (JDTI) Interrupt Service Routine (ISR).
 * @retval None
 **********************************************************************************************************************/
void jpeg_jdti_isr()
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /** Obtain the control block. */
    ssp_vector_info_t * p_vector_info = 0;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(),&p_vector_info);
    jpeg_common_ctrl_t * p_common_ctrl = (jpeg_common_ctrl_t *) *(p_vector_info->pp_ctrl);

    p_common_ctrl->p_jdti_isr_callback(p_common_ctrl->p_ctrl);

    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * @brief JPEG internal function: JPEG Compression/Decompression Process Interrupt (JEDI) Interrupt Service Routine (ISR).
 * @retval None
 **********************************************************************************************************************/
void jpeg_jedi_isr()
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /** Obtain the control block. */
    ssp_vector_info_t * p_vector_info = 0;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(),&p_vector_info);
    jpeg_common_ctrl_t * p_common_ctrl = (jpeg_common_ctrl_t *) *(p_vector_info->pp_ctrl);

    p_common_ctrl->p_jedi_isr_callback(p_common_ctrl->p_ctrl);

    SF_CONTEXT_RESTORE
}

