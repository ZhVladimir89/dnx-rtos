/*=========================================================================*//**
@file    libfat_unicode.c

@author  Daniel Zorychta

@brief   FAT file system library based on ChaN's code.

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

/*==============================================================================
  Include files
==============================================================================*/
#include "libfat.h"

#if _LIBFAT_USE_LFN != 0
#       if   _LIBFAT_CODE_PAGE == 932
#               include "codepage/cc932.c"
#       elif _LIBFAT_CODE_PAGE == 936
#               include "codepage/cc936.c"
#       elif _LIBFAT_CODE_PAGE == 949
#               include "codepage/cc949.c"
#       elif _LIBFAT_CODE_PAGE == 950
#               include "codepage/cc950.c"
#       else
#               include "codepage/ccsbcs.c"
#       endif
#endif

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local object definitions
==============================================================================*/

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

/*==============================================================================
  End of file
==============================================================================*/
