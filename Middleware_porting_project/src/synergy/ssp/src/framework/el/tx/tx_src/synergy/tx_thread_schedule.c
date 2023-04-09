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
#include "../tx_timer.h"
#include "tx_cmsis.h"

/* Variable used to hold the end of memory location.  */
#if   defined(__IAR_SYSTEMS_ICC__)
#pragma section="FREE_MEM"
/* Use &__tx_free_memory_start to get start of free RAM. */
__root void * __tx_free_memory_start @ "FREE_MEM";
#endif

#define SBYCR_SSBY_SLEEP_MODE               (0U)
#define SBYCR_SSBY_SOFTWARE_STANDBY_MODE    (1U)
#define PRCR_UNLOCK_LPM_REGISTER_ACCESS     (0xA502U)
#define PRCR_LOCK_LPM_REGISTER_ACCESS       (0xA500U)

#ifdef __ARM_ARCH_7EM__ //CM4
#define TX_PORT_ISA_CBZ_SUPPORTED
#define TX_PORT_ISA_STMDB_LDMIA_SUPPORTED
#define TX_PORT_ISA_THUMB2_SUB_ADD_SUPPORTED
#endif

#ifdef __ARM_ARCH_8M_BASE__ //CM23
#define TX_PORT_ISA_CBZ_SUPPORTED
#endif

/* This must follow the definition for each core instruction set. IAR does not support CBZ in inline assembly. */
#ifdef __IAR_SYSTEMS_ICC__
#undef TX_PORT_ISA_CBZ_SUPPORTED
#endif

/* Used to generate a compiler error (negative array size error) if the assertion fails.  This is used in place of "#error"
 * for expressions that cannot be evaluated by the preprocessor like offsetof(). */
#define SSP_COMPILE_TIME_ASSERT(e) ((void) sizeof(char[1 - 2 * !(e)]))

/* Used to convert an evaluated macro into a string. */
#define SSP_STRINGIFY_EXPANDED(s) SSP_STRINGIFY(s)

/* Used to convert text into a string. */
#define SSP_STRINGIFY(s) #s

/* The following macros are defined as hard coded numbers so they can be converted to strings in the
 * assembly language of PendSV_Handler. They are verified to be correct using compile time assertions
 * at the beginning of PendSV_Handler. */

/* The following offsets are relative to the base of the TX_THREAD structure.  For example:
   offsetof(TX_THREAD, tx_thread_run_count) == 4 */
#define TX_PORT_OFFSET_RUN_COUNT   4
#define TX_PORT_OFFSET_STACK_PTR   8
#define TX_PORT_OFFSET_STACK_START 12
#define TX_PORT_OFFSET_STACK_END   16
#define TX_PORT_OFFSET_TIME_SLICE  24

/* This is the base address for the SPMON registers. */
#define TX_PORT_SPMON_BASE       0x40000D00

/* The following offsets are relative to the base of the R_SPMON_Type structure.  For example:
   offsetof(R_SPMON_Type, PSPMPUCTL) == 20 */
#define TX_PORT_OFFSET_PSPMPUCTL 20
#define TX_PORT_OFFSET_PSPMPUSA  24
#define TX_PORT_OFFSET_PSPMPUEA  28

/* The following define the address offset of each entry into the data table passed into PendSV_Handler for IAR. These
 * defines are stringified directly into the assembly code, and used to initialize the data table g_pendsv_data. */
#define TX_PORT_ADDR_OFFSET_EXIT_FUNCTION   0
#define TX_PORT_ADDR_OFFSET_ENTER_FUNCTION  4

/* This macro converts the address offset to an array index into g_pendsv_data. */
#define TX_PORT_INDEX(x)                    (x / sizeof(uint32_t))

TX_FUNCTION_NAKED VOID PendSV_Handler (VOID);
VOID SysTick_Handler(VOID);

extern TX_THREAD * volatile _tx_thread_current_ptr;
extern TX_THREAD * volatile _tx_thread_execute_ptr;
extern volatile UINT        _tx_thread_preempt_disable;

