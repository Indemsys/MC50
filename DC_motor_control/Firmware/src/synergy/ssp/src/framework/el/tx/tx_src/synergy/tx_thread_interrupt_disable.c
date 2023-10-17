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
#include "tx_cmsis.h"
/*LDRA_ANALYSIS */

/***********************************************************************//**
 * @addtogroup TX_SYNERGY_PORT
 * @{
 **************************************************************************/

/*LDRA_INSPECTED 219 S API is defined by Express Logic, must start with
 * underscore. */
UINT   _tx_thread_interrupt_disable(VOID);

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_thread_interrupt_restore                         Synergy
 *
 *  DESCRIPTION
 *
 *    This function is responsible for disabling interrupts and returning
 *    the previous interrupt lockout posture.
 *
 *  INPUT
 *
 *    None
 *
 *  OUTPUT
 *
 *    old_posture                           Old interrupt lockout posture
 *
 *  CALLS
 *
 *    None
 *
 *  CALLED BY
 *
 *    Application Code
 *
 **************************************************************************/
/*LDRA_INSPECTED 219 S API is defined by Express Logic, must start with
 * underscore. */
UINT   _tx_thread_interrupt_disable(VOID)
{
UINT old_posture;

    /** Pickup current interrupt lockout posture.  */
    old_posture = __get_PRIMASK ();

    /** Disable interrupts */
    __disable_irq();

    /** Return old interrupt lockout posture.  */
    return old_posture;
}

/***********************************************************************//**
 * @} (end addtogroup TX_SYNERGY_PORT)
 **************************************************************************/
