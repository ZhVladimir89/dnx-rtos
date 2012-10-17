/*=============================================================================================*//**
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


*//*==============================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                            Include files
==================================================================================================*/
#include "initd.h"
#include "regdrv.h"
#include "tty.h"
#include <string.h>

#include "uart.h"
#include "ether.h"
#include "netconf.h"
#include "ds1307.h"
#include "MPL115A2.h"

#include "lwiptest.h"
#include "httpde.h"


/*==================================================================================================
                                  Local symbolic constants/macros
==================================================================================================*/


/*==================================================================================================
                                   Local types, enums definitions
==================================================================================================*/


/*==================================================================================================
                                      Local function prototypes
==================================================================================================*/
static i8_t decodeFn(ch_t character);


/*==================================================================================================
                                      Local object definitions
==================================================================================================*/


/*==================================================================================================
                                     Exported object definitions
==================================================================================================*/


/*==================================================================================================
                                        Function definitions
==================================================================================================*/

void test1(void *arg)
{
      (void) arg;

      for (;;)
      {
            TTY_AddMsg(0, "Test TTY0\r\n");
            Sleep(1000);
      }
}



void test2(void *arg)
{
      (void) arg;

      for (;;)
      {
            TTY_AddMsg(1, "-=Test TTY1=-\r\n");
            Sleep(2000);
      }
}


//================================================================================================//
/**
 * @brief Task which initialise high-level devices/applications etc
 * Task is responsible for low-level application runtime environment (stdio). Task connect
 * applications' stdios with hardware layer.
 */