VOID   _tx_thread_schedule(VOID);

void _tx_port_wait_thread_ready(void);

#if defined(__IAR_SYSTEMS_ICC__) && (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
VOID _tx_execution_thread_enter(void);
VOID _tx_execution_thread_exit(void);

/* Table of input data to the inline assembly in PendSV_Handler for the IAR compiler.
 * WARNING: If anything is inserted in this table, all inline assembly
 * offsets used to index into this table must be reevaluated. */
static uint32_t const g_pendsv_data[] =
{
    [TX_PORT_INDEX(TX_PORT_ADDR_OFFSET_EXIT_FUNCTION)]   = (uint32_t) _tx_execution_thread_exit,
    [TX_PORT_INDEX(TX_PORT_ADDR_OFFSET_ENTER_FUNCTION)]  = (uint32_t) _tx_execution_thread_enter,
};
#endif

/***********************************************************************//**
 * @addtogroup TX_SYNERGY_PORT
 * @{
 **************************************************************************/

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_thread_schedule                                  Synergy
 *
 *  DESCRIPTION
 *
 *    This function waits for a thread control block pointer to appear in
 *    the _tx_thread_execute_ptr variable. Once a thread pointer appears
 *    in the variable, the corresponding thread is resumed.
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
 *    _tx_initialize_kernel_enter          ThreadX entry function
 *    _tx_thread_system_return             Return to system from thread
 *
 **************************************************************************/
 VOID   _tx_thread_schedule(VOID)
{
     /* The following compile time assertions validate offsets used in the assembly code
      * of PendSV_Handler.  These assertions cannot be in PendSV_Handler because only
      * assembly code is allowed in naked/stackless functions.  These statements generate
      * build errors if the offset of any of the structure members used in this code changes.
      * These structure offsets are hardcoded because offsetof() is not supported in inline
      * assembly. */
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_RUN_COUNT == offsetof(TX_THREAD, tx_thread_run_count));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_STACK_PTR == offsetof(TX_THREAD, tx_thread_stack_ptr));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_STACK_START == offsetof(TX_THREAD, tx_thread_stack_start));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_STACK_END == offsetof(TX_THREAD, tx_thread_stack_end));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_TIME_SLICE == offsetof(TX_THREAD, tx_thread_time_slice));

#ifdef TX_HW_STACK_MONITOR_ENABLE
     SSP_COMPILE_TIME_ASSERT(TX_PORT_SPMON_BASE == (uint32_t) R_SPMON);
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_PSPMPUCTL == offsetof(R_SPMON_Type, PSPMPUCTL));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_PSPMPUSA == offsetof(R_SPMON_Type, PSPMPUSA));
     SSP_COMPILE_TIME_ASSERT(TX_PORT_OFFSET_PSPMPUEA == offsetof(R_SPMON_Type, PSPMPUEA));
#endif

    /** This function should only ever be called on Cortex-M from the first
       schedule request. Subsequent scheduling occurs from the PendSV handling
       routine below. */

    /** Clear the preempt-disable flag to enable rescheduling after initialization on Cortex-M targets.  */
    _tx_thread_preempt_disable = 0;


#ifdef TX_HW_STACK_MONITOR_ENABLE

    /* Setup the Synergy HW Stack Monitor for the MSP stack and setup default
       values for the PSP stack.  */

#if   defined(__IAR_SYSTEMS_ICC__)
    #pragma section=".stack"
#elif defined(__GNUC__)
    extern unsigned int __StackLimit;
    extern unsigned int __StackTop;
#endif

    R_SPMON->MSPMPUCTL = 0;                                    /* Disable MSP monitoring  */
    R_SPMON->PSPMPUCTL = 0;                                    /* Disable PSP monitoring  */
    R_SPMON->MSPMPUOAD = 0;                                    /* Setup NMI interrupt  */
    R_SPMON->PSPMPUOAD = 0;                                    /* Setup NMI interrupt  */
