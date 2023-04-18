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

#include "tx_api.h"
#include "../tx_thread.h"

/***********************************************************************//**
 * @addtogroup TX_SYNERGY_PORT
 * @{
 **************************************************************************/

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_thread_stack_build                               Synergy
 *
 *  DESCRIPTION
 *
 *    This function builds a stack frame on the supplied thread's stack.
 *    The stack frame results in a fake interrupt return to the supplied
 *    function pointer.
 *
 *  INPUT
 *
 *    thread_ptr                            Pointer to thread control blk
 *    function_ptr                          Pointer to return function
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
 *    _tx_thread_create                     Create thread service
 *
 **************************************************************************/
 
/** This macro is used to suppress compiler messages about a parameter not being used in a function. The nice thing
 * about using this implementation is that it does not take any extra RAM or ROM. */
/*LDRA_INSPECTED 340 s */ 
#define PARAMETER_NOT_USED(p) (void) ((p))
 
VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
{

    PARAMETER_NOT_USED(function_ptr);
    /** Build a fake interrupt frame.  The form of the fake interrupt stack
       on the Cortex-M look like the following after it is built:

       Stack Top:
                #ifdef TX_VFP_ENABLE
                       LR          Interrupted LR (LR at the time of PENDSV)
                #endif
                       r4          Initial value for r4
                       r5          Initial value for r5
                       r6          Initial value for r6
                       r7          Initial value for r7
                       r8          Initial value for r8
                       r9          Initial value for r9
                       r10 (sl)    Initial value for r10 (sl)
                       r11         Initial value for r11
                       r0          Initial value for r0    (Hardware stack starts here!!)
                       r1          Initial value for r1
                       r2          Initial value for r2
                       r3          Initial value for r3
                       r12         Initial value for r12
                       lr          Initial value for lr
                       pc          Initial value for pc
                       xPSR        Initial value for xPSR

    Stack Bottom: (higher memory address)  */

    __asm(
        "LDR     r2, [r0, #16]   \n"    /* Pickup end of stack area       */
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
        "BIC     r2, r2, #0x7    \n"    /* Align frame                    */
        "SUB     r2, r2, #68     \n"    /* Subtract frame size            */
        "LDR     r3, =0xFFFFFFFD \n"    /* Build initial LR value         */
        "STR     r3, [r2, #0]    \n"    /* Save on the stack              */
        "ADD     r2, r2, #4      \n");  /* Build the stack pointer to next location */
#else /* CM0+ */
#if defined(__GNUC__)
        ".syntax unified         \n"
#endif
        "SUBS    r2, r2, #64     \n"    /* Subtract frame size            */
        "MOVS    r3, #0x7        \n"
        "BICS    r2, r2, r3      \n");  /* Align frame                    */
#if defined(__GNUC__)
    __asm(
        ".syntax divided         \n");
#endif
#endif /* CM4 */

    /* Actually build the stack frame.  */

    __asm(
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
        "MOV     r3, #0          \n"    /* Build initial register value   */
#else /* CM0+ */
        "MOVS    r3, #0          \n"    /* Build initial register value   */
#endif /* CM4 */
        "STR     r3, [r2, #0]    \n"    /* Store initial r4               */
        "STR     r3, [r2, #4]    \n"    /* Store initial r5               */
        "STR     r3, [r2, #8]    \n"    /* Store initial r6               */
        "STR     r3, [r2, #12]   \n"    /* Store initial r7               */
        "STR     r3, [r2, #16]   \n"    /* Store initial r8               */
        "STR     r3, [r2, #20]   \n"    /* Store initial r9               */
        "LDR     r3, [r0, #12]   \n"    /* Pickup stack starting address  */
        "STR     r3, [r2, #24]   \n"    /* Store initial r10 (sl)         */
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
        "MOV     r3, #0          \n"    /* Build initial register value   */
#else /* CM0+ */
        "MOVS    r3, #0          \n"    /* Build initial register value   */
#endif /* CM4 */
        "STR     r3, [r2, #28]   \n");  /* Store initial r11              */

    /* The hardware stack follows.  */

    __asm(
        "STR     r3, [r2, #32]   \n"    /* Store initial r0               */
        "STR     r3, [r2, #36]   \n"    /* Store initial r1               */
        "STR     r3, [r2, #40]   \n"    /* Store initial r2               */
        "STR     r3, [r2, #44]   \n"    /* Store initial r3               */
        "STR     r3, [r2, #48]   \n");  /* Store initial r12              */
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
    __asm(
        "MOV     r3, #0xFFFFFFFF \n");  /* Poison EXC_RETURN value        */
#else /* CM0+ */
    __asm(
#if defined(__GNUC__)
        ".syntax unified         \n"
#endif
    /* Do not use syntax ("MOV     r3, %0" : : "r" (0xFFFFFFFF)) - it corrupts r1 on the IAR compiler. */
        "MOVS    r3, #1          \n"    /* Store 0xFFFFFFFF in r3 */
        "NEGS    r3, r3          \n");
#endif /* CM4 */
    __asm(
        "STR     r3, [r2, #52]   \n"    /* Store initial lr               */
        "STR     r1, [r2, #56]   \n");  /* Store initial pc               */
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
    __asm(
        "MOV     r3, #0x01000000 \n");  /* Only T-bit need be set         */
#else /* CM0+ */
    __asm(
        "MOVS    r3, #128        \n"
        "LSLS    r3, r3, #17     \n");  /* Only T-bit need be set, use r3 only */
#if defined(__GNUC__)
    __asm(
        ".syntax divided         \n");
#endif
#endif /* CM4 */
    __asm(
        "STR     r3, [r2, #60]   \n");  /* Store initial xPSR             */

    /* Setup the stack pointer.  */

    __asm(
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
        "SUB     r2, r2, #4      \n"    /* Build the stack frame address  */
#endif
        "STR     r2, [r0, #8]    \n");  /* Save stack pointer in thread's */
                                         /*  control block                 */

    PARAMETER_NOT_USED(thread_ptr);
}

/***********************************************************************//**
 * @} (end addtogroup TX_SYNERGY_PORT)
 **************************************************************************/
