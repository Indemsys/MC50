/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_jpeg_decode.c
 * Description  : JPEG device low level functions used to implement JPEG_DECODE interface driver.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_jpeg_decode.h"
#include "hw/hw_jpeg_decode_private.h"
#include "r_jpeg_decode_private.h"
#include "r_jpeg_decode_private_api.h"
#include "../r_jpeg_common/r_jpeg_common.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef JPEG_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define JPEG_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_module_version)
#endif

#define JPEG_ALIGNMENT_8   (0x07U)
#define JPEG_ALIGNMENT_16  (0x0FU)
#define JPEG_ALIGNMENT_32  (0x1FU)

#define JPEG_INBUFFER_SET  (0x02U)
#define JPEG_OUTBUFFER_SET (0x04U)

#define JPEG_MODE_DECODE   (0x80U)

#define BUFFER_MAX_SIZE    (0xfff8U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

#if (1 == JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE)

static ssp_err_t r_jpeg_decode_open_param_check (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                 jpeg_decode_cfg_t const * const p_cfg);

static ssp_err_t r_jpeg_decode_outbufferset_param_check (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                         void * p_output_buffer, uint32_t output_buffer_size);

static ssp_err_t r_jpeg_decode_horizontalstrideset_param_check (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                                uint32_t horizontal_stride);

#endif

static ssp_err_t r_jpeg_decode_line_number_get (jpeg_decode_instance_ctrl_t * const p_ctrl,
                                                uint16_t * p_lines_to_decode);

static void      r_jpeg_decode_input_start (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint32_t data_buffer_size);

static void      r_jpeg_decode_output_start (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint16_t lines_to_decode);

static void      r_jpeg_decode_input_resume (jpeg_decode_instance_ctrl_t * const p_ctrl,
                                             const uint32_t data_buffer_size);

static void      r_jpeg_decode_output_resume (jpeg_decode_instance_ctrl_t * const p_ctrl,
                                              const uint16_t lines_to_decode);
static void jpeg_decode_jdti_isr(void * const p_ctrl);

static void jpeg_decode_jedi_isr(void * const p_ctrl);
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/** Implementation of General JPEG Codec Driver  */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const jpeg_decode_api_t g_jpeg_decode_on_jpeg_decode =
{
    .open                = R_JPEG_Decode_Open,
    .outputBufferSet     = R_JPEG_Decode_OutputBufferSet,
    .inputBufferSet      = R_JPEG_Decode_InputBufferSet,
    .linesDecodedGet     = R_JPEG_Decode_LinesDecodedGet,
    .horizontalStrideSet = R_JPEG_Decode_HorizontalStrideSet,
    .imageSubsampleSet   = R_JPEG_Decode_ImageSubsampleSet,
    .imageSizeGet        = R_JPEG_Decode_ImageSizeGet,
    .statusGet           = R_JPEG_Decode_StatusGet,
    .close               = R_JPEG_Decode_Close,
    .versionGet          = R_JPEG_Decode_VersionGet,
    .pixelFormatGet      = R_JPEG_Decode_PixelFormatGet,
};

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