#if   defined(__IAR_SYSTEMS_ICC__)
    R_SPMON->MSPMPUSA = (uint32_t)__section_begin(".stack");   /* Setup start address  */
    R_SPMON->MSPMPUEA = (uint32_t)__section_end(".stack") - 1U;/* Setup end address  */
    R_SPMON->PSPMPUSA = (uint32_t)__section_begin(".stack");   /* Setup start address  */
    R_SPMON->PSPMPUEA = (uint32_t)__section_end(".stack") - 1U;/* Setup end address  */
#elif defined(__GNUC__)
    R_SPMON->MSPMPUSA = (uint32_t)&__StackLimit;               /* Setup start address  */
    R_SPMON->MSPMPUEA = (uint32_t)&__StackTop - 1U;            /* Setup end address  */
    R_SPMON->PSPMPUSA = (uint32_t)&__StackLimit;               /* Setup start address  */
    R_SPMON->PSPMPUEA = (uint32_t)&__StackTop - 1U;            /* Setup end address  */
#endif
    __set_PSP(__get_MSP());                                 /* Set the PSP to the MSP as a default  */
    R_ICU->NMIER |= 0x1000;                                 /* Set SPEEN bit to enable NMI on stack monitor exception */
    R_SPMON->MSPMPUCTL = 1;                                 /* Enable MSP monitoring  */
    R_SPMON->PSPMPUCTL = 1;                                 /* Enable PSP monitoring  */

#endif


    /** Enable interrupts */
    __enable_irq();


    /** Enter the scheduler for the first time.  */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;


    /* We should never get here - ever!  */
    while(1)
    {
#ifdef TX_ENABLE_BKPT
        __BKPT(0xEF);
#endif
    }
}

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    _tx_port_wait_thread_ready                           Synergy
 *
 *  DESCRIPTION
 *
 *    Waits for a new thread to become ready.
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
 *    PendSV_Handler
 *
 **************************************************************************/
/* These variables are global because this function is called from PendSV_Handler, which is a stackless
 * function. */