//================================================================================================//
void Initd(void *arg)
{
      (void) arg;

      /* short delay and lock task scheduling */
      TaskDelay(800);

      /*--------------------------------------------------------------------------------------------
       * initialization kprint()
       *------------------------------------------------------------------------------------------*/
      InitDrv("uart1");
//      UART_Init(UART_DEV_1);
      UART_Open(UART_DEV_1);
      kprintEnable();

      /* VT100 terminal configuration */
      clrscr(k);
      enableLineWrap(k);

      /* something about board and system */
      kprint("Board powered by "); fontGreen(k); kprint("FreeRTOS\n"); resetAttr(k);

      kprint("By "); fontCyan(k); kprint("Daniel Zorychta ");
      fontYellow(k); kprint("<daniel.zorychta@gmail.com>\n\n"); resetAttr(k);
      TaskDelay(1000);

      /* info about system start */
      kprint("[%d] initd: kernel print started\n", TaskGetTickCount());
      kprint("[%d] initd: init daemon started\n", TaskGetTickCount());

      /*--------------------------------------------------------------------------------------------
       * user initialization
       *------------------------------------------------------------------------------------------*/
//      if (ETHER_Init(ETH_DEV_1) != STD_RET_OK)
//            goto initd_net_end;
//
//      if (LwIP_Init() != STD_RET_OK)
//            goto initd_net_end;
//
//      kprint("Starting telnetd... ");
//      if (TaskCreate(telnetd, "telnetd", TELNETD_STACK_SIZE, NULL, 2, NULL) == pdPASS)
//      {
//            fontGreen(k);
//            kprint("SUCCESS\n");
//      }
//      else
//      {
//            fontRed(k);
//            kprint("FAILED\n");
//      }
//      resetAttr(k);
//
//      kprint("Starting httpde...");
//      if (TaskCreate(httpd_init, "httpde", HTTPDE_STACK_SIZE, NULL, 2, NULL) == pdPASS)
//      {
//            kprintOK();
//      }
//      else
//      {
//            kprintFail();
//      }
//
//      initd_net_end:
//
//      DS1307_Init();
//
//      MPL115A2_Init();

      /*--------------------------------------------------------------------------------------------
       * starting terminal
       *------------------------------------------------------------------------------------------*/
//      kprint("[%d] initd: starting interactive console... ", TaskGetTickCount());

      /* try to start terminal */
//      appArgs_t *appHdl = Exec("terminal", NULL);
//
//      if (appHdl == NULL)
//      {
//            kprintFail();
//            kprint("Probably no enough free space. Restarting board...");
//            TaskResumeAll();
//            TaskDelay(5000);
//            SystemReboot();
//      }
//      else
//      {
//            kprintOK();
//      }

      /* initd info about stack usage */
      kprint("[%d] initd: free stack: %d levels\n\n", TaskGetTickCount(), TaskGetStackFreeSpace(THIS_TASK));


      TaskCreate(test1, "testTTY0", MINIMAL_STACK_SIZE, NULL, 3, NULL);
      TaskCreate(test2, "testTTY1", MINIMAL_STACK_SIZE, NULL, 3, NULL);


      /*--------------------------------------------------------------------------------------------
       * main loop which read stdios from applications
       *------------------------------------------------------------------------------------------*/
      u8_t   currentTTY  = 0;
      ch_t   character;
      bool_t stdoutEmpty = FALSE;
      bool_t RxFIFOEmpty = FALSE;

      for (;;)
      {

            /* STDOUT support ------------------------------------------------------------------- */
//            if ((character = ufgetChar(appHdl->stdout)) != ASCII_CANCEL)
//            {
//                  UART_IOCtl(UART_DEV_1, UART_IORQ_SEND_BYTE, &character);
//                  stdoutEmpty = FALSE;
//            }
//            else
//            {
//                  stdoutEmpty = TRUE;
//            }

            if (TTY_CheckNewMsg(currentTTY))
            {
                  ch_t *msg = TTY_GetMsg(currentTTY, TTY_LAST_MSG);

                  if (msg)
                  {
                        UART_Write(UART_DEV_1, msg, strlen(msg), 0);
                  }
            }

            /* STDIN support -------------------------------------------------------------------- */
            if (UART_IOCtl(UART_DEV_1, UART_IORQ_GET_BYTE, &character) == STD_RET_OK)
            {
//                  i8_t keyFn = decodeFn(character);

                  if (character >= '0' && character <= '4')
                  {
                        if (currentTTY != (character - '0'))
                        {
                              currentTTY = character - '0';

                              ch_t *clrscr = "\x1B[2J";
                              UART_Write(UART_DEV_1, clrscr, strlen(clrscr), 0);

                              for (u8_t i = 0; i < TTY_MSGS; i++)
                              {
                                    ch_t *msg = TTY_GetMsg(currentTTY, i);

                                    if (msg)
                                    {
                                          UART_Write(UART_DEV_1, msg, strlen(msg), 0);
                                    }
                              }
                        }
                  }

//                  if (keyFn == -1)
//                  {
////                        ufputChar(appHdl->stdin, character);
//                        RxFIFOEmpty = FALSE;
//                  }
//                  else if (keyFn > 0)
//                  {
//                        currentTTY = keyFn - 1;
//                  }
            }
//            else
//            {
//                  RxFIFOEmpty = TRUE;
//            }

            /* application monitoring ----------------------------------------------------------- */
//            if (appHdl->exitCode != STD_RET_UNKNOWN)
//            {
//                  if (appHdl->exitCode == STD_RET_OK)
//                        kprint("\n[%d] initd: terminal was terminated.\n", TaskGetTickCount());
//                  else
//                        kprint("\n[%d] initd: terminal was terminated with error.\n", TaskGetTickCount());
//
//                  FreeStdio(appHdl);
//
//                  kprint("[%d] initd: disable FreeRTOS scheduler. Bye.\n", TaskGetTickCount());
//
//                  vTaskEndScheduler();
//
//                  while (TRUE)
//                        TaskDelay(1000);
//            }

            /* wait state */
//            if (stdoutEmpty && RxFIFOEmpty)
//                  TaskDelay(1);
      }

      /* this should never happen */
      TaskTerminate();
}



static i8_t decodeFn(ch_t character)
{
      static u8_t funcStep;

      /* try detect function keys ^[OP */
      switch (funcStep)
      {
            case 0:
            {
                  if (character == ASCII_ESC)
                        funcStep++;
                  else
                        funcStep = 0;
                  break;
            }

//            case 1:
//            {
//                  if (character == '[')
//                        funcStep++;
//                  else
//                        funcStep = 0;
//                  break;
//            }

            case 1:
            {
                  if (character == 'O')
                        funcStep++;
                  else
                        funcStep = 0;
                  break;
            }

            case 2:
            {
                  if (character == 'P')
                  {
                        return 1;
                  }
                  else if (character == 'Q')
                  {
                        return 2;
                  }
                  else if (character == 'R')
                  {
                        return 3;
                  }
                  else if (character == 'S')
                  {
                        return 4;
                  }
                  break;
            }

            default:
                  funcStep = 0;
      }

      if (funcStep)
            return 0;
      else
            return -1;
}



#ifdef __cplusplus
}
#endif

/*==================================================================================================
                                            End of file
==================================================================================================*/
