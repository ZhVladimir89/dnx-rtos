/*=========================================================================*//**
@file    irq.c

@author  Daniel Zorychta

@brief   This driver support external interrupts (EXTI).

@note    Copyright (C) 2014  Daniel Zorychta <daniel.zorychta@gmail.com>

         This program is free software; you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the  Free Software  Foundation;  either version 2 of the License, or
         any later version.

         This  program  is  distributed  in the hope that  it will be useful,
         but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
         MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
         GNU General Public License for more details.

         You  should  have received a copy  of the GNU General Public License
         along  with  this  program;  if not,  write  to  the  Free  Software
         Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


*//*==========================================================================*/

/*==============================================================================
  Include files
==============================================================================*/
#include "drivers/driver.h"
#include "stm32f1/irq_cfg.h"
#include "stm32f1/stm32f10x.h"
#include "../irq_ioctl.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define NUMBER_OF_IRQs          16

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
typedef struct {
        sem_t *sem[NUMBER_OF_IRQs];
} IRQ_t;

typedef struct {
        u8_t           priority;
        enum _IRQ_MODE mode;
} default_cfg_t;

/*==============================================================================
  Local function prototypes
==============================================================================*/
static enum IRQn IRQ_major_to_NVIC_IRQn (u8_t major);
static int       IRQ_configure          (u8_t major, enum _IRQ_MODE mode, int priority);
static bool      IRQ_handler            (u8_t major);

/*==============================================================================
  Local object definitions
==============================================================================*/
MODULE_NAME(IRQ);

static const default_cfg_t DEFAULT_CONFIG[NUMBER_OF_IRQs] = {
        {.priority = _IRQ_LINE_0_PRIO,  .mode = _IRQ_LINE_0_MODE },
        {.priority = _IRQ_LINE_1_PRIO,  .mode = _IRQ_LINE_1_MODE },
        {.priority = _IRQ_LINE_2_PRIO,  .mode = _IRQ_LINE_2_MODE },
        {.priority = _IRQ_LINE_3_PRIO,  .mode = _IRQ_LINE_3_MODE },
        {.priority = _IRQ_LINE_4_PRIO,  .mode = _IRQ_LINE_4_MODE },
        {.priority = _IRQ_LINE_5_PRIO,  .mode = _IRQ_LINE_5_MODE },
        {.priority = _IRQ_LINE_6_PRIO,  .mode = _IRQ_LINE_6_MODE },
        {.priority = _IRQ_LINE_7_PRIO,  .mode = _IRQ_LINE_7_MODE },
        {.priority = _IRQ_LINE_8_PRIO,  .mode = _IRQ_LINE_8_MODE },
        {.priority = _IRQ_LINE_9_PRIO,  .mode = _IRQ_LINE_9_MODE },
        {.priority = _IRQ_LINE_10_PRIO, .mode = _IRQ_LINE_10_MODE},
        {.priority = _IRQ_LINE_11_PRIO, .mode = _IRQ_LINE_11_MODE},
        {.priority = _IRQ_LINE_12_PRIO, .mode = _IRQ_LINE_12_MODE},
        {.priority = _IRQ_LINE_13_PRIO, .mode = _IRQ_LINE_13_MODE},
        {.priority = _IRQ_LINE_14_PRIO, .mode = _IRQ_LINE_14_MODE},
        {.priority = _IRQ_LINE_15_PRIO, .mode = _IRQ_LINE_15_MODE}
};

static IRQ_t *IRQ;

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Initialize device
 *
 * @param[out]          **device_handle        device allocated memory
 * @param[in ]            major                major device number
 * @param[in ]            minor                minor device number
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_INIT(IRQ, void **device_handle, u8_t major, u8_t minor)
{
        int err = ENODEV;

        if (major < NUMBER_OF_IRQs && minor == 0) {
                if (IRQ == NULL) {
                        err = sys_zalloc(sizeof(IRQ_t), cast(void*, &IRQ));
                }

                if (IRQ) {
                        err = sys_semaphore_create(1, 0, &IRQ->sem[major]);
                        if (err == ESUCC) {
                                // device's major number is used as identifier
                                *device_handle = cast(void*, cast(u32_t, major));

                                IRQ_configure(major,
                                              DEFAULT_CONFIG[major].mode,
                                              DEFAULT_CONFIG[major].priority);
                        }
                }
        }

        return err;
}