static volatile uint32_t g_sbycr_ssby = 0U;
static volatile uint32_t g_prcr = 0U;
__attribute__((used)) void _tx_port_wait_thread_ready(void)
{
    /* The following is the idle wait processing. In this case, no threads are ready for execution and the
       system will simply be idle until an interrupt occurs that makes a thread ready. Note that interrupts
       are disabled to allow use of WFI for waiting for a thread to arrive.  */

    while(1)
    {
        /* Disable interrupts - The next block is critical.  */
        __disable_irq();

        /* Make the new thread the current thread. */
        _tx_thread_current_ptr = _tx_thread_execute_ptr;

        /* If non-NULL, a new thread is ready! */
        if (_tx_thread_current_ptr != 0)
        {
            /* Interrupts are re-enabled after returning to PendSV_Handler. */
            break;
        }

        /* Save LPM Mode */
        g_sbycr_ssby = R_SYSTEM->SBYCR_b.SSBY;

        /** Check if the LPM peripheral is set to go to Software Standby mode with WFI instruction.
         *  If yes, change the LPM peripheral to go to Sleep mode. Otherwise skip following procedures
         *  to avoid the LPM register access which is high latency and impacts kernel performance.
         */
        if (SBYCR_SSBY_SOFTWARE_STANDBY_MODE == g_sbycr_ssby)
        {
            /* Save register protect value */
            g_prcr = R_SYSTEM->PRCR;

            /* Unlock LPM peripheral registers */
            R_SYSTEM->PRCR = PRCR_UNLOCK_LPM_REGISTER_ACCESS;

            /* Clear to set to sleep low power mode (not standby or deep standby) */
            R_SYSTEM->SBYCR_b.SSBY = SBYCR_SSBY_SLEEP_MODE;

            /* Restore register lock */
            R_SYSTEM->PRCR = (uint16_t)(PRCR_LOCK_LPM_REGISTER_ACCESS | g_prcr);
        }

        /**
         * DSB should be last instruction executed before WFI
         * infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHICBGB.html
         */
        __DSB();

        /* If there is a pending interrupt (wake up condition for WFI is true), the MCU does not enter low power mode:
         * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/BABHHGEB.html
         * Note that interrupt will bring the CPU out of the low power mode.  After exiting from low power mode,
           interrupt will be re-enabled. */
        __WFI();

        /* Instruction Synchronization Barrier. */
        __ISB();

        /** Re-enable interrupts */
        __enable_irq();

        /** Check if the LPM peripheral was supposed to go to Software Standby mode with WFI instruction.
         *  If yes, restore the LPM peripheral setting. Otherwise skip following procedures to avoid the
         *  LPM register access which is high latency and impacts kernel performance.
         */
        if (SBYCR_SSBY_SOFTWARE_STANDBY_MODE == g_sbycr_ssby)
        {
            /* Disable interrupts - The next block is critical.  */
            __disable_irq();

            /* Save register protect value */
            g_prcr = R_SYSTEM->PRCR;

            /* Unlock LPM peripheral registers */
            R_SYSTEM->PRCR = PRCR_UNLOCK_LPM_REGISTER_ACCESS;

            /* Restore LPM Mode */
            R_SYSTEM->SBYCR_b.SSBY = SBYCR_SSBY_SOFTWARE_STANDBY_MODE;

            /* Restore register lock */
            R_SYSTEM->PRCR = (uint16_t)(PRCR_LOCK_LPM_REGISTER_ACCESS | g_prcr);

            /* Re-enable interrupts */
            __enable_irq();
        }
    }

    /* At this point, we have a new thread ready to go.  */

    /* Clear any newly pended PendSV - since we are already in the handler!  */
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
}

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    PendSV_Handler                                       Synergy
 *
 *  DESCRIPTION
 *
 *    This ISR is responsible for scheduling and context switches.
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
 *    _tx_execution_thread_exit
 *    _tx_port_wait_thread_ready
 *    _tx_execution_thread_enter
 *
 *  CALLED BY
 *
 *    N/A
 *
 **************************************************************************/
