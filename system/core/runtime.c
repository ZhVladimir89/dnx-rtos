/*=========================================================================*//**
@file    appruntime.c

@author  Daniel Zorychta

@brief   This file support runtime environment for applications

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
#include "runtime.h"
#include "regprg.h"
#include "oswrap.h"
#include "taskmoni.h"
#include "io.h"
#include "dlist.h"
#include <string.h>

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
#define calloc(nmemb, msize)            tskm_calloc(nmemb, msize)
#define malloc(size)                    tskm_malloc(size)
#define free(mem)                       tskm_free(mem)

#define MTX_BTIME_FOR_PLIST             5

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
struct program_data {
        FILE_t *f_stdin;
        FILE_t *f_stdout;
        ch_t   *cwd;
        void   *global_vars;
        int     exit_code;
        enum prg_status status;
};

struct program_mangement {
        list_t  *program_list;
        mutex_t  mtx_program_list;
};

struct program_args {
        int (*main_function)(ch_t**, int);
        uint  globals_size;
        ch_t *name;
        ch_t *args;
};

/*==============================================================================
  Local function prototypes
==============================================================================*/
static stdRet_t init_program_management(void);
static ch_t   **new_argument_table(ch_t *arg, const ch_t *name, int *argc);
static void     delete_argument_table(ch_t **argv, int argc);
static void     task_program_startup(void *argv);
static stdRet_t set_program_exit_code(task_t taskhdl, int exit_code);
static stdRet_t set_program_globals(task_t taskhdl, void *globals);
static stdRet_t set_program_status(task_t taskhdl, enum prg_status pstatus);

/*==============================================================================
  Local object definitions
==============================================================================*/
static struct program_mangement pman;

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief Function start new program by name
 *
 * @param *name         program name
 * @param *args         program argument string
 *
 * @return 0 if error, otherwise task handler
 */
//==============================================================================
task_t run_program(ch_t *name, ch_t *args, FILE_t *fstdin, FILE_t *fstdout, ch_t *cwd)
{
        struct program_data *pdata = NULL;
        struct program_args *pargs = NULL;
        struct regprg_pdata  regpdata;
        task_t taskhdl;

        if (init_program_management() != STD_RET_OK) {
                return 0;
        }

        if (regprg_get_program_data(name, &regpdata) != STD_RET_OK) {
                return 0;
        }

        if ((pdata = calloc(1, sizeof(struct program_data))) == NULL) {
                goto error;
        }

        if ((pargs = calloc(1, sizeof(struct program_args))) == NULL) {
                goto error;
        }

        pargs->args          = args;
        pargs->globals_size  = regpdata.globals_size;
        pargs->main_function = regpdata.main_function;
        pargs->name          = regpdata.name;

        if (new_task(task_program_startup, regpdata.name, regpdata.stack_deep,
                     pargs, 0, &taskhdl) == OS_OK) {

                suspend_task(taskhdl);

                pdata->cwd      = cwd;
                pdata->f_stdin  = fstdin;
                pdata->f_stdout = fstdout;

                while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
                i32_t item = list_add_item(pman.program_list, (u32_t)taskhdl, pdata);
                mutex_unlock(pman.mtx_program_list);

                if (item < 0) {
                        goto error;
                }

                resume_task(taskhdl);

                return taskhdl;
        }

        /* an error occurred */
error:
        if (pdata) {
                free(pdata);
        }

        if (pargs) {
                free(pargs);
        }

        if (taskhdl) {
                delete_task(taskhdl);
        }

        return 0;
}

//==============================================================================
/**
 * @brief Function returns program status
 *
 * @param taskhdl       task handle
 *
 * @return status
 */
//==============================================================================
enum prg_status get_program_status(task_t taskhdl)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)taskhdl);
        mutex_unlock(pman.mtx_program_list);

        if (pdata) {
                return pdata->status;
        }

        return PROGRAM_NEVER_EXISTED;
}

//==============================================================================
/**
 * @brief Function returns stdin file
 *
 * @return stdin file or NULL if doesn't exist
 */
//==============================================================================
FILE_t *get_program_stdin(void)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)get_task_handle());
        mutex_unlock(pman.mtx_program_list);

        if (pdata) {
                return pdata->f_stdin;
        }

        return NULL;
}

//==============================================================================
/**
 * @brief Function returns stdout file
 *
 * @param taskhdl       task handle
 *
 * @return stdout file or NULL if doesn't exist
 */