static jpeg_common_ctrl_t jpeg_decode_isr_common_ctrl = {0U};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "jpeg";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug. This pragma suppresses the warnings in this
 * structure only, and will be removed when the SSP compiler is updated to v5.3. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_module_version =
{
    .api_version_minor  = JPEG_DECODE_API_VERSION_MINOR,
    .api_version_major  = JPEG_DECODE_API_VERSION_MAJOR,
    .code_version_major = JPEG_DECODE_CODE_VERSION_MAJOR,
    .code_version_minor = JPEG_DECODE_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/*******************************************************************************************************************//**
 * @addtogroup JPEG_DECODE
 * @{
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @brief  Initialize the JPEG Codec module.  This function configures the JPEG Codec for decoding
 *         operation, sets up the registers for data format and pixel format based on user-supplied
 *         configuration parameters.  Interrupts are enabled to support image size read operation and callback
 *         functions.
 *
 * @retval  SSP_SUCCESS         JPEG Codec module is properly configured and is ready to take input data.
 * @retval  SSP_ERR_IN_USE      JPEG Codec is already in use.
 * @retval  SSP_ERR_ASSERTION   Pointer to the control block or the configuration structure is NULL.
 * @retval  SSP_ERR_HW_LOCKED   JPEG Codec resource is locked.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_Open (jpeg_decode_ctrl_t * const p_api_ctrl, jpeg_decode_cfg_t const * const p_cfg)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err;
    uint8_t  inten0 = 0;
    uint32_t inten1 = 0;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    err = r_jpeg_decode_open_param_check (p_ctrl, p_cfg);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE == p_ctrl->status), SSP_ERR_IN_USE);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_JPEG;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_JPEG_Type *) info.ptr;

    /** Verify JPEG Codec is not already used.  */
    err = R_BSP_HardwareLock(&ssp_feature);
    JPEG_ERROR_RETURN((SSP_SUCCESS == err), SSP_ERR_HW_LOCKED);

    /** Update the common control parameter with the control and JEDI and JDTI callback handler for JPEG decode,
     *  the handlers will be called from r_jpeg_common, which implements JPEG JDTI and JEDI ISR for r_jpeg_decode and
     *  r_jpeg_encode driver.
     */
    jpeg_decode_isr_common_ctrl.p_ctrl = p_ctrl;
    jpeg_decode_isr_common_ctrl.p_jedi_isr_callback = jpeg_decode_jedi_isr;
    jpeg_decode_isr_common_ctrl.p_jdti_isr_callback = jpeg_decode_jdti_isr;

    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JDTI, &event_info);
    IRQn_Type jdti_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == jdti_irq)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    JPEG_ERROR_RETURN(SSP_INVALID_VECTOR != jdti_irq, SSP_ERR_IRQ_BSP_DISABLED);
    R_SSP_VectorInfoGet(jdti_irq, &p_vector_info);
    NVIC_SetPriority(jdti_irq, p_cfg->jdti_ipl);
    *(p_vector_info->pp_ctrl) = &jpeg_decode_isr_common_ctrl;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JEDI, &event_info);
    IRQn_Type jedi_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == jedi_irq)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    JPEG_ERROR_RETURN(SSP_INVALID_VECTOR != jedi_irq, SSP_ERR_IRQ_BSP_DISABLED);
    R_SSP_VectorInfoGet(jedi_irq, &p_vector_info);
    NVIC_SetPriority(jedi_irq, p_cfg->jedi_ipl);
    *(p_vector_info->pp_ctrl) = &jpeg_decode_isr_common_ctrl;
    /** Record the configuration settings. */
    p_ctrl->pixel_format = p_cfg->pixel_format;

    /** Initialize horizontal stride value. */
    p_ctrl->horizontal_stride = 0U;

    /** Initialize output buffer size. */
    p_ctrl->outbuffer_size = 0U;

    /** Initialize total_lines_decoded */
    p_ctrl->total_lines_decoded = 0U;

    /** Initialize horizontal sub-sample setting. */
    p_ctrl->horizontal_subsample = JPEG_DECODE_OUTPUT_NO_SUBSAMPLE;

    /** Provide power to the JPEG module.  */
    R_BSP_ModuleStart(&ssp_feature);

    /** Clear the image horizontal and vertical size, before starting the JPEG decode */
    HW_JPEG_ImageSizeSet(p_ctrl->p_reg, 0U, 0U);

    /** Perform bus reset */
    HW_JPEG_BusReset(p_ctrl->p_reg);

    /** Reset the destination buffer address.   */
    HW_JPEG_DecodeDestinationAddressSet(p_ctrl->p_reg, 0);

    /** Reset the source buffer address.   */
    HW_JPEG_DecodeSourceAddressSet(p_ctrl->p_reg, 0);

    /** Reset the horizontal stride.  */
    HW_JPEG_HorizontalStrideSet(p_ctrl->p_reg, 0);

    /** Configure the JPEG module for decode operation. */
    HW_JPEG_SetProcess(p_ctrl->p_reg, (uint8_t)JPEG_OPERATION_DECODE);

    /** Set image format for the decoded image. */
    HW_JPEG_DecodeOutputImageFormatSet(p_ctrl->p_reg, p_cfg->pixel_format);

    /** If the output pixel format is ARGB8888, also configure the alpha value. */
    if (JPEG_DECODE_PIXEL_FORMAT_ARGB8888 == p_cfg->pixel_format)
    {
        /** Set the alpha value for the decoded image. */
        HW_JPEG_DecodeOutputAlphaSet(p_ctrl->p_reg, p_cfg->alpha_value);
    }

    /** Set the output data format. */
    HW_JPEG_DecodeDataSwap(p_ctrl->p_reg, p_cfg->input_data_format, p_cfg->output_data_format);

    /** The following interrupts are enabled:
     *  Interrupt on all errors
     *  Interrupt on Image Size
     */
    inten0 |= JPEG_INTE0_INT5 | JPEG_INTE0_INT6 | JPEG_INTE0_INT7;
    /* Enable image size interrupt.  */
    inten0 |= JPEG_INTE0_INT3;

    NVIC_EnableIRQ(jedi_irq);

    HW_JPEG_InterruptEnable0Set(p_ctrl->p_reg, inten0);
    inten1 |= JPEG_INTE1_DBTEN;
    NVIC_EnableIRQ(jdti_irq);
    HW_JPEG_InterruptEnable1Set(p_ctrl->p_reg, inten1);

    /** Record user supplied callback routine. */
    p_ctrl->p_callback = p_cfg->p_callback;

    /** Set the driver status.  */
    p_ctrl->status = JPEG_DECODE_STATUS_IDLE;

    p_ctrl->p_context = p_cfg->p_context;

    /** All done.  Return success. */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Assign output buffer to the JPEG Codec for storing output data.
 * @note   The number of image lines to be decoded depends on the size of the buffer and the horizontal stride
 *         settings. Once the output buffer size is known, the horizontal stride value is known, and the input 
 *         pixel format is known (the input pixel format is obtained by the JPEG decoder from the JPEG headers), 
 *         the driver automatically computes the number of lines that can be decoded into the output buffer.  
 *         After these lines are decoded, the JPEG engine pauses and a callback function is triggered, so the application
 *         is able to provide the next buffer for the JPEG module to resume the operation.
 *
 *         The JPEG decoding operation automatically starts after both the input buffer and the output buffer are set,
 *         and the output buffer is big enough to hold at least eight lines of decoded image data.
 *
 * @retval  SSP_SUCCESS                        The output buffer is properly assigned to JPEG codec device.
 * @retval  SSP_ERR_ASSERTION                  Pointer to the control block is NULL, or the pointer to the output_buffer.
 *                                             is NULL, or the output_buffer_size is 0.
 * @retval  SSP_ERR_INVALID_ALIGNMENT          Buffer starting address is not 8-byte aligned.
 * @retval  SSP_ERR_NOT_OPEN                   JPEG not opened.
 * @retval  SSP_ERR_JPEG_BUFFERSIZE_NOT_ENOUGH Invalid buffer size
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_OutputBufferSet (jpeg_decode_ctrl_t * p_api_ctrl, void * p_output_buffer, uint32_t output_buffer_size)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
    uint16_t lines_to_decode = 0;