TX_FUNCTION_NAKED VOID PendSV_Handler (VOID)
{
    /* This function is assembly only to meet the requirements for naked/stackless functions. For GCC, only basic
     * assembly is used. The main stack is used when C functions are called. */

    /* Before entry into this exception handler, the hardware stacks XPSR, PC, LR, r12, r3, r2, r1, and r0 onto the
     * stack (typically the process stack of the executing thread). When the FPU is in use, the FPSCR and S0-S15
     * registers are also stored on the stack. All other registers are stored by software in this function. */

    /* Only r0, r1, r3 and r12 can be used before stack preservation is complete. r2 is reserved for passing inputs
     * when the IAR compiler is used.  The input table pointer is moved from r2 to r7 after stack preservation and
     * before any C function is called. */

    /* r0-r3 are not guaranteed to retain their values after C functions are called. */

    /* An exception is granted in this case to allow commented out code.  Commented out equivalent C code is
     * included before each section of assembly code that has a C equivalent.  This is done to improve readability
     * and maintainability.  Commented out equivalent C code is in single line comments with no code on the
     * same line. */

    __asm volatile (
#if defined(__GNUC__)
        ".syntax unified                 \n"
#endif

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
        /* Call the thread exit function to indicate the thread is no longer executing.  */
        // _tx_execution_thread_exit();
        "CPSID   i                           \n" // Disable interrupts
        "PUSH    {r0, lr}                    \n" // Save LR since it has been stored yet, and r0 to maintain double word stack alignment
    #if !defined(__IAR_SYSTEMS_ICC__)
        "BL      _tx_execution_thread_exit   \n" // Call the thread exit function
    #else
        /* Inputs for IAR are stored in R12 to ensure they are preserved when
         * _tx_execution_thread_exit is called. The input table is forced into r2 using
         * clobber registers, and any function call could overwrite the contents of
         * r2.  r12 is a scratch register and will not be overwritten by the compiler. */
        "MOV     r12, %[input_table]         \n"

        /* g_pendsv_data[TX_PORT_INDEX(TX_PORT_ADDR_OFFSET_EXIT_FUNCTION)] = _tx_execution_thread_exit */
        "LDR     r1, [%[input_table], #"SSP_STRINGIFY_EXPANDED(TX_PORT_ADDR_OFFSET_EXIT_FUNCTION)"] \n"
        "BLX     r1                          \n"

        /* Restore IAR input table from R12. */
        "MOV     %[input_table], r12         \n"
    #endif
        /* r0-r3 are undefined after branches. */
#if (defined(__IAR_SYSTEMS_ICC__) && (__CORE__ == __ARM7EM__)) || defined(__ARM_ARCH_7EM__) // CM4
        "POP     {r0, lr}                    \n" // Restore r0 and LR
#else
        "POP     {r0, r1}                    \n" // Restore r0 and LR
        "MOV     lr,r1                       \n"
#endif
        "CPSIE   i                           \n" // Enable interrupts
#endif

        /** Determine if there is a current thread to finish preserving.  */
        // if (_tx_thread_current_ptr != 0)
        // {
        "LDR     r0, =_tx_thread_current_ptr \n" // Build current thread pointer address
        "LDR     r1, [r0]                    \n" // Pickup current thread pointer
        "CMP     r1, #0                      \n"

        /* This branch is only taken the first time this function is called.  After that, there is always
         * a current thread to save. */
        "BEQ     __tx_no_current             \n"

            /* Recover PSP and preserve current thread context.  */
            "MRS     r3,  PSP                \n"   /* Pickup PSP pointer (thread's stack pointer) */
#ifdef TX_HW_STACK_MONITOR_ENABLE
#ifdef TX_PORT_ISA_THUMB2_SUB_ADD_SUPPORTED
            "SUB     r3,  r3, #32            \n"   /* Update PSP to utilize the HW stack monitor with the amount of room required to save r4-r11  */
            "MSR     PSP, r3                 \n"   /* Update PSP to utilize the HW stack monitor  */
            "ADD     r3,  r3, #32            \n"   /* Restore original PSP to r3 so we can continue stacking  */
#else
            "SUBS    r3,  r3, #32            \n"   /* Update PSP to utilize the HW stack monitor with the amount of room required to save r4-r11  */
            "MSR     PSP, r3                 \n"   /* Update PSP to utilize the HW stack monitor  */
            "ADDS    r3,  r3, #32            \n"   /* Restore original PSP to r3 so we can continue stacking  */
#endif
#endif
#ifdef TX_PORT_ISA_STMDB_LDMIA_SUPPORTED
            "STMDB   r3!, {r4-r11}           \n"   /* Save its remaining registers  */
#else /* TX_PORT_ISA_STMDB_LDMIA_SUPPORTED */
            "SUBS    r3, r3, #16             \n"   /* Allocate stack space */
            "STM     r3!, {r4-r7}            \n"   /* Save its remaining registers (M3 Instruction: STMDB r3!, {r4-r11}) */
            "MOV     r4,r8                   \n"
            "MOV     r5,r9                   \n"
            "MOV     r6,r10                  \n"
            "MOV     r7,r11                  \n"
            "SUBS    r3, r3, #32             \n"   /* Allocate stack space */
            "STM     r3!,{r4-r7}             \n"
            "SUBS    r3, r3, #16             \n"   /* Allocate stack space */
#endif /* TX_PORT_ISA_STMDB_LDMIA_SUPPORTED */
#ifdef TX_VFP_ENABLE
            "TST     LR, #0x10               \n"   /* Determine if the VFP extended frame is present  */
            "BNE     _skip_vfp_save          \n"   /* No, skip additional VFP save  */
    #ifdef TX_HW_STACK_MONITOR_ENABLE
            "SUB     r3,  r3, #64            \n"   /* Calculate the amount of room required to save s16-s31  */
            "MSR     PSP, r3                 \n"   /* Update PSP to utilize the HW stack monitor  */
            "ADD     r3,  r3, #64            \n"   /* Restore original PSP to r3 so we can continue stacking  */
    #endif
            "VSTMDB  r3!,{s16-s31}           \n"   /* Yes, save additional VFP registers  */
            "_skip_vfp_save:                 \n"
            "STMDB   r3!, {LR}               \n"   /* Save LR on the stack  */
#endif /* TX_VFP_ENABLE */

            // [r1, #TX_PORT_OFFSET_STACK_PTR] == _tx_thread_current_ptr->tx_thread_stack_ptr
            "STR     r3, [r1, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_STACK_PTR)"] \n" /* Save its stack pointer */


            /* Stack preservation is complete. */

            /* Determine if time-slice is active. If it isn't, skip time handling processing.  */
            //if (_tx_timer_time_slice != 0)
            //{
            "LDR     r4, =_tx_timer_time_slice \n" /* Build address of time-slice variable */

            "LDR     r3, [r4]                  \n" /* Pickup current time-slice */
#ifdef TX_PORT_ISA_CBZ_SUPPORTED
            "CBZ     r3, __tx_ts_new           \n" /* If not active, skip processing */
#else
            "CMP     r3, #0                    \n"
            "BEQ     __tx_ts_new               \n"
#endif

                /* Time-slice is active, save the current thread's time-slice and clear the global time-slice variable.  */
                // _tx_thread_current_ptr->tx_thread_time_slice = _tx_timer_time_slice;
                "STR     r3, [r1, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_TIME_SLICE)"] \n" /* Save current time-slice */

                /* Clear the global time-slice.  */
                // _tx_timer_time_slice = 0;
                "MOVS    r3, #0                \n" /* Build clear value */
                "STR     r3, [r4]              \n" /* Clear time-slice */

            // }

            /* The executing thread is now completely preserved!!!  */

        // }

        /* Now we are looking for a new thread to execute!  */
        "__tx_ts_new:                        \n"

        /* Get the address of the new thread. */
        "LDR     r6, =_tx_thread_execute_ptr \n" // Build execute thread pointer address