//==============================================================================
/**
 * @brief Release device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_RELEASE(IRQ, void *device_handle)
{
        u8_t major = cast(u32_t, device_handle);

        int err = sys_semaphore_destroy(IRQ->sem[major]);
        if (!err) {
                IRQ_configure(major, _IRQ_MODE_DISABLED, -1);

                bool free_module_mem = true;
                for (int i = 0; i < NUMBER_OF_IRQs && free_module_mem; i++) {
                        free_module_mem = IRQ->sem[i] == NULL;
                }

                if (free_module_mem) {
                        sys_free(cast(void*, &IRQ));
                }
        }

        return err;
}

//==============================================================================
/**
 * @brief Open device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           flags                  file operation flags (O_RDONLY, O_WRONLY, O_RDWR)
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_OPEN(IRQ, void *device_handle, u32_t flags)
{
        UNUSED_ARG2(device_handle, flags);

        return ESUCC;
}

//==============================================================================
/**
 * @brief Close device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           force                  device force close (true)
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_CLOSE(IRQ, void *device_handle, bool force)
{
        UNUSED_ARG2(device_handle, force);

        return ESUCC;
}

//==============================================================================
/**
 * @brief Write data to device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]          *src                    data source
 * @param[in ]           count                  number of bytes to write
 * @param[in ][out]     *fpos                   file position
 * @param[out]          *wrcnt                  number of written bytes
 * @param[in ]           fattr                  file attributes
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_WRITE(IRQ,
              void             *device_handle,
              const u8_t       *src,
              size_t            count,
              fpos_t           *fpos,
              size_t           *wrcnt,
              struct vfs_fattr  fattr)
{
        UNUSED_ARG6(device_handle, src, count, fpos, wrcnt, fattr);

        return ENOTSUP;
}

//==============================================================================
/**
 * @brief Read data from device
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *dst                    data destination
 * @param[in ]           count                  number of bytes to read
 * @param[in ][out]     *fpos                   file position
 * @param[out]          *rdcnt                  number of read bytes
 * @param[in ]           fattr                  file attributes
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_READ(IRQ,
             void            *device_handle,
             u8_t            *dst,
             size_t           count,
             fpos_t          *fpos,
             size_t          *rdcnt,
             struct vfs_fattr fattr)
{
        UNUSED_ARG6(device_handle, dst, count, fpos, rdcnt, fattr);

        return ENOTSUP;
}

//==============================================================================
/**
 * @brief IO control
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[in ]           request                request
 * @param[in ][out]     *arg                    request's argument
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_IOCTL(IRQ, void *device_handle, int request, void *arg)
{
        u8_t major = cast(u32_t, device_handle);
        int  err   = EINVAL;

        if (arg) {
                switch (request) {
                case IOCTL_IRQ__CATCH: {
                        const u32_t *timeout = arg;
                        if (IRQ->sem[major]) {
                                err = sys_semaphore_wait(IRQ->sem[major], *timeout);
                        } else {
                                err = ENODEV;
                        }
                        break;
                }

                case IOCTL_IRQ__TRIGGER: {
                        WRITE_REG(EXTI->SWIER, EXTI_SWIER_SWIER0 << major);
                        err = ESUCC;
                        break;
                }

                case IOCTL_IRQ__CONFIGURE: {
                        const IRQ_config_t *cfg = arg;
                        err = IRQ_configure(major, cast(enum _IRQ_MODE, *cfg), -1);
                        break;
                }

                default:
                        err = EBADRQC;
                }
        }

        return err;
}

//==============================================================================
/**
 * @brief Flush device
 *
 * @param[in ]          *device_handle          device allocated memory
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_FLUSH(IRQ, void *device_handle)
{
        UNUSED_ARG1(device_handle);

        return ESUCC;
}

//==============================================================================
/**
 * @brief Device information
 *
 * @param[in ]          *device_handle          device allocated memory
 * @param[out]          *device_stat            device status
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
API_MOD_STAT(IRQ, void *device_handle, struct vfs_dev_stat *device_stat)
{
        device_stat->st_size  = 0;

        return ESUCC;
}

//==============================================================================
/**
 * @brief Converts major IRQ number (0-15) to NVIC IRQ number
 *
 * @param major         device major number
 *
 * @return On success, NVIC IRQ number is returned, otherwise 0.
 */
//==============================================================================
static enum IRQn IRQ_major_to_NVIC_IRQn(u8_t major)
{
        switch (major) {
        case 0  ... 4 : return EXTI0_IRQn + major;
        case 5  ... 9 : return EXTI9_5_IRQn;
        case 10 ... 15: return EXTI15_10_IRQn;
        default:        return 0;
        }
}

//==============================================================================
/**
 * @brief Function enable or disable selected EXTI interrupt
 *
 * @param major         IRQ major number (0-15)
 * @param mode          IRQ mode (edge detector configuration)
 * @param priority      IRQ priority (-1 to ignore)
 *
 * @return None
 */
