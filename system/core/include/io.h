#ifndef IO_H_
#define IO_H_
/*=========================================================================*//**
@file    io.h

@author  Daniel Zorychta

@brief   This file support standard io functions

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
#include <stdarg.h>
#include "basic_types.h"
#include "systypes.h"
#include "config.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
/** translate function to STDC */
#define printf(...)                             io_fprintf(stdout, __VA_ARGS__)
#define fprintf(...)                            io_fprintf(__VA_ARGS__)
#define snprintf(bfr, size, ...)                io_snprintf(bfr, size, __VA_ARGS__)
#define vsnprintf(bfr, size, args)              io_vsnprintf(bfr, size, args)
#define kprint(...)                             io_kprint(__VA_ARGS__)
#define kprintEnable(path)                      io_kprintEnable(path)
#define kprintDisable()                         io_kprintDisable()
#define scanf(format, ...)                      io_fscanf(stdin, format, __VA_ARGS__)
#define fscanf(stream, format, ...)             io_fscanf(stream, format, __VA_ARGS__)
#define sscanf(str, format, ...)                io_sscanf(str, format, __VA_ARGS__)
#define vsscanf(str, format, args)              io_vsscanf(str, format, args)
#define putc(c, stream)                         io_fputc(c, stream)
#define fputc(c, stream)                        io_fputc(c, stream)
#define putchar(c)                              io_fputc(c, stdout)
#define getchar()                               io_getc(stdin)
#define getc(stream)                            io_getc(stream)
#define fgets(str, size, stream)                io_fgets(str, size, stream)
#define atoi(string, base, valuePtr)            io_atoi(string, base, valuePtr)

/** VT100 terminal commands */
#define ENABLE_LINE_WRAP                        "\x1B[?7h"
#define SET_NEW_LINE_MODE                       "\x1B[20h"
#define SET_LINE_FEED_MODE                      "\x1B[20l"
#define CLRSRC                                  "\x1B[2J"
#define ERASE_LINE                              "\x1B[2K"
#define ERASE_LINE_END                          "\x1B[K"
#define CUR_HOME                                "\x1B[H"
#define RESET_ATTRIBUTES                        "\x1B[0m"
#define FONT_BLINKING                           "\x1B[5m"
#define FONT_UNDERLINE                          "\x1B[4m"
#define FONT_NORMAL                             "\x1B[0m"
#define FONT_BOLD                               "\x1B[1m"
#define FONT_COLOR_BLACK                        "\x1B[30m"
#define FONT_COLOR_RED                          "\x1B[31m"
#define FONT_COLOR_GREEN                        "\x1B[32m"
#define FONT_COLOR_YELLOW                       "\x1B[33m"
#define FONT_COLOR_BLUE                         "\x1B[34m"
#define FONT_COLOR_MAGENTA                      "\x1B[35m"
#define FONT_COLOR_CYAN                         "\x1B[36m"
#define FONT_COLOR_WHITE                        "\x1B[37m"
#define BACK_COLOR_BLACK                        "\x1B[40m"
#define BACK_COLOR_RED                          "\x1B[41m"
#define BACK_COLOR_GREEN                        "\x1B[42m"
#define BACK_COLOR_YELLOW                       "\x1B[43m"
#define BACK_COLOR_BLUE                         "\x1B[44m"
#define BACK_COLOR_MAGENTA                      "\x1B[45m"
#define BACK_COLOR_CYAN                         "\x1B[46m"
#define BACK_COLOR_WHITE                        "\x1B[47m"

/** key definitions */
#define ASCII_LF                                0x0A
#define ASCII_CR                                0x0D
#define ASCII_BS                                0x08
#define ASCII_NULL                              0x00
#define ASCII_ESC                               0x1B
#define ASCII_CANCEL                            0x18

/** stream values */
#define EOF                                     (-1)

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/

extern ch_t  *io_atoi(ch_t *string, u8_t base, i32_t *value);
extern int_t  io_kprint(const ch_t *format, ...);
extern int_t  io_snprintf(ch_t *buf, u32_t size, const ch_t *format, ...);
extern int_t  io_fprintf(FILE_t *file, const ch_t *format, ...);
extern int_t  io_vsnprintf(ch_t *buf, size_t size, const ch_t *format, va_list arg);
extern void   io_kprintEnable(ch_t *filename);
extern void   io_kprintDisable(void);
extern int_t  io_fputc(int_t c, FILE_t *buf);
extern int_t  io_getc(FILE_t *buf);
extern ch_t  *io_fgets(ch_t *str, int_t size, FILE_t *stream);
extern int_t  io_fscanf(FILE_t *buf, const ch_t *format, ...);
extern int_t  io_sscanf(const ch_t *str, const ch_t *format, ...);
extern int_t  io_vsscanf(const ch_t *str, const ch_t *format, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* IO_H_ */
/*==============================================================================
  End of file
==============================================================================*/