#if defined(__IAR_SYSTEMS_ICC__) && (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
        /* Inputs for IAR are moved to R7 after stack preservation is complete to ensure they are
         * preserved when C functions are called. */
        "MOV     r7, %[input_table]          \n"
#endif

        /** Disable the interrupts.  */
        "CPSID   i                           \n"

        /** Is there a thread ready to execute?  */
        // if (_tx_thread_execute_ptr != 0)
        // {
        "LDR     r3, [r6]                    \n" // Is there another thread ready to execute?
#ifdef TX_PORT_ISA_CBZ_SUPPORTED
        "CBZ     r3, __tx_ts_wait            \n" // No, skip to the wait processing
#else
        "CMP     r3, #0                      \n"
        "BEQ     __tx_ts_wait                \n"
#endif

            /* Yes, another thread is ready for execution, make the current thread the new thread.  */
            //_tx_thread_current_ptr = _tx_thread_execute_ptr;

            "STR r3, [r0]                        \n" // Setup the current thread pointer to the new thread
            "B       __tx_ts_restore             \n" // Restore the thread

        // }
        // else
        // {

            /** If no thread is ready to execute, wait until one becomes ready.  */
            "__tx_ts_wait:          \n"
            "BL      _tx_port_wait_thread_ready   \n"

            /* r0-r3 are undefined after branches. */
        // }

        /** Re-enable interrupts.  */
        "__tx_ts_restore:                  \n"
        "LDR     r6, [r6]                  \n" // Get the base address of the thread ready to execute
        "CPSIE   i                         \n" // Enable interrupts

        /** Restore the thread.  */

        /** Increment the thread run count.  */
        //_tx_thread_current_ptr->tx_thread_run_count++;
        "LDR  r0, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_RUN_COUNT)"] \n" // Pickup the current thread run count
        "ADDS r0, r0, #1                                                    \n" // Increment the thread run count
        "STR  r0, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_RUN_COUNT)"] \n" // Store the new run count

        /** Setup global time-slice with thread's current time-slice.  */
        //  _tx_timer_time_slice = _tx_thread_current_ptr->tx_thread_time_slice;
        "LDR r0, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_TIME_SLICE)"] \n" // Pickup thread's current time-slice
        "STR r0, [r4]                                                       \n" // Setup global time-slice