#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    err = r_jpeg_decode_outbufferset_param_check (p_ctrl, p_output_buffer, output_buffer_size);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /* Return error code if any errors were detected. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)(p_ctrl->status))
    {
        return(p_ctrl->error_code);
    }

    /** Set the decoding destination address.  */
    HW_JPEG_DecodeDestinationAddressSet(p_ctrl->p_reg, (uint32_t) p_output_buffer);

    /** Record the size of the output buffer. */
    p_ctrl->outbuffer_size = output_buffer_size;

    /** If the image size is not ready yet, the driver does not know the input pixel format.
     *  Without that information, the driver is unable to compute the number of lines of image
     *  to decode.  In this case, the driver would record the output buffer size.  Once all the
     *  information is ready, the driver would attempt to start the decoding process. */
    if ((uint32_t)(JPEG_DECODE_STATUS_IMAGE_SIZE_READY) & (uint32_t)p_ctrl->status)
    {
        /** For a given buffer size, compute number of lines to decode if the image size acquisition is known. */
        if(p_ctrl->horizontal_stride)
        {
            err = r_jpeg_decode_line_number_get(p_ctrl, &lines_to_decode);
            if(SSP_SUCCESS == err)
            {
                /** If the driver status is IMAGE_SIZE_READY with no other flags,
                 *  that means the driver just received IMAGE_SIZE.  It has not started
                 *  the decoding process yet. */
                if (JPEG_DECODE_STATUS_IMAGE_SIZE_READY == p_ctrl->status)
                {
                    /** If Input buffer is set, output buffer is set, and horizontal stride is set, the driver is
                     *  able to determine the number of lines to decode, and start the decoding operation. */
                    if(HW_JPEG_DecodeSourceAddressGet(p_ctrl->p_reg))
                    {
                        r_jpeg_decode_output_start(p_ctrl, lines_to_decode);
                    }
                }
                /** If the current status is OUTPUT_PAUSE, the driver
                 *  needs to resume the operation. */
                else if ((uint32_t)(JPEG_DECODE_STATUS_OUTPUT_PAUSE) & (uint32_t)p_ctrl->status)
                {
                    r_jpeg_decode_output_resume(p_ctrl, lines_to_decode);
                }
                else
                {
                    /* Do nothing */
                }
            }
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief  Returns the number of lines decoded into the output buffer.
 * @note   Use this function to retrieve number of image lines written to the output buffer after JPEG decoded
 *         a partial image.  Combined with the horizontal stride settings and the output pixel format, the application
 *         can compute the amount of data to read from the output buffer.
 *
 * @retval        SSP_SUCCESS                The output buffer is properly assigned to JPEG codec device.
 * @retval        SSP_ERR_ASSERTION          Pointer to the control block is NULL, or the pointer to the output_buffer.
 *                                           is NULL, or the output_buffer_size is 0.
 * @retval        SSP_ERR_NOT_OPEN           JPEG not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_LinesDecodedGet (jpeg_decode_ctrl_t * p_api_ctrl, uint32_t * p_lines)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_lines);
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
#endif

    /* Return error code if any errors were detected. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        return(p_ctrl->error_code);
    }

    /* Get the pixel format */
    if (JPEG_DECODE_COLOR_SPACE_YCBCR420 == HW_JPEG_InputPixelFormatGet(p_ctrl->p_reg))
    {
        /* Get the line number of decoded.  */
        *p_lines = HW_JPEG_LinesDecodedGet(p_ctrl->p_reg) * 2;
    }
    else
    {
        /* Get the line number of decoded.  */
        *p_lines = HW_JPEG_LinesDecodedGet(p_ctrl->p_reg) * 1;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Assign input data buffer to JPEG codec for processing.
 * @note   After the amount of data is processed, the JPEG driver triggers a callback function with the flag
 *         JPEG_OPERATION_INPUT_PAUSE set.
 *         The application supplies the next chunk of data to the driver so JPEG decoding can resume.
 *
 *         The JPEG decoding operation automatically starts after both the input buffer and the output buffer are set,
 *         and the output buffer is big enough to hold at least one line of decoded image data.
 *
 * @retval        SSP_SUCCESS                The input data buffer is properly assigned to JPEG Codec device.
 * @retval        SSP_ERR_ASSERTION          Pointer to the control block is NULL, or the pointer to the input_buffer is
 *                                           NULL, or the input_buffer_size is 0.
 * @retval        SSP_ERR_INVALID_ALIGNMENT  Buffer starting address is not 8-byte aligned.
 * @retval        SSP_ERR_NOT_OPEN           JPEG not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_InputBufferSet (jpeg_decode_ctrl_t * const p_api_ctrl,
                                        void                       * p_data_buffer,
                                        uint32_t                   data_buffer_size)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_data_buffer);
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    JPEG_ERROR_RETURN(!(data_buffer_size & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
    JPEG_ERROR_RETURN(!((uint32_t) p_data_buffer & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
#endif

    /* Return error code if any errors were detected. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        return(p_ctrl->error_code);
    }

    /** Configure the input buffer address. */
    HW_JPEG_DecodeSourceAddressSet(p_ctrl->p_reg, p_data_buffer);

    /** If the system is idle, start the JPEG engine.  This allows the
     *  system to obtain image information (image size and input pixel format).  This
     *  information is needed to drive the decode process later on. */
    if (JPEG_DECODE_STATUS_IDLE == p_ctrl->status)
    {
        /** Based on buffer size, detect the in count mode setting.
         *  The driver is able to read input data in chunks.  However the size of each chunk
         *  is limited to BUFFER_MAX_SIZE.   Therefore, if the input data size is larger than
         *  BUFFER_MAX_SIZE, the driver assumes the entire input data is present, and can be decoded
         *  without additional input data.   Otherwise, the driver enables input stream feature.
         *  This works even if the entire input size is smaller than BUFFER_MAX_SIZE. */
        r_jpeg_decode_input_start (p_ctrl, data_buffer_size);
    }

    /* If the JPEG driver is paused for input data, the driver needs to resume the
     * operation. */
    else if ((uint32_t)(JPEG_DECODE_STATUS_INPUT_PAUSE) & (uint32_t)p_ctrl->status)
    {
        r_jpeg_decode_input_resume (p_ctrl, data_buffer_size);
    }
    else
    {
        /* Do nothing */
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure horizontal and vertical subsample.
 * @note   Use for scaling the decoded image.
 *
 * @retval        SSP_SUCCESS                Horizontal Stride value is properly configured.
 * @retval        SSP_ERR_ASSERTION          Pointer to the control block is NULL.
 * @retval        SSP_ERR_INVALID_ARGUMENT   Sub-sample setting is invalid.
 * @retval        SSP_ERR_NOT_OPEN           JPEG not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_ImageSubsampleSet (jpeg_decode_ctrl_t * const p_api_ctrl,
                                           jpeg_decode_subsample_t    horizontal_subsample,
                                           jpeg_decode_subsample_t    vertical_subsample)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    JPEG_ERROR_RETURN((JPEG_DECODE_OUTPUT_SUBSAMPLE_ONE_EIGHTH >= horizontal_subsample), SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN((JPEG_DECODE_OUTPUT_SUBSAMPLE_ONE_EIGHTH >= vertical_subsample), SSP_ERR_INVALID_ARGUMENT);
#endif

    /* Return error code if any errors were detected. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        return(p_ctrl->error_code);
    }

    /** Update horizontal sub-sample setting. */
    p_ctrl->horizontal_subsample = horizontal_subsample;

    HW_JPEG_OutputSubsampleSet(p_ctrl->p_reg, vertical_subsample, horizontal_subsample);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Configure horizontal stride setting.
 * @note   Use when the horizontal stride needs to match the image width and the image size is
 *         unknown when opening the JPEG driver. (If the image size is known prior to the open call,
 *         pass the horizontal stride value in the jpef_cfg_t  structure.) After the image size becomes available,
 *         use this function to update the horizontal stride value. If the driver must decode one
 *         line at a time, the horizontal stride can be set to zero.
 *
 * @retval        SSP_SUCCESS                Horizontal Stride value is properly configured.
 * @retval        SSP_ERR_ASSERTION          Pointer to the control block is NULL.
 * @retval        SSP_ERR_INVALID_ALIGNMENT  Horizontal stride is zero or not 8-byte aligned.
 * @retval        SSP_ERR_NOT_OPEN           JPEG not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_HorizontalStrideSet (jpeg_decode_ctrl_t * p_api_ctrl, uint32_t horizontal_stride)
{
    ssp_err_t err = SSP_SUCCESS;
    uint16_t lines_to_decode = 0U;
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    err = r_jpeg_decode_horizontalstrideset_param_check (p_ctrl, horizontal_stride);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /* Return error code if any errors were detected. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        return(p_ctrl->error_code);
    }

    if (JPEG_DECODE_PIXEL_FORMAT_ARGB8888 == HW_JPEG_DecodeOutputImageFormatGet(p_ctrl->p_reg))
    {
        horizontal_stride *= 4U;
    }
    else
    {
        horizontal_stride *= 2U;
    }

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    JPEG_ERROR_RETURN(!(horizontal_stride & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
#endif

    /** Record the horizontal stride value in the control block */
    p_ctrl->horizontal_stride = horizontal_stride;

    /** Set the horizontal stride.  */
    HW_JPEG_HorizontalStrideSet(p_ctrl->p_reg, horizontal_stride);

    /** If the parameters all are set, resume the core to decode.   */
    if ((HW_JPEG_DecodeDestinationAddressGet(p_ctrl->p_reg)) && (HW_JPEG_DecodeSourceAddressGet(p_ctrl->p_reg)))
    {
        /** For the given buffer size, compute number of lines to decode. */
        if (p_ctrl->outbuffer_size)
        {
            err = r_jpeg_decode_line_number_get(p_ctrl, &lines_to_decode);
            if(SSP_SUCCESS == err)
            {
                r_jpeg_decode_output_start(p_ctrl, lines_to_decode);
            }
        }
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief  Cancel an outstanding JPEG codec operation and close the device.
 *
 * @retval        SSP_SUCCESS                The input data buffer is properly assigned to JPEG Codec device.
 * @retval        SSP_ERR_ASSERTION          Pointer to the control block is NULL.
 * @retval        SSP_ERR_NOT_OPEN           JPEG not opened.
 * @return        See @ref Common_Error_Codes or functions called by this function for other possible return codes.
 *                This function calls:
 *                                    * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_Close (jpeg_decode_ctrl_t * p_api_ctrl)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_JPEG;

    /** Clear JPEG JINTE0 interrupt and JINTE1 interrupt. */
    HW_JPEG_InterruptEnable0Set(p_ctrl->p_reg, 0);
    HW_JPEG_InterruptEnable1Set(p_ctrl->p_reg, 0);

    HW_JPEG_InterruptStatus0Set(p_ctrl->p_reg, 0);
    HW_JPEG_InterruptStatus1Set(p_ctrl->p_reg, 0);

    /** Disable
     *  JEDI and JDTI at NVIC */
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JDTI, &event_info);
    IRQn_Type jdti_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != jdti_irq)
    {
        R_SSP_VectorInfoGet(jdti_irq, &p_vector_info);
        NVIC_DisableIRQ(jdti_irq);
        *(p_vector_info->pp_ctrl) = NULL;
    }
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JEDI, &event_info);
    IRQn_Type jedi_irq = event_info.irq;
    if (SSP_INVALID_VECTOR != jedi_irq)
    {
        R_SSP_VectorInfoGet(jedi_irq, &p_vector_info);
        NVIC_DisableIRQ(jedi_irq);
        *(p_vector_info->pp_ctrl) = NULL;
    }

    HW_JPEG_DecodeInCountModeResume(p_ctrl->p_reg, (uint8_t)0x01);
    HW_JPEG_DecodeOutCountModeResume(p_ctrl->p_reg, (uint8_t)0x01);


    /** Power off the JPEG codec.  */
    R_BSP_ModuleStop(&ssp_feature);

    /** Reset the jpeg status flag in the driver.  */
    p_ctrl->status = JPEG_DECODE_STATUS_FREE;

    /** Unlock module at BSP level. */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Obtain the size of the image.  This operation is valid during
 *         JPEG decoding operation.
 *
 * @retval        SSP_SUCCESS                 The image size is available and the horizontal and vertical values are
 *                                            stored in the memory pointed to by p_horizontal_size and p_vertical_size.
 * @retval        SSP_ERR_ASSERTION           Pointer to the control block is NULL.
 * @retval        SSP_ERR_IMAGE_SIZE_UNKNOWN  The image size is unknown.  More input data may be needed.
 * @retval        SSP_ERR_INVALID_MODE        JPEG Codec module is not decoding.
 * @retval        SSP_ERR_NOT_OPEN            JPEG is not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_ImageSizeGet (jpeg_decode_ctrl_t * p_api_ctrl,
                                      uint16_t           * p_horizontal_size,
                                      uint16_t           * p_vertical_size)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_horizontal_size);
    SSP_ASSERT(NULL != p_vertical_size);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
#endif

    /* Get the image horizontal and vertical size.*/
    HW_JPEG_ImageSizeGet(p_ctrl->p_reg, p_horizontal_size, p_vertical_size);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get the status of the JPEG codec.  This function can also be used to poll the device.
 *
 * @retval         SSP_SUCCESS                 The status information is successfully retrieved.
 * @retval         SSP_ERR_ASSERTION           Pointer to the control block is NULL.
 * @retval         SSP_ERR_NOT_OPEN            JPEG is not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_StatusGet (jpeg_decode_ctrl_t * p_api_ctrl, jpeg_decode_status_t * p_status)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
#endif


    /** HW does not report error.  Return internal status information. */
    *p_status = p_ctrl->status;

    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        return(p_ctrl->error_code);
    }

    return SSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * @brief  Get the input pixel format.
 *
 * @retval         SSP_SUCCESS                 The status information is successfully retrieved.
 * @retval         SSP_ERR_ASSERTION           Pointer to the control block is NULL.
 * @retval         SSP_ERR_NOT_OPEN            JPEG is not opened.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_PixelFormatGet (jpeg_decode_ctrl_t * p_api_ctrl, jpeg_decode_color_space_t * p_color_space)
{
    jpeg_decode_instance_ctrl_t * p_ctrl = (jpeg_decode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_color_space);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
#endif

    /** HW does not report error.  Return internal status information. */
    *p_color_space = HW_JPEG_InputPixelFormatGet(p_ctrl->p_reg);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Get version of the display interface and GLCD HAL code.
 *
 * @retval      SSP_SUCCESS        Version number
 * @retval      SSP_ERR_ASSERTION  The parameter p_version is NULL.
 * @note  This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_VersionGet (ssp_version_t * p_version)
{
#if JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    *p_version = g_module_version;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup JPEG_DECODE)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/

#if (1 == JPEG_DECODE_CFG_PARAM_CHECKING_ENABLE)

/*******************************************************************************************************************//**
 * @brief  Parameter check function for JPEG Decode driver open processing.
 *
 * @param[in]   p_ctrl           Pointer to control block structure.
 * @param[in]   p_cfg            Pointer to configuration structure.
 * @retval SSP_SUCCESS           All the parameter are valid.
 * @retval SSP_ERR_ASSERTION     One of the following parameters is NULL: p_cfg, or p_ctrl
 * @retval SSP_ERR_INVALID_ARGUMENT  Invalid parameter is passed.
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_decode_open_param_check (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                 jpeg_decode_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    ssp_err_t err = SSP_SUCCESS;
    if (  (JPEG_DECODE_PIXEL_FORMAT_ARGB8888 != p_cfg->pixel_format)
        &&(JPEG_DECODE_PIXEL_FORMAT_RGB565   != p_cfg->pixel_format))
    {
        err = SSP_ERR_INVALID_ARGUMENT;
    }
    JPEG_ERROR_RETURN((JPEG_DECODE_DATA_FORMAT_LONGWORD_WORD_BYTE_SWAP >= p_cfg->input_data_format), SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN((JPEG_DECODE_DATA_FORMAT_LONGWORD_WORD_BYTE_SWAP >= p_cfg->output_data_format), SSP_ERR_INVALID_ARGUMENT);
    return err;
}

/*******************************************************************************************************************//**
 * @brief  Parameter check function for JPEG Decode driver outBufferSet processing.
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   p_output_buffer      Pointer to the output buffer.
 * @param[in]   output_buffer_size   Size of the output buffer.
 * @retval SSP_SUCCESS               All the parameter are valid.
 * @retval SSP_ERR_ASSERTION         One of the following parameters is NULL: p_cfg, or p_ctrl or the callback.
 * @retval SSP_ERR_INVALID_ALIGNMENT p_output_buffer is not on 8-byte memory boundary, or output_buffer_size is not
 *                                   multiple of eight.
 * @retval SSP_ERR_JPEG_BUFFERSIZE_NOT_ENOUGH Invalid buffer size
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_decode_outbufferset_param_check (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                         void * p_output_buffer, uint32_t output_buffer_size)
{
    SSP_ASSERT(NULL != p_output_buffer);
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    JPEG_ERROR_RETURN((0 != output_buffer_size), SSP_ERR_JPEG_BUFFERSIZE_NOT_ENOUGH);
    JPEG_ERROR_RETURN(!(output_buffer_size & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
    JPEG_ERROR_RETURN(!((uint32_t) p_output_buffer & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Parameter check function for JPEG Decode driver horizontalStrideSet processing.
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   horizontal_stride    Horizontal stride of the outbput buffer.
 * @retval SSP_SUCCESS               All the parameter are valid.
 * @retval SSP_ERR_ASSERTION         One of the following parameters is NULL: p_cfg, or p_ctrl or the callback.
 * @retval SSP_ERR_INVALID_ALIGNMENT horizontal_stride is zero.
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_decode_horizontalstrideset_param_check  (jpeg_decode_instance_ctrl_t const * const p_ctrl,
                                                                 uint32_t horizontal_stride)
{
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((0U != horizontal_stride), SSP_ERR_INVALID_ALIGNMENT);
    JPEG_ERROR_RETURN((JPEG_DECODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
return SSP_SUCCESS;
}

#endif

/*******************************************************************************************************************//**
 * @brief Get the number of line to decode and check the image size is valid against the Output buffer.
 *
 * @param[in]   p_ctrl                Pointer to control block structure.
 * @param[out]  p_lines_to_decode     Pointer to number of Output data lines.
 * @retval SSP_SUCCESS               All the parameter are valid.
 * @retval SSP_ERR_JPEG_BUFFERSIZE_NOT_ENOUGH    The size of Output buffer is not enough.
 * @retval SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE   The number of horizontal pixels exceeds horizontal memory stride.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_decode_line_number_get (jpeg_decode_instance_ctrl_t * const p_ctrl,
                                                    uint16_t * p_lines_to_decode)
{
    ssp_err_t err = SSP_SUCCESS;
    uint16_t horizontal;
    uint16_t vertical;
    uint16_t horizontal_bytes;
    uint16_t lines_to_decode;

    lines_to_decode = (uint16_t) ((uint32_t)p_ctrl->outbuffer_size / (uint32_t)p_ctrl->horizontal_stride);

    HW_JPEG_ImageSizeGet(p_ctrl->p_reg, &horizontal, &vertical);
    if(lines_to_decode > (uint16_t)(vertical - p_ctrl->total_lines_decoded))
    {
        lines_to_decode = (uint16_t)(vertical - p_ctrl->total_lines_decoded);
    }

    if (JPEG_DECODE_COLOR_SPACE_YCBCR420 == HW_JPEG_InputPixelFormatGet(p_ctrl->p_reg))
    {
        lines_to_decode &= (uint16_t) (~15);
        lines_to_decode = lines_to_decode/2;
    }
    else
    {
        lines_to_decode &= (uint16_t) (~7);
    }

    *p_lines_to_decode = lines_to_decode;

    if (0U != lines_to_decode)
    {
        if(JPEG_DECODE_PIXEL_FORMAT_ARGB8888 == p_ctrl->pixel_format)
        {
           horizontal_bytes = (uint16_t) (horizontal * 4);
        }
        else
        {
           horizontal_bytes = (uint16_t) (horizontal * 2);
        }

        /** Sub-sample setting is considered to check horizontal pixel size and image stride. */
        if((uint32_t)(horizontal_bytes >> (int32_t)(p_ctrl->horizontal_subsample)) > p_ctrl->horizontal_stride)
        {
           p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_ERROR));
           p_ctrl->error_code = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;

           err = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;
        }
    }
    else
    {
        err = SSP_ERR_JPEG_BUFFERSIZE_NOT_ENOUGH;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief Start JPEG decompression. The JPEG hardware will be set to the Input Count mode if data_buffer_size is smaller
 *  than or equal to BUFFER_MAX_SIZE, else it will not be set to the Input Count mode and the JPEG hardware will not halt
 *  decompression even if the number of input data bytes processed is reached to data_buffer_size. If zero is given to
 *  data_buffer_size, it will not set the JPEG hardware to the Input Count mode.
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   data_buffer_size     Input data size in bytes.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_decode_input_start (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint32_t data_buffer_size)
{
    /** Configure the input count mode. */
    if ((0 != data_buffer_size) && (data_buffer_size <= BUFFER_MAX_SIZE))
    {
        uint32_t inten1;
        inten1  = HW_JPEG_InterruptEnable1Get(p_ctrl->p_reg);
        inten1 |= JPEG_INTE1_JINEN;
        HW_JPEG_InterruptEnable1Set(p_ctrl->p_reg, inten1);

        HW_JPEG_DecodeInCountModeEnable(p_ctrl->p_reg);

        HW_JPEG_DecodeInCountModeConfig(p_ctrl->p_reg, JPEG_DECODE_COUNT_MODE_ADDRESS_REINITIALIZE, (uint16_t) data_buffer_size);
    }
    else
    {
        HW_JPEG_DecodeInCountModeDisable(p_ctrl->p_reg);
    }

    /** Set the internal status.  */
    p_ctrl->status = JPEG_DECODE_STATUS_HEADER_PROCESSING;

    /* Start the core. */
    HW_JPEG_CoreStart(p_ctrl->p_reg);
}

/*******************************************************************************************************************//**
 * @brief Set the JPEG hardware to run in the Output Count mode and start JPEG decompression.
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   lines_to_decode     Number of Output data lines.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_decode_output_start (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint16_t lines_to_decode)
{
    /** Configure the out count mode. */
    uint32_t inten1;
    inten1  = HW_JPEG_InterruptEnable1Get(p_ctrl->p_reg);
    inten1 |= JPEG_INTE1_DOUTLEN;
    HW_JPEG_InterruptEnable1Set(p_ctrl->p_reg, inten1);

    HW_JPEG_DecodeOutCountModeEnable(p_ctrl->p_reg);

    HW_JPEG_DecodeOutCountModeConfig(p_ctrl->p_reg, JPEG_DECODE_COUNT_MODE_ADDRESS_REINITIALIZE, lines_to_decode);

    /** Set the driver status to JPEG_DECODE_STATUS_RUNNING. */
    p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_RUNNING));

    /** Clear JPEG stop. */
    HW_JPEG_CoreStopClear(p_ctrl->p_reg);
}

/*******************************************************************************************************************//**
 * @brief Set the JPEG hardware to run in the Input Count mode and resume JPEG decompression.
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   data_buffer_size     Input data size in bytes.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_decode_input_resume (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint32_t data_buffer_size)
{
    /** Clear internal status information. */
    p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_INPUT_PAUSE)));

    /** Set internal status information. */
    p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)JPEG_DECODE_STATUS_RUNNING);

    HW_JPEG_DecodeInCountModeConfig(p_ctrl->p_reg, JPEG_DECODE_COUNT_MODE_ADDRESS_REINITIALIZE, (uint16_t) data_buffer_size);
    /** Resume the jpeg core.  */
    HW_JPEG_DecodeInCountModeResume(p_ctrl->p_reg, (uint8_t)0x01);
}

