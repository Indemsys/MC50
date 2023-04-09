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


/* #define TX_SOURCE_CODE  */


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

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_thread_interrupt_control                         Synergy
 *
 *  DESCRIPTION
 *
 *    This function is responsible for changing the interrupt lockout
 *    posture of the system.
 *
 *  INPUT
 *
 *    new_posture                           New interrupt lockout posture
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
UINT   _tx_thread_interrupt_control(UINT new_posture)
{

UINT old_posture;

    /** Pickup current interrupt lockout posture.  */
    old_posture = __get_PRIMASK ();

    /** Apply the new interrupt posture.  */
    __set_PRIMASK(new_posture);

    /** Return the original value */
    return old_posture;
}

/***********************************************************************//**
 * @} (end addtogroup TX_SYNERGY_PORT)
 **************************************************************************/
