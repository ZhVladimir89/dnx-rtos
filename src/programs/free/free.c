/*=========================================================================*//**
@file    free.c

@author  Daniel Zorychta

@brief   Show used/free memory

@note    Copyright (C) 2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dnx/os.h>
#include <dnx/vt100.h>

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
int_main(free, STACK_DEPTH_VERY_LOW, int argc, char *argv[])
{
        (void)argc;

        uint  drv_count = get_number_of_modules();
        int *modmem = malloc(drv_count * sizeof(int));
        if (!modmem) {
                perror(NULL);
                return EXIT_FAILURE;
        }

        memstat_t sysmem;
        get_memory_usage_details(&sysmem);

        for (uint module = 0; module < drv_count; module++) {
                modmem[module] = get_module_memory_usage(module);
        }

        u32_t m_free = get_free_memory();
        u32_t m_used = get_used_memory();

        printf("Total: %d\n", get_memory_size());
        printf("Free : %d\n", m_free);
        printf("Used : %d\n", m_used);
        printf("Memory usage: %d%%\n", (m_used * 100)/get_memory_size());

        if (strcmp(argv[1], "-d") == 0) {
                printf("\nDetailed memory usage:\n");
                printf("  Kernel     : %d\n", sysmem.kernel_memory_usage);
                printf("  Filesystems: %d\n", sysmem.filesystems_memory_usage);
                printf("  Modules    : %d\n", sysmem.modules_memory_usage);
                printf("  Network    : %d\n", sysmem.network_memory_usage);
                printf("  Programs   : %d\n", sysmem.programs_memory_usage);
                printf("  Shared     : %d\n", sysmem.shared_memory_usage);
                printf("  Static     : %d\n\n", sysmem.static_memory_usage);

                printf("Detailed modules memory usage:\n");
                for (uint module = 0; module < drv_count; module++) {
                        printf("  %s"VT100_CURSOR_BACKWARD(99)VT100_CURSOR_FORWARD(14)": %d\n",
                               get_module_name(module),
                               get_module_memory_usage(module));
                }
        }

        free(modmem);

        return EXIT_SUCCESS;
}

/*==============================================================================
  End of file
==============================================================================*/