/*******************************************************************************************************************//**
 * @brief Set the JPEG hardware to run in the Output Count mode and resume JPEG decompression.
 *
 * @param[in]   p_ctrl              Pointer to control block structure.
 * @param[in]   lines_to_decode     Number of Output data lines.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_decode_output_resume (jpeg_decode_instance_ctrl_t * const p_ctrl, const uint16_t lines_to_decode)
{
    /** Clear internal status information. */
    p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_OUTPUT_PAUSE)));

    /** Set internal status information. */
    p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_RUNNING));

    HW_JPEG_DecodeOutCountModeConfig(p_ctrl->p_reg, JPEG_DECODE_COUNT_MODE_ADDRESS_REINITIALIZE, lines_to_decode);

    /** Resume the jpeg core.  */
    HW_JPEG_DecodeOutCountModeResume(p_ctrl->p_reg, (uint8_t)0x01);
}

/*******************************************************************************************************************//**
 * @brief Check if number of pixels is valid against JPEG hardware constraints (the number must be multiple of MCU).
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 * @param[in]   horizontal           Number of horizontal pixels.
 * @param[in]   vertical             Number of vertical pixels.
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_decode_pixel_number_check(jpeg_decode_instance_ctrl_t * const p_ctrl, const uint16_t horizontal,
                                                                                         const uint16_t vertical)
{
    jpeg_decode_color_space_t color_format;

    color_format = HW_JPEG_InputPixelFormatGet(p_ctrl->p_reg);

    if((horizontal & JPEG_ALIGNMENT_8) || (vertical & JPEG_ALIGNMENT_8))
    {
        /* Check if horizontal pixels are multiple of 8 pixels or if vertical pixels are multiple of 8 pixels.
         * (horizontal & JPEG_ALIGNMENT_8): The constraint is for YCbCr444
         * (vertical & JPEG_ALIGNMENT_8): The constraint is for YCbCr444, YCbCr422, YCbCr411
         */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_ERROR));
        p_ctrl->error_code = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;
    }

    /* Check the number of pixels for individual YCbCr color space formats. */
    switch(color_format)
    {
    case JPEG_DECODE_COLOR_SPACE_YCBCR422:
        /* 16 pixels. */
        if(horizontal & JPEG_ALIGNMENT_16)
        {
            p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_ERROR));
            p_ctrl->error_code = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;
        }
        break;

    case JPEG_DECODE_COLOR_SPACE_YCBCR411:
        /* 8 lines by 32 pixels. */
        if(horizontal & JPEG_ALIGNMENT_32)
        {
            p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_ERROR));
            p_ctrl->error_code = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;
        }
        break;

    case JPEG_DECODE_COLOR_SPACE_YCBCR420:
        /* 16 lines by 16 pixels. */
        if((horizontal & JPEG_ALIGNMENT_16) || (vertical & JPEG_ALIGNMENT_16))
        {
            p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_ERROR));
            p_ctrl->error_code = SSP_ERR_JPEG_UNSUPPORTED_IMAGE_SIZE;
        }
        break;

    default:
        break;
    }
}

