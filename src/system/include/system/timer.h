/*=========================================================================*//**
@file    timer.h

@author  Daniel Zorychta

@brief   Software timer library.

@note    Copyright (C) 2013 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include "kernel/kwrapper.h"

/*==============================================================================
  Exported macros
==============================================================================*/

/*==============================================================================
  Exported object types
==============================================================================*/
typedef int timer_t;

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Exported functions
==============================================================================*/

/*==============================================================================
  Exported inline functions
==============================================================================*/
//==============================================================================
/**
 * @brief Function reset timer
 *
 * @return timer start value
 */
//==============================================================================
static inline timer_t timer_reset(void)
{
        return _kernel_get_time_ms();
}

//==============================================================================
/**
 * @brief Function check if timer is expired
 *
 * @param timer         timer value
 * @param time          expiration time
 *
 * @return true if timer expired, otherwise false
 */
//==============================================================================
static inline bool timer_is_expired(timer_t timer, int time)
{
        return (_kernel_get_time_ms() - timer >= time);
}

//==============================================================================
/**
 * @brief Function check if timer is not expired
 *
 * @param timer         timer value
 * @param time          expiration time
 *
 * @return true if timer not expired, otherwise false
 */
//==============================================================================
static inline bool timer_is_not_expired(timer_t timer, int time)
{
        return (_kernel_get_time_ms() - timer < time);
}

//==============================================================================
/**
 * @brief Function set timer to expired value
 *
 * @return timer expired value
 */
//==============================================================================
static inline timer_t timer_set_expired(void)
{
        return 0;
}

//==============================================================================
/**
 * @brief Function calculate timer time difference (abs value)
 *
 * @param timer1        timer value
 * @param timer2        timer value
 *
 * @return timer expired value
 */
//==============================================================================
static inline int timer_difftime(timer_t timer1, timer_t timer2)
{
        return timer1 > timer2 ? timer1 - timer2 : timer2 - timer1;
}

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
/*==============================================================================
  End of file
==============================================================================*/