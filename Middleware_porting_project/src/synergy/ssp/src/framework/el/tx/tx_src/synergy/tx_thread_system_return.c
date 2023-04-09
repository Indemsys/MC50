/*LDRA_INSPECTED 119 S */
/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#ifdef TX_DISABLE_INLINE

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

/*LDRA_NOANALYSIS These include files are maintained by Express Logic. */
#include "tx_api.h"
#include "../tx_thread.h"
#include "../tx_timer.h"
#include "tx_cmsis.h"
/*LDRA_ANALYSIS */

/***********************************************************************//**
 * @addtogroup TX_SYNERGY_PORT
 * @{
 **************************************************************************/

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_thread_system_return                             Synergy
 *
 *  DESCRIPTION
 *
 *    This function is target processor specific.  It is used to transfer
 *    control from a thread back to the ThreadX system.  Only a
 *    minimal context is saved since the compiler assumes temp registers
 *    are going to get slicked by a function call anyway.
 *
 *  INPUT
 *
 *    None
 *
 *  OUTPUT
 *
 *    None
 *
 *  CALLS
 *
 *    None
 *
 *  CALLED BY
 *
 *    ThreadX components
 *
 **************************************************************************/
/*LDRA_INSPECTED 219 S API is defined by Express Logic, must start with
 * underscore. */
VOID   _tx_thread_system_return(VOID)
{
    /** Set PENDSVBIT in ICSR */
    /* CMSIS code is mixing unsigned long and unsigned int types
     * here.  Unsigned long and unsigned int are the same width for
     * Cortex M compilers, so this is safe.  We are not modifying
     * CMSIS code, so allow an exception here. */
    /*LDRA_INSPECTED 96 S *//*LDRA_INSPECTED 93 S */
    /*LDRA_INSPECTED 330 S *//*LDRA_INSPECTED 330 S */
    SCB->ICSR = (uint32_t) SCB_ICSR_PENDSVSET_Msk;

    /** If a thread is returning, briefly enable and restore interrupts. */
    if (__get_IPSR() == 0)
    {
        UINT int_posture;
        int_posture = __get_PRIMASK();
        __enable_irq();
        __set_PRIMASK(int_posture);
    }
}

#endif /* TX_DISABLE_INLINE */

/***********************************************************************//**
 * @} (end addtogroup TX_SYNERGY_PORT)
 **************************************************************************/
