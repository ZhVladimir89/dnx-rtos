/*=========================================================================*//**
@file    telnet.c

@author  Daniel Zorychta

@brief   The simple example program

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

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include <stdio.h>
#include <string.h>
#include "system/dnx.h"
#include "system/netapi.h"

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
GLOBAL_VARIABLES_SECTION_BEGIN
/* put here global variables */
GLOBAL_VARIABLES_SECTION_END

/*==============================================================================
  Exported object definitions
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

static void shell_main(netapi_conn_t *conn)
{
        while (true) {
                netapi_buf_t *rx_buffer;
                if (netapi_recv(conn, &rx_buffer) == NETAPI_ERR_OK) {
                        char *string;
                        u16_t len;
                        if (netapi_buf_data(rx_buffer, (void *)&string, &len) == NETAPI_ERR_OK) {
                                fwrite(string, 1, len, stdout);

                                if (strncmp(string, "test\r\n", len) == 0) {
                                        puts("Command detected!");

                                        const char *str = "::Connection test::\n";
                                        netapi_write(conn, str, strlen(str), NETAPI_CONN_FLAG_COPY);

                                        char *test = calloc(1, 50);
                                        strcpy(test, "Test buffer...\n");
                                        netapi_write(conn, test, strlen(test), NETAPI_CONN_FLAG_COPY);
                                        free(test);
                                }
                        }

                        netapi_delete_buf(rx_buffer);
                } else {
                        puts("Rx buffer creation error!");
                        return;
                }
        }
}

//==============================================================================
/**
 * @brief Program main function
 *
 * @param  argc         count of arguments
 * @param *argv[]       argument table
 *
 * @return program status
 */
//==============================================================================
PROGRAM_MAIN(telnet, int argc, char *argv[])
{
        (void) argc;
        (void) argv;

        netapi_conn_t *listener = netapi_new_conn(NETAPI_CONN_TYPE_TCP);
        if (listener) {
                puts("New connection created");

                if (netapi_bind(listener, (netapi_ip_t *)&ip_addr_any, 23) == NETAPI_ERR_OK) {
                        puts("Binded successfully");

                        if (netapi_listen(listener) == NETAPI_ERR_OK) {
                                puts("Successfully goes to listen mode");

                                for (;;) {
                                        netapi_conn_t *new_connection;
                                        if (netapi_accept(listener, &new_connection) == NETAPI_ERR_OK) {
                                                printf("New connection: 0x%x\n", new_connection);
                                                shell_main(new_connection);
                                                netapi_delete_conn(new_connection);
                                        } else {
                                                puts("Acceptance error!");
                                        }
                                }
                        } else {
                                puts("Unable to start in listen mode!");
                        }
                } else {
                        puts("Address and port bind error!");
                }
        } else {
                puts("Unable to create connection!");
        }

        return -1;
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
