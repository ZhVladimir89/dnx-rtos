/*=========================================================================*//**
@file    cpuctl.c

@author  Daniel Zorychta

@brief   This file support CPU control

@note    Copyright (C) 2017 Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation and modified by the dnx RTOS exception.

         NOTE: The modification  to the GPL is  included to allow you to
               distribute a combined work that includes dnx RTOS without
               being obliged to provide the source  code for proprietary
               components outside of the dnx RTOS.

         The dnx RTOS  is  distributed  in the hope  that  it will be useful,
         but WITHOUT  ANY  WARRANTY;  without  even  the implied  warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         Full license text is available on the following file: doc/license.txt.


*//*==========================================================================*/

/*==============================================================================
  Include files
==============================================================================*/
#include "config.h"
#include "stm32f4/cpuctl.h"
#include "stm32f4/stm32f4xx.h"
#include "stm32f4/lib/stm32f4xx_rcc.h"
#include "stm32f4/lib/misc.h"
#include "kernel/kwrapper.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
/* Cortex System Control register address */
#define SCB_SysCtrl             (*((__IO uint32_t *)0xE000ED10))
#define SysCtrl_SLEEPDEEP       ((uint32_t)0x00000004)

#define RAM2_START              ((void *)&__ram2_start)
#define RAM2_SIZE               ((size_t)&__ram2_size)

#define RAM3_START              ((void *)&__ram3_start)
#define RAM3_SIZE               ((size_t)&__ram3_size)

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local object definitions
==============================================================================*/
extern void *__ram2_start;
extern void *__ram2_size;
extern void *__ram3_start;
extern void *__ram3_size;

static _mm_region_t ram2;
static _mm_region_t ram3;

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief  Basic (first) CPU/microcontroller configuration. This function is
 *         called before system start.
 *
 * @param  None
 *
 * @return None
 */
//==============================================================================
void _cpuctl_init(void)
{
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* enable sleep on idle debug */
        SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);

        #if (__OS_MONITOR_CPU_LOAD__ > 0)
        _cpuctl_init_CPU_load_counter();
        #endif

        _mm_register_region(&ram2, RAM2_START, RAM2_SIZE);
        _mm_register_region(&ram3, RAM3_START, RAM3_SIZE);
}

//==============================================================================
/**
 * @brief  This function restart CPU.
 */
//==============================================================================
void _cpuctl_restart_system(void)
{
        NVIC_SystemReset();
}

//==============================================================================
/**
 * @brief  This function restart CPU.
 */
//==============================================================================
void _cpuctl_shutdown_system(void)
{
        // Note: implementation enters to deep sleep mode.

        /* enable power module */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;

        /* Clear Wake-up flag */
        PWR->CR |= PWR_CR_CWUF;

        /* Select STANDBY mode */
        PWR->CR |= PWR_CR_PDDS;

        /* Set SLEEPDEEP bit of Cortex System Control Register */
        SCB_SysCtrl |= SysCtrl_SLEEPDEEP;

        /* Request Wait For Interrupt */
        __WFI();
}

//==============================================================================
/**
 * @brief  Start counter used for CPU load measurement. Timer should be set
 *         to at least 1MHz and should not overflow until 1 second.
 *
 * @param  None
 *
 * @return None
 */
//==============================================================================
#if (__OS_MONITOR_CPU_LOAD__ > 0)
void _cpuctl_init_CPU_load_counter(void)
{
}
#endif

//==============================================================================
/**
 * @brief  Function return valut that was counted from last call of this function.
 *         This function must reset timer after read. Function is called from
 *         IRQs.
 *
 * @param  None
 *
 * @return Timer value for last read (time delta).
 */
//==============================================================================
#if (__OS_MONITOR_CPU_LOAD__ > 0)
u32_t _cpuctl_get_CPU_load_counter_delta(void)
{
        static uint32_t last;
        bool  ovf = SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
        u32_t now = SysTick->VAL;

        u32_t delta;
        if (ovf) {
                delta = ((SysTick->LOAD + 1) - now) + last;
        } else {
                delta = last - now;
        }

        last = now;

        return delta;
}
#endif

//==============================================================================
/**
 * @brief  Function sleep CPU weakly. All IRQs must be able to wake up CPU.
 *
 * @param  None
 *
 * @return None
 */
//==============================================================================
void _cpuctl_sleep(void)
{
        __WFI();
}

//==============================================================================
/**
 * @brief  Function update all system clock after CPU frequency change.
 *         Function must update all devices which base on main clock oscillator.
 *         Function is called after clock/frequency change from clock management driver.
 *
 * @param  None
 *
 * @return None
 */
//==============================================================================
void _cpuctl_update_system_clocks(void)
{
        /* update CPU load timer frequency */
#if (__OS_MONITOR_CPU_LOAD__ > 0)
        _cpuctl_init_CPU_load_counter();
#endif

        /* update context switch counter frequency */
        _critical_section_begin();
        RCC_ClocksTypeDef freq;
        RCC_GetClocksFreq(&freq);
        SysTick_Config((freq.HCLK_Frequency / (u32_t)__OS_TASK_SCHED_FREQ__) - 1);
        _critical_section_end();
}

//==============================================================================
/**
 * @brief  Function delay code processing in microseconds.
 *
 * @note   Function should block CPU for specified amount of time.
 * @note   Function should work in critical section and interrupts.
 *
 * @param  microseconds         microsecond delay
 */
//==============================================================================
void _cpuctl_delay_us(u16_t microseconds)
{
        u32_t ticks = ((u64_t)microseconds * SysTick->LOAD * __OS_TASK_SCHED_FREQ__) / 1000000;

        while (ticks > 0) {
                i32_t now = SysTick->VAL;

                if (now - ticks > 0) {
                        while (SysTick->VAL > (u32_t)(now - ticks));
                        ticks = 0;
                } else {
                        while (SysTick->VAL <= (u32_t)now);
                        ticks -= now;
                }
        }
}

/*==============================================================================
  End of file
==============================================================================*/