/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Data Transfer Interrupt (JDTI) Interrupt Service Routine (ISR).
 * @retval  None
 **********************************************************************************************************************/
static void jpeg_decode_jdti_isr(void * const p_common_ctrl)
{
    jpeg_decode_instance_ctrl_t *p_ctrl = p_common_ctrl;
    uint32_t            intertype;
    jpeg_decode_callback_args_t args;
    /** Get the interrupt type.  */
    intertype = HW_JPEG_InterruptStatus1Get(p_ctrl->p_reg);

    /** Clear the interrupt flag.  */
    HW_JPEG_InterruptStatus1Set(p_ctrl->p_reg, 0x0);

    /** Clear the interrupt flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Return if there are errors. */
    if ((uint32_t)(JPEG_DECODE_STATUS_ERROR) & (uint32_t)p_ctrl->status)
    {
        /* If error is detected, no need to further process this interrupt.  Simply return. */
        return;
    }

    if (intertype & JPEG_INTE1_JINF)
    {
        /** Clear internal status information. */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_RUNNING)));

        /* Set the ctrl status.  */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_INPUT_PAUSE));

        /** Clear the source address.  */
        HW_JPEG_DecodeSourceAddressSet(p_ctrl->p_reg, 0);

        /** Invoke user-supplied callback function if it is set. */
        if ((NULL != p_ctrl->p_callback))
        {
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    if (intertype  & JPEG_INTE1_DOUTLF)
    {
        uint32_t lines_decoded = 0;
        uint16_t horizontal;
        uint16_t vertical;

        /** Clear internal status information. */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_RUNNING)));

        /* Set the ctrl status.  */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_OUTPUT_PAUSE));

        /** Clear the destination address.  */
        HW_JPEG_DecodeDestinationAddressSet(p_ctrl->p_reg, 0);

        /** Obtain the number of lines decoded in this operation. */
        if(SSP_SUCCESS != R_JPEG_Decode_LinesDecodedGet(p_ctrl, &lines_decoded))
        {
            /* If error is detected, no need to further process this interrupt.  Simply return. */
            return;
        }

        /** Increment the number of lines decoded. */
        p_ctrl->total_lines_decoded = (uint16_t)(p_ctrl->total_lines_decoded + (uint16_t)lines_decoded);

        HW_JPEG_ImageSizeGet(p_ctrl->p_reg, &horizontal, &vertical);
        if(p_ctrl->total_lines_decoded > vertical)
        {
            p_ctrl->total_lines_decoded = vertical;
        }

        /** Invoke user-supplied callback function if it is set. */
        if ((NULL != p_ctrl->p_callback))
        {
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    if (intertype & JPEG_INTE1_DBTF)
    {
        /** Clear internal status information. */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_RUNNING)));
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_OUTPUT_PAUSE)));

        /* Set the ctrl status.  */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_DONE));

        /** Invoke user-supplied callback function if it is set. */
        if ((NULL != p_ctrl->p_callback))
        {
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

}

