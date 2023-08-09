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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

/*LDRA_NOANALYSIS These include files are maintained by Express Logic. */
#include "tx_api.h"
#include "tx_initialize.h"
#include "../tx_thread.h"
#include "../tx_timer.h"
#include "tx_cmsis.h"
/*LDRA_ANALYSIS */

#ifdef __ARM_ARCH_7EM__ // CM4 for GCC
#define TX_PORT_ARM_CM4
#endif
#ifdef __IAR_SYSTEMS_ICC__ //IAR
#if __CORE__ == __ARM7EM__ // CM4 for IAR
#define TX_PORT_ARM_CM4
#endif
#endif

/***********************************************************************//**
 * @defgroup TX_SYNERGY_PORT ThreadX Synergy Port
 * @{
 **************************************************************************/

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_initialize_low_level                             Synergy
 *
 *  DESCRIPTION
 *
 *    This function is responsible for any low-level processor
 *    initialization, including definition of the ThreadX periodic
 *    timer interrupt vector, saving the system stack pointer, and
 *    finding the first available RAM memory address for
 *    tx_application_define.
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
 *    _tx_initialize_kernel_enter           ThreadX entry function
 *
 **************************************************************************/
/*LDRA_INSPECTED 219 S API is defined by Express Logic, must start with
 * underscore. */
VOID   _tx_initialize_low_level(VOID)
{
    /** Ensure that interrupts are disabled.  */
    __disable_irq();

    /** Set base of available memory to end of non-initialized RAM area.  */
    _tx_initialize_unused_memory = TX_UCHAR_POINTER_ADD(TX_FREE_MEMORY_START, 4);

    /** Set system stack pointer from vector value.  */
    /* The vector table is not uninitialized.  It is initialized in a file that is not in the 
     * scope of this set. */
    /*LDRA_INSPECTED 69 D */
    _tx_thread_system_stack_ptr = TX_VECTOR_TABLE[0];

#ifdef TX_PORT_ARM_CM4
    /* Enable the cycle count register.  */
    DWT->CTRL |= (uint32_t) DWT_CTRL_CYCCNTENA_Msk;
#endif

#ifndef TX_NO_TIMER
    /** Configure SysTick based on user configuration (100 Hz by default).  */
    SysTick_Config(SystemCoreClock / TX_TIMER_TICKS_PER_SECOND);
#endif

    /** Configure the handler priorities. */
    NVIC_SetPriority(SVCall_IRQn, 0xFF); // Note: SVC must be lowest priority, which is 0xFF
    NVIC_SetPriority(PendSV_IRQn, 0xFF); // Note: PnSV must be lowest priority, which is 0xFF
#ifndef TX_NO_TIMER
#ifndef TX_PORT_CFG_SYSTICK_IPL
    NVIC_SetPriority(SysTick_IRQn, 0x40);
#else
    NVIC_SetPriority(SysTick_IRQn, TX_PORT_CFG_SYSTICK_IPL); // Set User configured Priority for Systick Interrupt
#endif
#endif

#ifdef TX_VFP_ENABLE
    /** Reset lazy stacking if the MCU has an FPU. */
    __asm(
        "MRS     R12, CONTROL     \n"
        "BIC     R12, R12, #4     \n"      /* Clear FPCA. */
        "MSR     CONTROL, R12     \n"
        "                         \n");
#endif
}

/***********************************************************************//**
 * @} (end defgroup TX_SYNERGY_PORT)
 **************************************************************************/