#if (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
        /* Call the thread entry function to indicate the thread is executing.  */
        // _tx_execution_thread_enter();
        "CPSID   i                       \n" // Disable interrupts
    #if !defined(__IAR_SYSTEMS_ICC__)
        "BL  _tx_execution_thread_enter  \n" // Call the thread enter function
    #else
        /* g_pendsv_data[TX_PORT_INDEX(TX_PORT_ADDR_OFFSET_ENTER_FUNCTION)] = _tx_execution_thread_enter */
        "LDR     r1, [r7, #"SSP_STRINGIFY_EXPANDED(TX_PORT_ADDR_OFFSET_ENTER_FUNCTION)"] \n"
        "BLX     r1                      \n"
    #endif
        "CPSIE   i                       \n" // Enable interrupts
        /* r0-r3 are undefined after branches. */
#endif

#ifdef TX_HW_STACK_MONITOR_ENABLE
        /* Setup the Hardware Stack Monitor for thread to schedule.  */

        /* Disable PSP monitoring  */
        // R_SPMON->PSPMPUCTL = 0;
        "LDR     r0, ="SSP_STRINGIFY_EXPANDED(TX_PORT_SPMON_BASE)" \n" /* R_SPMON base. */

#ifdef TX_PORT_ISA_THUMB2_SUB_ADD_SUPPORTED
        "MOV     r1, #0                                                        \n"
#else
        "MOVS    r1, #0                                                        \n"
#endif
        "STR     r1, [r0, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_PSPMPUCTL)"] \n"

        /* Setup start address  */
        /* Bits 0 and 1 of _tx_thread_current_ptr->tx_thread_stack_start are always 0 since the stack must be aligned
         * on an 8 byte boundary. */
        // R_SPMON->PSPMPUSA = (uint32_t)_tx_thread_current_ptr->tx_thread_stack_start;
        "LDR     r1, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_STACK_START)"] \n"
        "STR     r1, [r0, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_PSPMPUSA)"]    \n"

        /* Setup end address  */
        /* Bits 0 and 1 of _tx_thread_current_ptr->tx_thread_stack_end are always 1 since the stack must be aligned
         * on an 8 byte boundary and a multiple of 8 bytes. */
        // R_SPMON->PSPMPUEA = (uint32_t)_tx_thread_current_ptr->tx_thread_stack_end;
        "LDR     r1, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_STACK_END)"] \n"
        "STR     r1, [r0, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_PSPMPUEA)"]  \n"
#endif

        /** Restore the thread context and PSP.  */

        /* Get stack pointer for this thread. */
        // [r6, #TX_PORT_OFFSET_STACK_PTR] == _tx_thread_current_ptr->tx_thread_stack_ptr
        "LDR     r3, [r6, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_STACK_PTR)"] \n"

#ifdef TX_VFP_ENABLE
        /* Set LR (EXC_RETURN) to the value saved in the thread control block. This value is always one of the
         * following:
         *    * 0xFFFFFFFD: Return to Thread mode, exception return uses non-floating-point state from the PSP
         *                  and execution uses PSP after return.
         *    * 0xFFFFFFED: Return to Thread mode, exception return uses floating-point state from PSP and execution
         *                  uses PSP after return.
         */
        "LDMIA     r3!, {lr}               \n"
#else
        /* Return to thread on process stack */
        "LDR r1, =0xFFFFFFFD                \n"

        /* Set LR (EXC_RETURN) to 0xFFFFFFFD. Return to Thread mode, exception return uses non-floating-point state
         * from the PSP and execution uses PSP after return. */
        "MOV lr, r1                         \n"
