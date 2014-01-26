/*=========================================================================*//**
@file    wdg_def.h

@author  Daniel Zorychta

@brief   WDG driver

@note    Copyright (C) 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifndef _WDG_DEF_H_
#define _WDG_DEF_H_

/*==============================================================================
  Include files
==============================================================================*/
#include "core/ioctl_macros.h"
#include "stm32f1/wdg_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Exported macros
==============================================================================*/
#define _WDG_MAJOR_NUMBER               0
#define _WDG_MINOR_NUMBER               0

#define WDG_IORQ_RESET                  _IO('C', 0x00)

/*==============================================================================
  Exported object types
==============================================================================*/

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Exported functions
==============================================================================*/

/*==============================================================================
  Exported inline functions
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _WDG_DEF_H_ */
/*==============================================================================
  End of file
==============================================================================*/