/*******************************************************************************************************************//**
 * @brief   JPEG internal function: JPEG Decompression Process Interrupt (JEDI) Interrupt Service Routine (ISR).
 * @retval  None
 **********************************************************************************************************************/
static void jpeg_decode_jedi_isr(void * const p_common_ctrl)
{
    uint8_t    intertype;
    uint16_t   horizontal = 0;
    uint16_t   vertical = 0;
    jpeg_decode_callback_args_t args;
    jpeg_decode_instance_ctrl_t *p_ctrl = p_common_ctrl;
    /** Get the interrupt type.  */
    intertype = HW_JPEG_InterruptStatus0Get(p_ctrl->p_reg);

    /** Clear the interrupt status.  */
    HW_JPEG_InterruptStatus0Set(p_ctrl->p_reg, 0);

    /** Clear the request.  */
    HW_JPEG_ClearRequest(p_ctrl->p_reg);

    /** Clear the interrupt flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    if (intertype & JPEG_INTE0_INS5)
    {
        /* Set the internal status.  */
        p_ctrl->status = JPEG_DECODE_STATUS_ERROR;

        p_ctrl->error_code = (ssp_err_t) (HW_JPEG_ErrorGet(p_ctrl->p_reg) + (uint32_t) SSP_ERR_JPEG_ERR);
        /** Invoke user-supplied callback function if it is set. */
        if (NULL != p_ctrl->p_callback)
        {
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        /* If error is detected, no need to further process this interrupt.  Simply return. */
        return;
    }

    if (intertype & JPEG_INTE0_INS3)
    {
        /** Clear internal status information. */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_HEADER_PROCESSING)));

        /* Set the ctrl status.  */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_IMAGE_SIZE_READY));

        /** JPEG header is decoded.  Obtain image size, and input pixel format.  Verify that image size (width and height)
                is aligned to the Minimum Coded Unit. */
        HW_JPEG_ImageSizeGet(p_ctrl->p_reg, &horizontal, &vertical);
        r_jpeg_decode_pixel_number_check(p_ctrl, horizontal, vertical);

        /** Invoke user-supplied callback function if it is set. */
        if (NULL != p_ctrl->p_callback)
        {
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        /** If both Input buffer and  output buffer are set, and horizontal stride is set, the driver is available
         *  to determine the number of lines to decode, and start the decoding operation. */
        if ((HW_JPEG_DecodeSourceAddressGet(p_ctrl->p_reg)) && (p_ctrl->outbuffer_size) && (p_ctrl->horizontal_stride))
        {
            uint16_t lines_to_decode = 0;

            if (SSP_SUCCESS != r_jpeg_decode_line_number_get(p_ctrl, &lines_to_decode))
            {
                /* If error is detected, no need to further process this interrupt.  Simply return. */
                return;
            }

            /* Set the ctrl status.  */
            p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_DECODE_STATUS_RUNNING));

            /** according the buffer size, detect the out count mode setting.  */
            r_jpeg_decode_output_start(p_ctrl, lines_to_decode);
        }
    }

    if (intertype & JPEG_INTE0_INS6)
    {
        /** Clear internal status information. */
        p_ctrl->status = (jpeg_decode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_DECODE_STATUS_RUNNING)));
    }
}