//==============================================================================
FILE_t *get_program_stdout(void)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)get_task_handle());
        mutex_unlock(pman.mtx_program_list);

        if (pdata) {
                return pdata->f_stdout;
        }

        return NULL;
}

//==============================================================================
/**
 * @brief Function returns global variable address
 *
 * @param taskhdl       task handle
 *
 * @return pointer to globals or NULL
 */
//==============================================================================
void *get_program_globals(void)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)get_task_handle());
        mutex_unlock(pman.mtx_program_list);

        if (pdata) {
                return pdata->global_vars;
        }

        return NULL;
}

//==============================================================================
/**
 * @brief Function returns current working path
 *
 * @param taskhdl       task handle
 *
 * @return current working path pointer or NULL if error
 */
//==============================================================================
ch_t *get_program_cwd(void)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)get_task_handle());
        mutex_unlock(pman.mtx_program_list);

        if (pdata) {
                return pdata->cwd;
        }

        return NULL;
}

//==============================================================================
/**
 * @brief Program startup
 *
 * @param *argv         pointer to program's informations
 */
//==============================================================================
static void task_program_startup(void *argv)
{
        struct program_args *pdata = argv;
        int    pargc   = 0;
        void  *globals = NULL;
        ch_t **pargv   = NULL;

        if (set_program_status(get_task_handle(), PROGRAM_INITING) != STD_RET_OK) {
                goto task_exit;
        }

        if ((globals = calloc(1, pdata->globals_size)) == NULL) {
                set_program_status(get_task_handle(), PROGRAM_NOT_ENOUGH_FREE_MEMORY);
                goto task_exit;
        }

        if (set_program_globals(get_task_handle(), globals)) {
                goto task_exit;
        }

        if ((pargv = new_argument_table(pdata->args, pdata->name, &pargc)) == NULL) {
                set_program_status(get_task_handle(), PROGRAM_ARGUMENTS_PARSE_ERROR);
                goto task_exit;
        }

        set_program_status(get_task_handle(), PROGRAM_RUNNING);
        set_program_exit_code(get_task_handle(), pdata->main_function(pargv, pargc));
        set_program_status(get_task_handle(), PROGRAM_ENDED);

        task_exit:
        free(globals);
        delete_argument_table(pargv, pargc);
        free(pdata);
        terminate_task();
}

//==============================================================================
/**
 * @brief Function initialize program manager
 *
 * @retval STD_RET_OK           manager variables initialized successfully
 * @retval STD_RET_ERROR        variables not initialized
 */
//==============================================================================
static stdRet_t init_program_management(void)
{
        if (pman.program_list != NULL && pman.mtx_program_list != NULL) {
                return STD_RET_OK;
        }

        pman.program_list = new_list();
        pman.mtx_program_list          = new_mutex();

        if (pman.program_list == NULL || pman.mtx_program_list == NULL) {
                if (pman.program_list == NULL) {
                        delete_list(pman.program_list);
                        pman.program_list = NULL;
                }

                if (pman.mtx_program_list == NULL) {
                        delete_mutex(pman.mtx_program_list);
                        pman.mtx_program_list = NULL;
                }

                return STD_RET_ERROR;
        }

        return STD_RET_OK;
}

//==============================================================================
/**
 * @brief Function create new table with argument pointers
 *
 * @param[in]  *arg             argument string
 * @param[in]  *name            program name (argument argv[0])
 * @param[out] *arg_count       number of argument
 *
 * @return argument table pointer if success, otherwise NULL
 */