//==============================================================================
static int IRQ_configure(u8_t major, enum _IRQ_MODE mode, int priority)
{
        enum IRQn IRQn = IRQ_major_to_NVIC_IRQn(major);
        int       err  = ENODEV;

        if (IRQn != 0) {
                if (mode == _IRQ_MODE_DISABLED) {
                        CLEAR_BIT(EXTI->IMR, EXTI_IMR_MR0 << major);
                        CLEAR_BIT(EXTI->EMR, EXTI_EMR_MR0 << major);

                        switch (IRQn) {
                        case EXTI9_5_IRQn: {
                                static const u32_t IRQ9_5_MASK = EXTI_IMR_MR5
                                                               | EXTI_IMR_MR6
                                                               | EXTI_IMR_MR7
                                                               | EXTI_IMR_MR8
                                                               | EXTI_IMR_MR9;

                                if ((EXTI->IMR & IRQ9_5_MASK) == 0) {
                                        NVIC_DisableIRQ(IRQn);
                                }
                                break;
                        }

                        case EXTI15_10_IRQn: {
                                static const u32_t IRQ15_10_MASK = EXTI_IMR_MR10
                                                                 | EXTI_IMR_MR11
                                                                 | EXTI_IMR_MR12
                                                                 | EXTI_IMR_MR13
                                                                 | EXTI_IMR_MR14
                                                                 | EXTI_IMR_MR15;

                                if ((EXTI->IMR & IRQ15_10_MASK) == 0) {
                                        NVIC_DisableIRQ(IRQn);
                                }
                                break;
                        }

                        default:
                                NVIC_DisableIRQ(IRQn);
                                break;
                        }

                        err = ESUCC;
                } else {
                        switch (mode) {
                        case _IRQ_MODE_FALLING_EDGE:
                                SET_BIT(EXTI->FTSR, EXTI_FTSR_TR0 << major);
                                CLEAR_BIT(EXTI->RTSR, EXTI_RTSR_TR0 << major);
                                err = ESUCC;
                                break;

                        case _IRQ_MODE_RISING_EDGE:
                                CLEAR_BIT(EXTI->FTSR, EXTI_FTSR_TR0 << major);
                                SET_BIT(EXTI->RTSR, EXTI_RTSR_TR0 << major);
                                err = ESUCC;
                                break;

                        case _IRQ_MODE_FALLING_AND_RISING_EDGE:
                                SET_BIT(EXTI->FTSR, EXTI_FTSR_TR0 << major);
                                SET_BIT(EXTI->RTSR, EXTI_RTSR_TR0 << major);
                                err = ESUCC;
                                break;

                        default:
                                err = ENOTSUP;
                                break;
                        }

                        if (err == ESUCC) {
                                SET_BIT(EXTI->IMR, EXTI_IMR_MR0 << major);
                                SET_BIT(EXTI->EMR, EXTI_EMR_MR0 << major);

                                NVIC_EnableIRQ(IRQn);

                                if (priority >= 0) {
                                        NVIC_SetPriority(IRQn, priority);
                                }
                        }
                }
        }

        return err;
}

//==============================================================================
/**
 * @brief Handle interrupts for selected EXTI line
 *
 * @param major         IRQ major number
 *
 * @return true if task woken, otherwise false
 */
//==============================================================================
static bool IRQ_handler(u8_t major)
{
        WRITE_REG(EXTI->PR, EXTI_PR_PR0 << major);

        if (IRQ) {
                bool woken = false;
                sys_semaphore_signal_from_ISR(IRQ->sem[major], &woken);
                return woken;
        } else {
                return false;
        }
}

//==============================================================================
/**
 * @brief EXTI0 IRQ Handler
 */
//==============================================================================
void EXTI0_IRQHandler(void)
{
        sys_thread_yield_from_ISR(IRQ_handler(0));
}

//==============================================================================
/**
 * @brief EXTI1 IRQ Handler
 */
//==============================================================================
void EXTI1_IRQHandler(void)
{
        sys_thread_yield_from_ISR(IRQ_handler(1));
}

//==============================================================================
/**
 * @brief EXTI2 IRQ Handler
 */
//==============================================================================
void EXTI2_IRQHandler(void)
{
        sys_thread_yield_from_ISR(IRQ_handler(2));
}

//==============================================================================
/**
 * @brief EXTI3 IRQ Handler
 */
//==============================================================================
void EXTI3_IRQHandler(void)
{
        sys_thread_yield_from_ISR(IRQ_handler(3));
}

//==============================================================================
/**
 * @brief EXTI4 IRQ Handler
 */
//==============================================================================
void EXTI4_IRQHandler(void)
{
        sys_thread_yield_from_ISR(IRQ_handler(4));
}

//==============================================================================
/**
 * @brief EXTI5-9 IRQ Handler
 */
//==============================================================================
void EXTI9_5_IRQHandler(void)
{
        bool woken = false;

        for (uint i = 5; i <= 9; i++) {
                woken |= IRQ_handler(i);
        }

        sys_thread_yield_from_ISR(woken);
}

//==============================================================================
/**
 * @brief EXTI10-15 IRQ Handler
 */
//==============================================================================
void EXTI15_10_IRQHandler(void)
{
        bool woken = false;

        for (uint i = 10; i <= 15; i++) {
                woken |= IRQ_handler(i);
        }

        sys_thread_yield_from_ISR(woken);
}

/*==============================================================================
  End of file
==============================================================================*/
