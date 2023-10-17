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
 * File Name    : r_jpeg_encode.c
 * Description  : JPEG device low level functions used to implement JPEG_ENCODE interface driver.
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_jpeg_encode.h"
#include "hw/hw_jpeg_encode_private.h"
#include "r_jpeg_encode_private_api.h"
#include "../r_jpeg_common/r_jpeg_common.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef JPEG_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define JPEG_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_module_version)
#endif

#define JPEG_ALIGNMENT_8               (0x07U)
/* Table 0(zero) is used for Luminance and Table 1(one) is used for Chrominance - Cr and Cb component */
#define JPEG_QUANTIZATION_TABLE_NUMBER (0x14U)
/* AC and DC table for Luminance and Chrominance */
#define JPEG_HUFFMAN_TABLE_NUMBER      (0x3cU)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if (1 == JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t r_jpeg_encode_open_param_check        (jpeg_encode_instance_ctrl_t const * const p_ctrl,
                                                        jpeg_encode_cfg_t           const * const p_cfg);
static ssp_err_t r_jpeg_encode_input_buffer_param_check(jpeg_encode_instance_ctrl_t const * const p_ctrl,
                                                        uint8_t                     const * const p_data_buffer);
#endif

static void r_jpeg_data_correction(jpeg_encode_instance_ctrl_t * p_ctrl , uint8_t final_output);

static void r_jpeg_upload_quantization_table(jpeg_encode_cfg_t const * const p_cfg);

static void r_jpeg_upload_huffman_table(jpeg_encode_cfg_t const * const p_cfg);

static void jpeg_encoder_jdti_isr(void * const p_ctrl);

static void jpeg_encoder_jedi_isr(void * const p_ctrl);

static void r_jpeg_get_multiply_factor(uint8_t quality_factor, uint16_t *p_multiply_factor);

static void r_jpeg_encode_input_start(jpeg_encode_instance_ctrl_t * p_ctrl);

static void r_jpeg_encode_correct_initial_data(uint8_t * p_start_data, uint8_t *p_initial_data);
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
const jpeg_encode_api_t g_jpeg_encode_on_jpeg_encode =
{
 .open                = R_JPEG_Encode_Open,
 .outputBufferSet     = R_JPEG_Encode_OutputBufferSet,
 .imageParameterSet   = R_JPEG_Encode_ImageParameterSet,
 .inputBufferSet      = R_JPEG_Encode_InputBufferSet,
 .statusGet           = R_JPEG_Encode_StatusGet,
 .close               = R_JPEG_Encode_Close,
 .versionGet          = R_JPEG_Encode_VersionGet,
};

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