//==============================================================================
static ch_t **new_argument_table(ch_t *arg, const ch_t *name, int *argc)
{
        int     arg_count  = 0;
        ch_t  **arg_table  = NULL;
        list_t *arg_list   = NULL;
        ch_t   *arg_string = NULL;

        if (arg == NULL || name == NULL || argc == NULL) {
                goto exit_error;
        }

        if ((arg_list = new_list()) == NULL) {
                goto exit_error;
        }

        if (list_add_item(arg_list, ++arg_count, (ch_t*)name) < 0) {
                goto exit_error;
        }

        if (arg[0] == '\0') {
                goto add_args_to_table;
        }

        if ((arg_string = calloc(strlen(arg) + 1, sizeof(ch_t))) == NULL) {
                goto exit_error;
        }

        strcpy(arg_string, arg);

        while (*arg_string != '\0') {
                ch_t *arg_to_add = NULL;

                if (*arg_string == '\'') {
                        arg_to_add = ++arg_string;

                        while (*arg_string != '\0') {
                                if ( *arg_string == '\''
                                   && (  *(arg_string + 1) == ' '
                                      || *(arg_string + 1) == '\0') ) {
                                        break;
                                }

                                arg_string++;
                        }

                        if (*arg_string == '\0') {
                                goto exit_error;
                        }

                } else if (*arg_string == '"') {
                        arg_to_add = ++arg_string;

                        while (*arg_string != '\0') {
                                if ( *arg_string == '"'
                                   && (  *(arg_string + 1) == ' '
                                      || *(arg_string + 1) == '\0') ) {
                                        break;
                                }

                                arg_string++;
                        }

                        if (*arg_string == '\0') {
                                goto exit_error;
                        }

                } else if (*arg_string != ' ') {
                        arg_to_add = arg_string;

                        while (*arg_string != ' ' && *arg_string != '\0') {
                                arg_string++;
                        }
                } else {
                        arg_string++;
                        continue;
                }

                /* add argument to list */
                if (arg_to_add == NULL) {
                        goto exit_error;
                }

                if (list_add_item(arg_list, ++arg_count, arg_to_add) < 0) {
                        goto exit_error;
                }

                /* terminate argument */
                if (*arg_string == '\0') {
                        break;
                } else {
                        *arg_string++ = '\0';
                }
        }

add_args_to_table:
        if ((arg_table = calloc(arg_count, sizeof(ch_t*))) == NULL) {
                goto exit_error;
        }

        for (int i = 0; i < arg_count; i++) {
                arg_table[i] = list_get_nitem_data(arg_list, 0);

                if (arg_table[i] == NULL) {
                        goto exit_error;
                }

                list_unlink_nitem_data(arg_list, 0);
                list_rm_nitem(arg_list, 0);
        }

        delete_list(arg_list);

        *argc = arg_count;
        return arg_table;


        /* error occurred - memory/object deallocation */
exit_error:
        if (arg_table) {
                free(arg_table);
        }

        if (arg_list) {
                i32_t items_in_list = list_get_item_count(arg_list);
                while (items_in_list-- > 0) {
                        list_unlink_nitem_data(arg_list, 0);
                        list_rm_nitem(arg_list, 0);
                }

                delete_list(arg_list);
        }

        if (arg_string) {
                free(arg_string);
        }

        *argc = 0;
        return NULL;
}

//==============================================================================
/**
 * @brief Function remove argument table
 *
 * @param **argv        pointer to argument table
 * @param   argc        argument count
 */
//==============================================================================
static void delete_argument_table(ch_t **argv, int argc)
{
        if (argv == NULL) {
                return;
        }

        if (argc > 1) {
                if (argv[1]) {
                        free(argv[1]);
                }
        }

        free(argv);
}

//==============================================================================
/**
 * @brief Function set exit code in selected task in the program list
 *
 * @param taskhdl       task handle
 * @param exit_code     exit code
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
static stdRet_t set_program_exit_code(task_t taskhdl, int exit_code)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)taskhdl);
        if (pdata) {
                pdata->exit_code = exit_code;
                mutex_unlock(pman.mtx_program_list);
                return STD_RET_OK;
        }
        mutex_unlock(pman.mtx_program_list);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function set program's pointer to global variables
 *
 * @param  taskhdl      task handle
 * @param *globals      pointer to global variables
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
static stdRet_t set_program_globals(task_t taskhdl, void *globals)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)taskhdl);
        if (pdata) {
                pdata->global_vars = globals;
                mutex_unlock(pman.mtx_program_list);
                return STD_RET_OK;
        }
        mutex_unlock(pman.mtx_program_list);

        return STD_RET_ERROR;
}

//==============================================================================
/**
 * @brief Function set program's status
 *
 * @param taskhdl       task handle
 * @param pstatus       program status
 *
 * @retval STD_RET_OK
 * @retval STD_RET_ERROR
 */
//==============================================================================
static stdRet_t set_program_status(task_t taskhdl, enum prg_status pstatus)
{
        struct program_data *pdata;

        while (mutex_lock(pman.mtx_program_list, MTX_BTIME_FOR_PLIST) != OS_OK);
        pdata = list_get_iditem_data(pman.program_list, (u32_t)taskhdl);
        if (pdata) {
                pdata->status = pstatus;
                mutex_unlock(pman.mtx_program_list);
                return STD_RET_OK;
        }
        mutex_unlock(pman.mtx_program_list);

        return STD_RET_ERROR;
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
