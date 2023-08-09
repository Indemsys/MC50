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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/**  NetX Duo Component                                                   */
/**                                                                       */
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** This file selects NetX Duo port specific information based on the MCU */
/** and compiler selected.                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_PORT_H_TOP_LEVEL
#define NX_PORT_H_TOP_LEVEL

#define NX_INCLUDE_USER_DEFINE_FILE
#if   defined(__IAR_SYSTEMS_ICC__)
    #if   (__CORE__ == __ARM7EM__)
        #include "../cm4_iar/nxd/nx_port.h"
    #elif (__CORE__ == __ARM6M__)
        #include "../cm0_iar/nxd/nx_port.h"
    #elif (__CORE__ == __ARM8M_BASELINE__)
        #include "../cm23_iar/nxd/nx_port.h"
    #else
        #warning Unsupported Architecture
    #endif
#elif defined(__GNUC__)
    #if   __ARM_ARCH_7EM__
        #include "./cm4_gcc/nxd/nx_port.h"
    #elif __ARM_ARCH_6M__
        #include "./cm0_gcc/nxd/nx_port.h"
    #elif __ARM_ARCH_8M_BASE__
        #include "./cm23_gcc/nxd/nx_port.h"
    #else
        #warning Unsupported Architecture
    #endif
#else
    #warning Unsupported compiler
#endif

#endif /* NX_PORT_H_TOP_LEVEL */
