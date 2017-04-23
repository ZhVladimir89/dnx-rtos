/*=========================================================================*//**
@file    df.c

@author  Daniel Zorychta

@brief   List mounted file systems

@note    Copyright (C) 2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <dnx/misc.h>
#include <dnx/vt100.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define KiB                             (u32_t)(1024)
#define MiB                             (u32_t)(1024*1024)
#define GiB                             (u64_t)(1024*1024*1024)
#define CONVERT_TO_KiB(_val)            (_val >> 10)
#define CONVERT_TO_MiB(_val)            (_val >> 20)
#define CONVERT_TO_GiB(_val)            (_val >> 30)

/*==============================================================================
  Local types, enums definitions
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local object definitions
==============================================================================*/
GLOBAL_VARIABLES_SECTION {
};

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/
//==============================================================================
/**
 * @brief Cat main function
 */
//==============================================================================
int_main(df, STACK_DEPTH_LOW, int argc, char *argv[])
{
        (void) argc;
        (void) argv;

        printf(VT100_FONT_BOLD"File system"VT100_CURSOR_FORWARD(5)"Total"VT100_CURSOR_FORWARD(5)
               "Free"VT100_CURSOR_FORWARD(6)"%%Used  Mount point"VT100_RESET_ATTRIBUTES"\n");

        struct mntent mnt;
        int           i = 0;
        while (getmntentry(i++, &mnt) == 0) {
                u32_t dtotal;
                u32_t dfree;
                const char *unit;

                if (mnt.mnt_total > 10*GiB) {
                        dtotal = CONVERT_TO_GiB(mnt.mnt_total);
                        dfree  = CONVERT_TO_GiB(mnt.mnt_free);
                        unit   = "GiB";
                } else if (mnt.mnt_total > 10*MiB) {
                        dtotal = CONVERT_TO_MiB(mnt.mnt_total);
                        dfree  = CONVERT_TO_MiB(mnt.mnt_free);
                        unit   = "MiB";
                } else if (mnt.mnt_total > 10*KiB) {
                        dtotal = CONVERT_TO_KiB(mnt.mnt_total);
                        dfree  = CONVERT_TO_KiB(mnt.mnt_free);
                        unit   = "KiB";
                } else {
                        dtotal = mnt.mnt_total;
                        dfree  = mnt.mnt_free;
                        unit   = "B";
                }

                u32_t percent = ((dtotal - dfree) * 1000) / dtotal;

                printf("%s"  VT100_CURSOR_BACKWARD(90)VT100_CURSOR_FORWARD(16)
                       "%u%s"VT100_CURSOR_BACKWARD(90)VT100_CURSOR_FORWARD(26)
                       "%u%s"VT100_CURSOR_BACKWARD(90)VT100_CURSOR_FORWARD(36)
                       "%u.%u%%"VT100_CURSOR_BACKWARD(90)VT100_CURSOR_FORWARD(43)
                       "%s\n",
                       mnt.mnt_fsname, dtotal, unit, dfree, unit,
                       percent / 10, percent % 10, mnt.mnt_dir);
        }

        return EXIT_SUCCESS;
}

/*==============================================================================
  End of file
==============================================================================*/
