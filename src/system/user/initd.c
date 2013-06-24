/*=========================================================================*//**
@file    initd.c

@author  Daniel Zorychta

@brief   This file contain initialize and runtime daemon

@note    Copyright (C) 2012 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include <stdio.h>
#include "user/initd.h"
#include "drivers/tty_def.h"
#include "drivers/sdspi_def.h"

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

//==============================================================================
/**
 * @brief Initialise devices and programs
 * User program which provide basic system functionality e.g. STDIO handle
 * (joining TTY driver with program), basic program starting and etc. This task
 * is an example to show how this can be implemented.
 */
//==============================================================================
void task_initd(void *arg)
{
        (void)arg;

        set_priority(INITD_PRIORITY);

        /* mount main file system */
        mount("lfs", NULL, "/");

        mkdir("/bin");
        mkdir("/dev");
        mkdir("/etc");
        mkdir("/home");
        mkdir("/mnt");
        mkdir("/proc");
        mkdir("/tmp");

        mount("procfs", NULL, "/proc");

        /* early initialization - basic drivers start */
        if (init_driver("pll", NULL) != STD_RET_OK) {
                while (TRUE);
        }

        init_driver("gpio", "/dev/gpio");

        /* early initialization - terminal support */
        init_driver("uart1", "/dev/ttyS0");
        init_driver("tty0", "/dev/tty0");
        enable_printk("/dev/tty0");

        /* something about board and system */
        printk(FONT_COLOR_GREEN FONT_BOLD "%s/%s" FONT_NORMAL " by "
               FONT_COLOR_CYAN "Daniel Zorychta "
               FONT_COLOR_YELLOW "<daniel.zorychta@gmail.com>" RESET_ATTRIBUTES "\n\n",
               get_OS_name(), get_kernel_name());

        /* driver initialization */
        init_driver("tty1", "/dev/tty1");
        init_driver("tty2", "/dev/tty2");
        init_driver("tty3", "/dev/tty3");
        init_driver("sdspi", "/dev/sda");


        /* initializing SD card and detecting partitions */
        printk("Detecting SD card... ");
        FILE *sd = fopen("/dev/sda", "r+");
        if (!sd) {
                printk(FONT_COLOR_RED"Cannot open file!"RESET_ATTRIBUTES"\n");
        } else {
                bool status;
                ioctl(sd, SDSPI_IORQ_INITIALIZE_CARD, &status);

                if (status == true) {
                        printk(FONT_COLOR_GREEN"Initialized."RESET_ATTRIBUTES"\n");
                } else {
                        printk(FONT_COLOR_RED"Fail\n"RESET_ATTRIBUTES);
                }
        }
        fclose(sd);


        /* initd info about stack usage */
        printk("[%d] initd: free stack: %d levels\n\n", get_tick_counter(), get_free_stack());

        /* change TTY for printk to last TTY */
        enable_printk("/dev/tty3");

        /* stdio program control */
        FILE *tty[TTY_DEV_COUNT]                 = {NULL};
        FILE *tty0                               = NULL;
        task_t *program[TTY_DEV_COUNT - 1]       = {NULL};
        enum prog_state state[TTY_DEV_COUNT - 1] = {PROGRAM_UNKNOWN_STATE};
        int current_tty                          = -1;

        while ((tty0 = fopen("/dev/tty0", "r+")) == NULL) {
                sleep_ms(200);
        }

        for (;;) {
                ioctl(tty0, TTY_IORQ_GET_CURRENT_TTY, &current_tty);

                if (current_tty >= 0 && current_tty < TTY_DEV_COUNT - 1) {
                        if (!program[current_tty]) {
                                if (tty[current_tty] == NULL) {
                                        char path[16];
                                        snprintf(path, sizeof(path), "/dev/tty%c", '0' + current_tty);
                                        tty[current_tty] = fopen(path, "r+");
                                }

                                program[current_tty] = new_program("terminal", "", "/",
                                                                   tty[current_tty],
                                                                   tty[current_tty],
                                                                   &state[current_tty],
                                                                   NULL);

                                if (program[current_tty]) {
                                        set_task_priority(program[current_tty], 0);
                                }

                                switch (state[current_tty]) {
                                case PROGRAM_UNKNOWN_STATE:
                                        printk("Program does not start!\n");
                                        break;
                                case PROGRAM_RUNNING:
                                        printk("Program started.\n");
                                        break;
                                case PROGRAM_ENDED:
                                        printk("Program finished.\n");
                                        break;
                                case PROGRAM_NOT_ENOUGH_FREE_MEMORY:
                                        printk("No enough free memory!\n");
                                        break;
                                case PROGRAM_ARGUMENTS_PARSE_ERROR:
                                        printk("Bad arguments!\n");
                                        break;
                                case PROGRAM_DOES_NOT_EXIST:
                                        printk("Program does not exist!\n");
                                        break;
                                case PROGRAM_HANDLE_ERROR:
                                        printk("Handle error!\n");
                                        break;
                                }
                        }
                }

                for (uint i = 0; i < TTY_DEV_COUNT - 1; i++) {
                        if (program[i] == NULL) {
                                continue;
                        }

                        if (state[i] != PROGRAM_RUNNING) {
                                printk("Program closed.\n");

                                program[i] = NULL;
                                state[i]   = PROGRAM_UNKNOWN_STATE;

                                ioctl(tty[i], TTY_IORQ_CLEAN_TTY);
                                fclose(tty[i]);
                                tty[i] = NULL;

                                ioctl(tty0, TTY_IORQ_SWITCH_TTY_TO, TTY_DEV_0);
                        }
                }

                sleep_ms(500);
        }

        task_exit();
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
