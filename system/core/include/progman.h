#ifndef PROGMAN_H_
#define PROGMAN_H_
/*=========================================================================*//**
@file    progman.h

@author  Daniel Zorychta

@brief   This file support programs layer

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
#include "systypes.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
#define new_program(name, args, cwd, fstdin, fstdout, status, exit_code) \
        prgm_new_program(name, args, cwd, fstdin, fstdout, status, exit_code)

#define delete_program(taskhdl)                 prgm_delete_program(taskhdl)

#define GLOBAL_VARIABLES                        struct __global_vars__

#define PROGRAM_PARAMS(name, stack)\
        const uint prog_##name##_gs = sizeof(struct __global_vars__);\
        const uint prog_##name##_stack = (stack)

#define EXPORT_PROGRAM_PARAMS(name)\
        extern const uint prog_##name##_gs;\
        extern const uint prog_##name##_stack

#define PROGRAM_MAIN(name, argc, argv)          program_##name##_main(argc, argv)
#define stdin                                   get_this_task_data()->f_stdin
#define stdout                                  get_this_task_data()->f_stdout
#define global                                  ((struct __global_vars__*)get_this_task_data()->f_global_vars)
#define create_fast_global(name)                struct __global_vars__*name = global

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/
enum prg_status {
        PROGRAM_RUNNING,
        PROGRAM_ENDED,
        PROGRAM_NOT_ENOUGH_FREE_MEMORY,
        PROGRAM_ARGUMENTS_PARSE_ERROR,
        PROGRAM_HANDLE_ERROR,
};

/*==============================================================================
  Exported function prototypes
==============================================================================*/
extern task_t *prgm_new_program(char*, char*, char*, FILE_t*, FILE_t*, enum prg_status*, int*);
extern void    prgm_delete_program(task_t*);

#ifdef __cplusplus
}
#endif

#endif /* PROGMAN_H_ */
/*==============================================================================
  End of file
==============================================================================*/