static jpeg_common_ctrl_t jpeg_encode_isr_common_ctrl = {0U};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "jpeg_encoder";
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
 .api_version_minor  = JPEG_ENCODE_API_VERSION_MINOR,
 .api_version_major  = JPEG_ENCODE_API_VERSION_MAJOR,
 .code_version_major = JPEG_ENCODE_CODE_VERSION_MAJOR,
 .code_version_minor = JPEG_ENCODE_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/*******************************************************************************************************************//**
 * @addtogroup JPEG_ENCODE
 * @{
 **********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @brief  Initialize the JPEG Codec module.  This function configures the JPEG Codec for encoding
 *         operation, sets up the registers for data format, pixel format, vertical and horizontal resolution stride
 *         based on user-supplied configuration parameters.
 *
 * @retval  SSP_SUCCESS               JPEG Codec module is properly configured and is ready to take input data.
 * @retval  SSP_ERR_IN_USE            JPEG Codec is already in use.
 * @retval  SSP_ERR_ASSERTION         Pointer to the control block or the configuration structure is NULL.
 * @retval  SSP_ERR_HW_LOCKED         JPEG Codec resource is locked.
 * @retval  SSP_ERR_INVALID_ARGUMENT  Invalid parameter is passed.
 * @retval  SSP_ERR_INVALID_ALIGNMENT Horizontal stride is not 8-byte aligned.
 * @return                See @ref Common_Error_Codes or functions called by this function for other possible
 *                        return codes. This function calls:
 *                             * fmi_api_t::productFeatureGet
 *                             * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_Open (jpeg_encode_ctrl_t * const p_api_ctrl, jpeg_encode_cfg_t const * const p_cfg)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
    uint32_t inten1 = 0;

#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    err = r_jpeg_encode_open_param_check (p_ctrl, p_cfg);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = 0U;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_JPEG;
    fmi_feature_info_t info = {0U};
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};

    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_JPEG_Type *) info.ptr;

    /** Verify JPEG Codec is not already used.  */
    err = R_BSP_HardwareLock(&ssp_feature);
    JPEG_ERROR_RETURN((SSP_SUCCESS == err), SSP_ERR_HW_LOCKED);

    /** Update the common control parameter with the control and JDEI and JDTI callback handler for JPEG encode,
     *  the handlers will be called from r_jpeg_common, which implements JPEG JDTI and JDEI ISR for r_jpeg_decode and
     *  r_jpeg_encode driver.
     */
    jpeg_encode_isr_common_ctrl.p_ctrl = p_ctrl;
    jpeg_encode_isr_common_ctrl.p_jedi_isr_callback = jpeg_encoder_jedi_isr;
    jpeg_encode_isr_common_ctrl.p_jdti_isr_callback = jpeg_encoder_jdti_isr;

    /** Get the JDTI event information from FMI */
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JDTI, &event_info);
    IRQn_Type jdti_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == jdti_irq)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }

    JPEG_ERROR_RETURN(SSP_INVALID_VECTOR != jdti_irq, SSP_ERR_IRQ_BSP_DISABLED);
    /** Get the vector table information of JDTI event */
    R_SSP_VectorInfoGet(jdti_irq, &p_vector_info);
    NVIC_SetPriority(jdti_irq, p_cfg->jdti_ipl);

    /** Record the JPEG Encoder ctrl and internal ISR callback handler to JDTI event vector table */
    *(p_vector_info->pp_ctrl) = &jpeg_encode_isr_common_ctrl;

    /** Get the JEDI event information from FMI */
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_JPEG_JEDI, &event_info);
    IRQn_Type jedi_irq = event_info.irq;
    if (SSP_INVALID_VECTOR == jedi_irq)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
    }

    JPEG_ERROR_RETURN(SSP_INVALID_VECTOR != jedi_irq, SSP_ERR_IRQ_BSP_DISABLED);
    /** Get the vector table information of JEDI event */
    R_SSP_VectorInfoGet(jedi_irq, &p_vector_info);
    NVIC_SetPriority(jedi_irq, p_cfg->jedi_ipl);

    /** Record the JPEG Encoder ctrl and internal ISR callback handler to JEDI event vector table */
    *(p_vector_info->pp_ctrl) = &jpeg_encode_isr_common_ctrl;

    /** Provide power to the JPEG module.  */
    R_BSP_ModuleStart(&ssp_feature);

    /** Perform bus reset */
    HW_JPEG_BusReset(p_ctrl->p_reg);

    /** Reset the destination buffer address.   */
    HW_JPEG_EncodeDestinationAddressSet(p_ctrl->p_reg, 0);

    /** Reset the source buffer address.   */
    HW_JPEG_EncodeSourceAddressSet(p_ctrl->p_reg, 0);

    /** Set the horizontal stride.  */
    HW_JPEG_EncodeHorizontalStrideSet(p_ctrl->p_reg, p_cfg->horizontal_resolution*YCBCR_BYTES_PER_PIXEL);

    /** Set the image horizontal and vertical size.*/
    HW_JPEG_ImageSizeSet(p_ctrl->p_reg, p_cfg->horizontal_resolution, p_cfg->vertical_resolution);

    /** Configure the JPEG module for encode operation. */
    HW_JPEG_SetProcess(p_ctrl->p_reg, (uint8_t)JPEG_OPERATION_ENCODE);

    /** Set the output data format. */
    HW_JPEG_EncodeDataSwap(p_ctrl->p_reg, p_cfg->input_data_format, p_cfg->output_data_format);

    /** Set input pixel format for JPEG Encoder
     * NOTE : only ycbcr422 is valid for encoder */
    HW_JPEG_EncodeInputPixelFormatSet (p_ctrl->p_reg, YCBCR422);

    /** Quantization table setting
     * NOTE: Table 0(zero) is used for Luminance and Table 1(one) is used for Chrominance - Cr and Cb component */
    HW_JPEG_EncoderQunatizationTable (p_ctrl->p_reg, JPEG_QUANTIZATION_TABLE_NUMBER);

    /** Upload Luminance and Chrominance table to JPEG Codec */
    r_jpeg_upload_quantization_table(p_cfg);

    /** Huffman table setting */
    HW_JPEG_EncoderHuffmanTable (p_ctrl->p_reg, JPEG_HUFFMAN_TABLE_NUMBER);

    /** Upload the Huffman table to JPEG Codec */
    r_jpeg_upload_huffman_table(p_cfg);

    /** Reset Marker setting */
    HW_JPEG_EncoderResetMarker(p_ctrl->p_reg, p_cfg->dri_marker);

    /** Record image parameters to ctrl */
    p_ctrl->vertical_resolution = p_cfg->vertical_resolution;
    p_ctrl->horizontal_stride = p_cfg->horizontal_resolution * YCBCR_BYTES_PER_PIXEL;

    /** Set the driver status.  */
    p_ctrl->status = JPEG_ENCODE_STATUS_IDLE;

    /** Record the user context information */
    p_ctrl->p_context = p_cfg->p_context;

    /** Record user supplied callback routine. */
    p_ctrl->p_callback = p_cfg->p_callback;

    /** Enabled JPEG Compression data transfer complete interrupt and Count mode Interrupt  */
    inten1 = JPEG_INTE1_CBTEN | JPEG_INTE1_DINLEN;

    HW_JPEG_InterruptEnable1Set(p_ctrl->p_reg, inten1);

    /** Enable the JDTI and JDEI interrupts */
    NVIC_EnableIRQ(jedi_irq);
    NVIC_EnableIRQ(jdti_irq);

    /** All done.  Return success. */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Assign output buffer to the JPEG Codec for storing output data.
 * @note   Buffer size should be sufficient to hold the encoded jpeg image.
 *
 * @retval  SSP_SUCCESS                 The output buffer is properly assigned to JPEG codec device.
 * @retval  SSP_ERR_ASSERTION           Pointer to the control block is NULL, or the pointer to the output_buffer.
 *                                      is NULL, or the output_buffer_size is 0.
 * @retval  SSP_ERR_INVALID_ALIGNMENT   Buffer starting address is not 8-byte aligned.
 * @retval  SSP_ERR_NOT_OPEN            JPEG not opened.
 * @retval  SSP_ERR_INVALID_CALL        An invalid call has been made, Codec output buffer address is attempted
 *                                      to changed during codec operation
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_OutputBufferSet (jpeg_encode_ctrl_t * p_api_ctrl, void * p_output_buffer)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;
#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_ENCODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    SSP_ASSERT(NULL != p_output_buffer);
    JPEG_ERROR_RETURN(!((uint32_t) p_output_buffer & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
#endif

    /** Output buffer cannot be change during codec operation */
    JPEG_ERROR_RETURN(!((uint32_t)(JPEG_ENCODE_STATUS_RUNNING)&(uint32_t)p_ctrl->status),SSP_ERR_INVALID_CALL);
    JPEG_ERROR_RETURN(!((uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE)&(uint32_t)p_ctrl->status),SSP_ERR_INVALID_CALL);

    /** Set the encoding destination address.  */
    HW_JPEG_EncodeDestinationAddressSet(p_ctrl->p_reg, (uint32_t) p_output_buffer);

    return err;
}

/*******************************************************************************************************************//**
 * @brief  Assign input data buffer to JPEG codec for processing.
 * @note   1.After the amount of data is processed, the JPEG driver triggers a callback function with the flag
 *         JPEG_OPERATION_INPUT_PAUSE set.
 *         The application supplies the next chunk of data to the driver so JPEG encoding can resume.
 *         2.Image size should be greater or equal to minimum coded unit (MCU) for YCbCr422 (only supported color space)
 *         the MCU is 8 lines by 16 pixel (where 1 pixel = 2 bytes) hence size can not be less than 8x16x2 = 256
 *
 *         The JPEG encoding operation automatically starts after setting the input buffer.
 *
 * @retval  SSP_SUCCESS                   The input data buffer is properly assigned to JPEG Codec device.
 * @retval  SSP_ERR_ASSERTION             Pointer to the control block is NULL, or the pointer to the p_data_buffer is
 *                                        NULL.
 * @retval  SSP_ERR_INVALID_ALIGNMENT     Buffer starting address or image line to encode or size of buffer is not
 *                                        8-byte aligned.
 * @retval  SSP_ERR_NOT_OPEN              JPEG not opened.
 * @retval  SSP_ERR_INVALID_CALL          An invalid call has been made, set output buffer first
 * @retval  SSP_ERR_JPEG_IMAGE_SIZE_ERROR Image size is not supported by JPEG Codec
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_InputBufferSet (jpeg_encode_ctrl_t * const p_api_ctrl,
                                        void                       * p_data_buffer,
                                        uint32_t                   data_buffer_size)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;
#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    err = r_jpeg_encode_input_buffer_param_check(p_ctrl, p_data_buffer);
    JPEG_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Validate the the size :
     * JPEG Codec can process minimum up to 8 lines by 16 pixel for YCbCr422 meaning 8x16x2 = 256 */
    JPEG_ERROR_RETURN((!(data_buffer_size < 256U)),SSP_ERR_JPEG_IMAGE_SIZE_ERROR);

    /** Calculate the number of lines to be encode */
    p_ctrl->lines_to_encoded = (uint16_t)((data_buffer_size)/(p_ctrl->horizontal_stride));

    /** JPEG Codec required Lines to be byte aligned */
    JPEG_ERROR_RETURN(!((uint32_t) p_ctrl->lines_to_encoded & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT)

    /** Check, If output image buffer is set or not */
    JPEG_ERROR_RETURN(0U != HW_JPEG_EncodeDestinationAddressGet(p_ctrl->p_reg),SSP_ERR_INVALID_CALL)

    /** Configure the input buffer address. */
    HW_JPEG_EncodeSourceAddressSet(p_ctrl->p_reg, p_data_buffer);

    /** if JPEG is just opened or completed one image, make DONE and IDLE status flag zero to encode next image */
    if(((uint32_t)(JPEG_ENCODE_STATUS_IDLE) & (uint32_t)p_ctrl->status)||
            ((uint32_t)(JPEG_ENCODE_STATUS_DONE) & (uint32_t)p_ctrl->status))
    {
        /** Remove the Done and IDLE status flag */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_DONE)));
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_IDLE)));

        /** Set the driver status to JPEG_ENCODE_STATUS_RUNNING. */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_ENCODE_STATUS_RUNNING));

        /** Start the encoder */
        r_jpeg_encode_input_start (p_ctrl);
    }
    /** JPEG is PAUSE for next chunk of image */
    else if((uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE) & (uint32_t)p_ctrl->status)
    {
        /** Clear internal status information. */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status &(~(uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE)));

        /** Set RUNNING status */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_ENCODE_STATUS_RUNNING));

        /** Resume the count mode */
        HW_JPEG_EncodeInputCountModeConfig(p_ctrl->p_reg,JPEG_ENCODE_COUNT_MODE_ADDRESS_REINITIALIZE,
                                           p_ctrl->lines_to_encoded);
        HW_JPEG_EncodeInputCountModeResume(p_ctrl->p_reg,JPEG_ENCODE_COUNT_ENABLE);
    }
    else
    {
        /** JPEG is running Notify the user, return error*/
        err = SSP_ERR_IN_USE;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief  Setup  the  image parameters to JPEG Codec device.
 * @note   Image parameters needs to be set before the setting the input buffer.DO NOT call this function during the
 * JPEG Codec operation.
 *
 * @retval   SSP_SUCCESS                Image parameter is properly assigned to JPEG Codec device.
 * @retval   SSP_ERR_ASSERTION          Pointer to the control block is NULL,
 * @retval   SSP_ERR_INVALID_ALIGNMENT  Horizontal stride is not 8-byte aligned.
 * @retval   SSP_ERR_INVALID_ARGUMENT   Horizontal and Vertical resolution is invalid or zero.
 * @retval   SSP_ERR_NOT_OPEN           JPEG not opened.
 * @retval   SSP_ERR_INVALID_CALL       An invalid call has been made.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_ImageParameterSet(jpeg_encode_ctrl_t        * const p_api_ctrl,
                                          jpeg_encode_raw_image_parameters * p_image_parameters)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;
#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_image_parameters);
    JPEG_ERROR_RETURN((JPEG_ENCODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    JPEG_ERROR_RETURN(0U != p_image_parameters->horizontal_resolution, SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN(0U != p_image_parameters->vertical_resolution, SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN(!(p_image_parameters->horizontal_stride & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
#endif

    /** Do not change the JPEG Codec image setting while JPEG Codec is in progress */
    JPEG_ERROR_RETURN(!((uint32_t)(JPEG_ENCODE_STATUS_RUNNING)&(uint32_t)p_ctrl->status),SSP_ERR_INVALID_CALL);
    JPEG_ERROR_RETURN(!((uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE)&(uint32_t)p_ctrl->status),SSP_ERR_INVALID_CALL);

    /** Record the horizontal stride and vertical size value in the control block */
    p_ctrl->horizontal_stride = p_image_parameters->horizontal_stride * YCBCR_BYTES_PER_PIXEL;
    p_ctrl->vertical_resolution = p_image_parameters->vertical_resolution;
    /** Set the horizontal stride.  */
    HW_JPEG_EncodeHorizontalStrideSet(p_ctrl->p_reg, p_ctrl->horizontal_stride);

    /** Set the image horizontal and vertical size.*/
    HW_JPEG_ImageSizeSet(p_ctrl->p_reg, p_image_parameters->horizontal_resolution,
                         p_image_parameters->vertical_resolution);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get the status of the JPEG codec.  This function can also be used to poll the device.
 *
 * @retval  SSP_SUCCESS                 The status information is successfully retrieved.
 * @retval  SSP_ERR_ASSERTION           Pointer to the control block is NULL.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_StatusGet (jpeg_encode_ctrl_t * p_api_ctrl, volatile jpeg_encode_status_t * p_status)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_status);
#endif

    /** HW does not report error.  Return internal status information. */
    *p_status = p_ctrl->status;

    return SSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief  Cancel an outstanding JPEG codec operation and close the device.
 *
 * @retval  SSP_SUCCESS                The input data buffer is properly assigned to JPEG Codec device.
 * @retval  SSP_ERR_ASSERTION          Pointer to the control block is NULL.
 * @retval  SSP_ERR_NOT_OPEN           JPEG not opened.
 * @return  See @ref Common_Error_Codes or functions called by this function for other possible return codes.
 *          This function calls:
 *             * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_Close (jpeg_encode_ctrl_t * p_api_ctrl)
{
    jpeg_encode_instance_ctrl_t * p_ctrl = (jpeg_encode_instance_ctrl_t *) p_api_ctrl;

#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_ENCODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
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

    HW_JPEG_EncodeInputCountModeResume(p_ctrl->p_reg, JPEG_ENCODE_COUNT_DISABLE);

    /** Power off the JPEG codec.  */
    R_BSP_ModuleStop(&ssp_feature);

    /** Reset the jpeg status flag in the driver.  */
    p_ctrl->status = JPEG_ENCODE_STATUS_FREE;

    /** Unlock module at BSP level. */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Get version of the display interface and GLCD HAL code.
 *
 * @retval  SSP_SUCCESS        Version number
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
 * @note  This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Encode_VersionGet (ssp_version_t * p_version)
{
#if JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    *p_version = g_module_version;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup JPEG_ENCODE)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/

#if (1 == JPEG_ENCODE_CFG_PARAM_CHECKING_ENABLE)

/*******************************************************************************************************************//**
 * @brief  Parameter check function for JPEG Encode driver open processing.
 *
 * @param[in]   p_ctrl           Pointer to control block structure.
 * @param[in]   p_cfg            Pointer to configuration structure.
 * @retval SSP_SUCCESS           All the parameter are valid.
 * @retval SSP_ERR_ASSERTION     One of the following parameters is NULL: p_cfg, or p_ctrl
 * @retval SSP_ERR_INVALID_ARGUMENT  Invalid parameter is passed.
 * @retval SSP_ERR_INVALID_ALIGNMENT  Horizontal stride is not 8-byte aligned.
 * @retval SSP_ERR_IN_USE             JPEG Codec is already in use.
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_encode_open_param_check (jpeg_encode_instance_ctrl_t const * const p_ctrl,
                                                 jpeg_encode_cfg_t const * const p_cfg)
{
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    ssp_err_t err = SSP_SUCCESS;
    JPEG_ERROR_RETURN((JPEG_ENCODE_DATA_FORMAT_LONGWORD_WORD_BYTE_SWAP >= p_cfg->input_data_format),
                       SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN((JPEG_ENCODE_DATA_FORMAT_LONGWORD_WORD_BYTE_SWAP >= p_cfg->output_data_format),
                       SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN(0U != p_cfg->horizontal_resolution, SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN(0U != p_cfg->vertical_resolution, SSP_ERR_INVALID_ARGUMENT);
    JPEG_ERROR_RETURN(!((p_cfg->horizontal_resolution*YCBCR_BYTES_PER_PIXEL) & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
    JPEG_ERROR_RETURN((JPEG_ENCODE_STATUS_FREE == p_ctrl->status), SSP_ERR_IN_USE);
    return err;
}
/*******************************************************************************************************************//**
 * @brief  Parameter check function for JPEG Encode driver inputbufferset.
 *
 * @param[in]   p_ctrl                    Pointer to control block structure.
 * @param[in]   p_data_buffer             pointer to image buffer
 * @retval  SSP_SUCCESS                   All the parameter are valid.
 * @retval  SSP_ERR_ASSERTION             Pointer to the control block is NULL, or the pointer to the p_data_buffer is
 *                                        NULL.
 * @retval  SSP_ERR_INVALID_ALIGNMENT     Buffer starting address is not 8-byte aligned.
 * @retval  SSP_ERR_NOT_OPEN              JPEG not opened.
 **********************************************************************************************************************/
static ssp_err_t r_jpeg_encode_input_buffer_param_check(jpeg_encode_instance_ctrl_t const * const p_ctrl,
                                                        uint8_t const * const p_data_buffer)
{
    ssp_err_t err = SSP_SUCCESS;
    SSP_ASSERT(NULL != p_data_buffer);
    SSP_ASSERT(NULL != p_ctrl);
    JPEG_ERROR_RETURN((JPEG_ENCODE_STATUS_FREE != p_ctrl->status), SSP_ERR_NOT_OPEN);
    JPEG_ERROR_RETURN(!((uint32_t) p_data_buffer & JPEG_ALIGNMENT_8), SSP_ERR_INVALID_ALIGNMENT);
    return err;
}
#endif

/*******************************************************************************************************************//**
 * @brief       JPEG internal function: Output data correction function.
 *
 * @param[in]   p_ctrl        Pointer to control block structure.
 * @param[in]   final_output  Number bytes need to be corrected
 * @retval      None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_data_correction(jpeg_encode_instance_ctrl_t * p_ctrl, uint8_t final_output)
{
    uint8_t *p_end_data = NULL;
    uint8_t final_data[6] = {0U};
    uint8_t initial_data[6] = {0U};
    uint8_t *p_start_data = NULL;
    uint8_t i;

    /*In this function because remainder is 1 to 6 bytes */
    p_start_data = (uint8_t *)HW_JPEG_EncodeDestinationAddressGet(p_ctrl->p_reg);
    p_end_data   = (uint8_t *)((uint8_t *)(HW_JPEG_EncodeDestinationAddressGet(p_ctrl->p_reg) + p_ctrl->output_buffer_size) - 1);

    /* read the final remainder bytes */
    for (i = 0U; i < final_output; i++)
    {
        final_data[i] = *(p_end_data - i);
    }

    /* Read the 6 initial bytes */
    for (i =0U; i < 6U; i++)
    {
        initial_data[i] = *(p_start_data + i);
    }

    /* Is the final coded data EOI (0xFFD9)? */
    if ((0xD9U != final_data[0]) && (0xFFU != final_data[1]))
    {
        /* Correct final data */
        for(i = 0U; i < final_output; i++)
        {
            *(p_end_data - ((final_output - 1U) + i)) = initial_data[i];
        }
    }

    /* Correct data at JIFEDA address */
    r_jpeg_encode_correct_initial_data(p_start_data,initial_data);
}
/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Correct data at JIFEDA address
 *
 * @param[in]    p_initial_data        Pointer initial data.
 * @param[in]    p_start_data          pointer to starting data in JIFEDA
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_encode_correct_initial_data(uint8_t * p_start_data, uint8_t *p_initial_data)
{
    if ((0xFFU != p_initial_data[0]) && (0xD8U != p_initial_data[1]) && (0xFFU != p_initial_data[2]) && (0xDBU != p_initial_data[3])
            && (0x00U != p_initial_data[4]) && (0x84U != p_initial_data[5]))
    {
        *p_start_data = 0xFFU;
        *(p_start_data + 1) = 0xD8U;
        *(p_start_data + 2) = 0xFFU;
        *(p_start_data + 3) = 0xDBU;
        *(p_start_data + 4) = 0x00U;
        *(p_start_data + 5) = 0x84U; /* Quantization table 2 */
    }
}
/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Modifies the standard Quantization table based on quality factor and uploads
 *          it to the JPEG internal register.
 *
 * @param[in]    p_cfg        Pointer to config structure.
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_upload_quantization_table(jpeg_encode_cfg_t const * const p_cfg)
{
    uint16_t new_table_value = 0U;
    uint16_t multiply_factor = 0U;
    int16_t i;

    /* Multiplication factor is required to calculate new values from base (standard) table */
    r_jpeg_get_multiply_factor(p_cfg->quality_factor, &multiply_factor);

    /** Init quantization tables. */
    for(i = 0; i < QUANT_TABLE_SIZE; i++)
    {
        new_table_value = (uint16_t)(((p_cfg->p_quant_luma_table[i] * multiply_factor) + 50)/100);

        if(new_table_value < 1U)
        {
            new_table_value = 1U;
        }
        else if(new_table_value > 255U)
        {
            new_table_value = 255U;
        }
        else
        {
            /* Do nothing */
        }
        R_JPEG_QUANT_TABLE->JCQTBL0[i] = (uint8_t)new_table_value;
    }

    for(i = 0; i < QUANT_TABLE_SIZE; i++)
    {
        new_table_value = (uint16_t)(((p_cfg->p_quant_croma_table[i] * multiply_factor) + 50)/100);

        if(new_table_value < 1U)
        {
            new_table_value = 1U;
        }
        else if(new_table_value > 255U)
        {
            new_table_value = 255U;
        }
        else
        {
            /* Do nothing */
        }
        R_JPEG_QUANT_TABLE->JCQTBL1[i] = (uint8_t)new_table_value;
    }
}

/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Updates the JPEG internal Huffman table registers with JPEG standard Huffman tables
 *
 * @param[in]     p_cfg     Pointer to config structure.
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_upload_huffman_table(jpeg_encode_cfg_t const * const p_cfg)
{
    int16_t i;

    /* Init huffman tables. */
    for(i = 0; i < HUFFM_DC_TABLE_SIZE; i++)
    {
        R_JPEG_HUFFM_TABLE->JCHTBD0[i] = p_cfg->p_huffman_luma_dc_table[i];
    }

    for(i = 0; i < HUFFM_DC_TABLE_SIZE; i++)
    {
        R_JPEG_HUFFM_TABLE->JCHTBD1[i] = p_cfg->p_huffman_croma_dc_table[i];
    }


    for(i = 0; i < HUFFM_AC_TABLE_SIZE; i++)
    {
        R_JPEG_HUFFM_TABLE->JCHTBA0[i] = p_cfg->p_huffman_luma_ac_table[i];
    }

    for(i = 0; i < HUFFM_AC_TABLE_SIZE; i++)
    {
        R_JPEG_HUFFM_TABLE->JCHTBA1[i] = p_cfg->p_huffman_croma_ac_table[i];
    }
}

/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Calculate the multiplication factor from the quality factor, used by
 *          r_jpeg_upload_quantization_table internal API.
 *
 * @param[in]       quality_factor      Quality factor value.
 * @param[out]      p_multiply_factor   Multiplication factor value
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_get_multiply_factor(uint8_t quality_factor, uint16_t *p_multiply_factor)
{
    if(quality_factor > 100U)
    {
        quality_factor = 100U;
        *p_multiply_factor = (uint16_t)(200U - (quality_factor * 2U));
    }
    else if(quality_factor <= 0U)
    {
        quality_factor = 1U;
        *p_multiply_factor = (uint16_t)(5000U/quality_factor);
    }
    else if(quality_factor < 50U)
    {
        *p_multiply_factor = (uint16_t)(5000U/quality_factor);
    }
    else
    {
        *p_multiply_factor = (uint16_t)(200U - (quality_factor * 2U));
    }
}
/*******************************************************************************************************************//**
 * @brief Start JPEG Compression. The JPEG hardware will be set to the Input Count mode if total lines to encode
 *  is greater than or equal to vertical size of image, else it will not be set to the Input Count mode and the
 *  JPEG hardware will not halt
 *
 * @param[in]   p_ctrl               Pointer to control block structure.
 *
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void r_jpeg_encode_input_start(jpeg_encode_instance_ctrl_t * p_ctrl)
{
    /** JPEG driver assume the entire image is present if vertical resolution set by user is equal to lines to encode */
    /** In this case do not enable input count mode */
    if(p_ctrl->lines_to_encoded == p_ctrl->vertical_resolution)
    {
        HW_JPEG_EncodeInCountModeDisable(p_ctrl->p_reg);
    }
    else
    {
        /** Configure the line count and address resume mode */
        HW_JPEG_EncodeInputCountModeConfig(p_ctrl->p_reg, JPEG_ENCODE_COUNT_MODE_ADDRESS_REINITIALIZE,
                                           p_ctrl->lines_to_encoded);
        /** Enable count mode */
        HW_JPEG_EncodeInCountModeEnable(p_ctrl->p_reg);
    }
    HW_JPEG_Encoder_start(p_ctrl->p_reg);
}
/*******************************************************************************************************************//**
 * @brief   JPEG internal function: Data Transfer Interrupt (JDTI) Interrupt callback handler.
 *
 * @param[in]       p_common_ctrl       Pointer to void type
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void jpeg_encoder_jdti_isr(void * const p_common_ctrl)
{
    jpeg_encode_instance_ctrl_t *p_ctrl = p_common_ctrl;
    uint8_t remainder = 0;
    uint32_t intertype = 0;
    /** Get the Interrupt status flag */
    intertype = HW_JPEG_InterruptStatus1Get(p_ctrl->p_reg);

    /** Clear interrupt status flag */
    HW_JPEG_InterruptStatus1Set(p_ctrl->p_reg, 0x0U);

    /** Clear the interrupt flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /** Data transfer complete */
    if (intertype & JPEG_INTE1_CBTF)
    {
        /** Record the output buffer size */
        p_ctrl->output_buffer_size = HW_JPEG_EncoderCompressedLineGet(p_ctrl->p_reg);

        /** Data correction */
        remainder = (p_ctrl->output_buffer_size % 8);

        /** Data correction decision, if remainder is 1 to 6 bytes */
        if (((remainder) > 0U) && ((remainder) < 7U))
        {
            r_jpeg_data_correction(p_ctrl, remainder);
        }

        /** Clear internal status information. */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE)));
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_RUNNING)));

        /** Set the JPEG status to DONE */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_ENCODE_STATUS_DONE));
        p_ctrl->encoded_lines = 0U;
        if (NULL != p_ctrl->p_callback)
        {
            jpeg_encode_callback_args_t args;
            args.image_size = p_ctrl->output_buffer_size;
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }
    /** Count mode interrupt occurred */
    else if(intertype & JPEG_INTE1_DINLF)
    {
        /*Number of lines encoded*/
        p_ctrl->encoded_lines = (uint16_t)(p_ctrl->encoded_lines + p_ctrl->lines_to_encoded);

        /** Record the output buffer size */
        p_ctrl->output_buffer_size = HW_JPEG_EncoderCompressedLineGet(p_ctrl->p_reg);

        /** Clear internal status information. */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_RUNNING)));

        /** Set the ctrl status.  */
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status | (uint32_t)(JPEG_ENCODE_STATUS_INPUT_PAUSE));

        /* Invoke the callback with INPUT_PAUSE status */
        if ((NULL != p_ctrl->p_callback) &&
                ((p_ctrl->encoded_lines) <= (p_ctrl->vertical_resolution - p_ctrl->lines_to_encoded)))
        {
            jpeg_encode_callback_args_t args;
            args.image_size = p_ctrl->output_buffer_size;
            args.status = p_ctrl->status;
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }
}
/*******************************************************************************************************************//**
 * @brief   JPEG internal function: JPEG Decompression Process Interrupt (JEDI) Interrupt callback handler.
 *
 * @param[in]       p_common_ctrl       Pointer to void type
 * @retval  None
 * @note This is a private function in the driver module so not check the input parameter.
 **********************************************************************************************************************/
static void jpeg_encoder_jedi_isr(void * const p_common_ctrl)
{
    jpeg_encode_instance_ctrl_t *p_ctrl = p_common_ctrl;
    uint32_t intertype =0;

    /** Get the Interrupt status flag */
    intertype = HW_JPEG_InterruptStatus0Get(p_ctrl->p_reg);

    /** Clear interrupt status flag */
    HW_JPEG_InterruptStatus0Set(p_ctrl->p_reg,0);

    /** Clear the request.  */
    HW_JPEG_ClearRequest(p_ctrl->p_reg);

    /** Clear the interrupt flag. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    if (JPEG_INTE0_INS6 & intertype)
    {
        p_ctrl->status = (jpeg_encode_status_t)((uint32_t)p_ctrl->status & (~(uint32_t)(JPEG_ENCODE_STATUS_RUNNING)));
    }
}