#endif

#ifdef TX_VFP_ENABLE
        "TST     LR, #0x10          \n"     /* Determine if the VFP extended frame is present  */
        "BNE     _skip_vfp_restore  \n"     /* If so, restore additional VFP registers and setup the proper exception return  */
        "VLDMIA  r3!, {s16-s31}     \n"     /* Yes, restore additional VFP registers  */
        "_skip_vfp_restore:         \n"
#endif
#ifdef TX_PORT_ISA_STMDB_LDMIA_SUPPORTED
        "LDMIA   r3!, {r4-r11}      \n"     /* Recover thread's registers  */
#else /* TX_PORT_ISA_STMDB_LDMIA_SUPPORTED */
        "LDM     r3!,{r4-r7}        \n"     /* Recover thread's registers (M3 Instruction: LDMIA r12!, {r4-r11} ) */
        "MOV     r11,r7             \n"
        "MOV     r10,r6             \n"
        "MOV     r9,r5              \n"
        "MOV     r8,r4              \n"
        "LDM     r3!,{r4-r7}        \n"
#endif /* TX_PORT_ISA_STMDB_LDMIA_SUPPORTED */
        "MSR     PSP, r3            \n"

#ifdef TX_HW_STACK_MONITOR_ENABLE
        /* Enable PSP monitoring  */
        // R_SPMON->PSPMPUCTL = 1;
#ifdef TX_PORT_ISA_THUMB2_SUB_ADD_SUPPORTED
        "MOV     r1, #1                                                        \n"
#else
        "MOVS    r1, #1                                                        \n"
#endif
        "STR     r1, [r0, #"SSP_STRINGIFY_EXPANDED(TX_PORT_OFFSET_PSPMPUCTL)"] \n"
#endif

        /** Return to the thread.  */
        "BX      LR                 \n"

        /* We should never get here!  */
#ifdef TX_ENABLE_BKPT
        "BKPT    0                  \n"
#endif
        // while(1);
        "__tx_error:                \n"
        "B       __tx_error         \n"

        "__tx_no_current:           \n"

        /* There is no current thread.  Load r4 and r5 with the values determined during thread preservation. */
        "LDR     r4, =_tx_timer_time_slice  \n" /* Build address of time-slice variable */

        "B __tx_ts_new                      \n"
#if defined(__IAR_SYSTEMS_ICC__) && (defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE))
        /* This is extended assembly syntax, which is required for IAR and not allowed in naked functions for GCC.
         * Refer to the Inline Assembler chapter of the EWARM Development Guide for syntax references. */
        : /* No outputs. */
        : /* Input table.  This passes &g_pendsv_data[0] in a register (forced to be r2 by the clobber list below).
             The register containing &g_pendsv_data[0] is referred to in the inline assembly as %[input_table]. */
          [input_table]"r"(&g_pendsv_data[0])
        : /* Clobbers, forces the compiler to use r2 to pass input table */
          "r0", "r1", "r3", "r4", "r5", "r6", "r7", "r12", "r14", "cc", "memory"
    #if (__CORE__ == __ARM7EM__) // CM4
          , "r8", "r9", "r10", "r11"
    #endif /* CM4 */
#endif
    );
}

/***********************************************************************//**
 *
 *  FUNCTION                                               RELEASE
 *
 *    SysTick_Handler                                      Synergy
 *
 *  DESCRIPTION
 *
 *    This ISR calls the timer interrupt.
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
 *    _tx_timer_interrupt
 *
 *  CALLED BY
 *
 *    N/A
 *
 **************************************************************************/
/* Moved from tx_timer_interrupt.c to ensure this code is not removed by the linker. */
#ifndef TX_NO_TIMER
extern VOID   _tx_timer_interrupt(VOID);
VOID SysTick_Handler(VOID)
{
    _tx_timer_interrupt();
}
#endif
/***********************************************************************//**
 * @} (end addtogroup TX_SYNERGY_PORT)
 **************************************************************************/

