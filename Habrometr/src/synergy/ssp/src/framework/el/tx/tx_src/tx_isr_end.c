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
/**   ISR                                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#include "tx_api.h"
#ifdef TX_ENABLE_EVENT_TRACE
#include "tx_trace.h"
#endif
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
#include "tx_execution_profile.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    tx_isr_end                                         Synergy/GNU      */ 
/*                                                           5.0          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is responsible for indicating the end of processing   */
/*    for the specified interrupt. This is purely for diagnostic use with */ 
/*    either TraceX or execution profiling.                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_trace_isr_exit_insert         Mark the end of an ISR            */ 
/*    _tx_execution_isr_exit            Mark the end of ISR processing    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ISR                                                                 */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-01-2015     William E. Lamie         Initial Version 5.0           */
/*                                                                        */ 
/**************************************************************************/ 

/** This macro is used to suppress compiler messages about a parameter not being used in a function. The nice thing
 * about using this implementation is that it does not take any extra RAM or ROM. */
/*LDRA_INSPECTED 340 s */
#define PARAMETER_NOT_USED(p) (void) ((p))
 
VOID  tx_isr_end(ULONG isr_id)
{

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
TX_INTERRUPT_SAVE_AREA
#endif

#ifdef TX_ENABLE_EVENT_TRACE

    /* Indicate the end of the ISR in the trace buffer.  */
    _tx_trace_isr_exit_insert(isr_id);
#else
    PARAMETER_NOT_USED(isr_id);
#endif    
    
#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))

    /* Disable interrupts.  */
    TX_DISABLE
    
    /* Indicate the end of ISR processing.  */
    _tx_execution_isr_exit();

    /* Restore interrupts.  */
    TX_RESTORE
#endif
}
