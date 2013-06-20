/*=========================================================================*//**
@file    libfat.c

@author  Daniel Zorychta

@brief   FAT file system library based od ChaN's code.

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
/*-----------------------------------------------------------------------------/
/  FatFs - FAT file system module  R0.09b                 (C)ChaN, 2013
/------------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following terms.
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Include files
==============================================================================*/
#include <string.h>
#include "libfat.h"
#include "libfat_user.h"

/*==============================================================================
  Local symbolic constants/macros
==============================================================================*/
/* Definitions on sector size */
#if _LIBFAT_MAX_SS != 512 && _LIBFAT_MAX_SS != 1024 && _LIBFAT_MAX_SS != 2048 && _LIBFAT_MAX_SS != 4096
#       error Wrong sector size.
#endif

#if _LIBFAT_MAX_SS != 512
#       define SS(fs)           ((fs)->ssize)   /* Variable sector size */
#else
#       define SS(fs)           512U            /* Fixed sector size */
#endif

/* Reentrancy related */
#define ENTER_FF(fs)            {if (!lock_fs(fs)) return FR_TIMEOUT;}
#define LEAVE_FF(fs, res)       {unlock_fs(fs, res); return res;}

#define ABORT(fs, res)          {fp->flag |= LIBFAT_FA__ERROR; LEAVE_FF(fs, res);}

/* DBCS code ranges and SBCS extend char conversion table */
#if   _LIBFAT_CODE_PAGE == 932        /* Japanese Shift-JIS */
#define _DF1S        0x81             /* DBC 1st byte range 1 start */
#define _DF1E        0x9F             /* DBC 1st byte range 1 end */
#define _DF2S        0xE0             /* DBC 1st byte range 2 start */
#define _DF2E        0xFC             /* DBC 1st byte range 2 end */
#define _DS1S        0x40             /* DBC 2nd byte range 1 start */
#define _DS1E        0x7E             /* DBC 2nd byte range 1 end */
#define _DS2S        0x80             /* DBC 2nd byte range 2 start */
#define _DS2E        0xFC             /* DBC 2nd byte range 2 end */

#elif _LIBFAT_CODE_PAGE == 936        /* Simplified Chinese GBK */
#define _DF1S        0x81
#define _DF1E        0xFE
#define _DS1S        0x40
#define _DS1E        0x7E
#define _DS2S        0x80
#define _DS2E        0xFE

#elif _LIBFAT_CODE_PAGE == 949        /* Korean */
#define _DF1S        0x81
#define _DF1E        0xFE
#define _DS1S        0x41
#define _DS1E        0x5A
#define _DS2S        0x61
#define _DS2E        0x7A
#define _DS3S        0x81
#define _DS3E        0xFE

#elif _LIBFAT_CODE_PAGE == 950        /* Traditional Chinese Big5 */
#define _DF1S        0x81
#define _DF1E        0xFE
#define _DS1S        0x40
#define _DS1E        0x7E
#define _DS2S        0xA1
#define _DS2E        0xFE

#elif _LIBFAT_CODE_PAGE == 437        /* U.S. (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x90,0x41,0x8E,0x41,0x8F,0x80,0x45,0x45,0x45,0x49,0x49,0x49,0x8E,0x8F,0x90,0x92,0x92,0x4F,0x99,0x4F,0x55,0x55,0x59,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0x41,0x49,0x4F,0x55,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 720        /* Arabic (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x45,0x41,0x84,0x41,0x86,0x43,0x45,0x45,0x45,0x49,0x49,0x8D,0x8E,0x8F,0x90,0x92,0x92,0x93,0x94,0x95,0x49,0x49,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 737        /* Greek (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x92,0x92,0x93,0x94,0x95,0x96,0x97,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87, \
                0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0xAA,0x92,0x93,0x94,0x95,0x96,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0x97,0xEA,0xEB,0xEC,0xE4,0xED,0xEE,0xE7,0xE8,0xF1,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 775        /* Baltic (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x91,0xA0,0x8E,0x95,0x8F,0x80,0xAD,0xED,0x8A,0x8A,0xA1,0x8D,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0x95,0x96,0x97,0x97,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xE0,0xA3,0xA3,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xB5,0xB6,0xB7,0xB8,0xBD,0xBE,0xC6,0xC7,0xA5,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE3,0xE8,0xE8,0xEA,0xEA,0xEE,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 850        /* Multilingual Latin 1 (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0xDE,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x59,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
                0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE7,0xE9,0xEA,0xEB,0xED,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 852        /* Latin 2 (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xDE,0x8F,0x80,0x9D,0xD3,0x8A,0x8A,0xD7,0x8D,0x8E,0x8F,0x90,0x91,0x91,0xE2,0x99,0x95,0x95,0x97,0x97,0x99,0x9A,0x9B,0x9B,0x9D,0x9E,0x9F, \
                0xB5,0xD6,0xE0,0xE9,0xA4,0xA4,0xA6,0xA6,0xA8,0xA8,0xAA,0x8D,0xAC,0xB8,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBD,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC6,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD2,0xD3,0xD2,0xD5,0xD6,0xD7,0xB7,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE3,0xD5,0xE6,0xE6,0xE8,0xE9,0xE8,0xEB,0xED,0xED,0xDD,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xEB,0xFC,0xFC,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 855        /* Cyrillic (OEM) */
#define _DF1S        0
#define _EXCVT {0x81,0x81,0x83,0x83,0x85,0x85,0x87,0x87,0x89,0x89,0x8B,0x8B,0x8D,0x8D,0x8F,0x8F,0x91,0x91,0x93,0x93,0x95,0x95,0x97,0x97,0x99,0x99,0x9B,0x9B,0x9D,0x9D,0x9F,0x9F, \
                0xA1,0xA1,0xA3,0xA3,0xA5,0xA5,0xA7,0xA7,0xA9,0xA9,0xAB,0xAB,0xAD,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB6,0xB6,0xB8,0xB8,0xB9,0xBA,0xBB,0xBC,0xBE,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD3,0xD3,0xD5,0xD5,0xD7,0xD7,0xDD,0xD9,0xDA,0xDB,0xDC,0xDD,0xE0,0xDF, \
                0xE0,0xE2,0xE2,0xE4,0xE4,0xE6,0xE6,0xE8,0xE8,0xEA,0xEA,0xEC,0xEC,0xEE,0xEE,0xEF,0xF0,0xF2,0xF2,0xF4,0xF4,0xF6,0xF6,0xF8,0xF8,0xFA,0xFA,0xFC,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 857        /* Turkish (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0x98,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x98,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9E, \
                0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA6,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xDE,0x59,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 858        /* Multilingual Latin 1 + Euro (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0xDE,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x59,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, \
                0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD1,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE7,0xE9,0xEA,0xEB,0xED,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 862        /* Hebrew (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0x41,0x49,0x4F,0x55,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 866        /* Russian (OEM) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0x90,0x91,0x92,0x93,0x9d,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xF0,0xF0,0xF2,0xF2,0xF4,0xF4,0xF6,0xF6,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 874        /* Thai (OEM, Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 1250       /* Central Europe (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x8D,0x8E,0x8F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xA3,0xB4,0xB5,0xB6,0xB7,0xB8,0xA5,0xAA,0xBB,0xBC,0xBD,0xBC,0xAF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF}

#elif _LIBFAT_CODE_PAGE == 1251       /* Cyrillic (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x82,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x80,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x8D,0x8E,0x8F, \
                0xA0,0xA2,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB2,0xA5,0xB5,0xB6,0xB7,0xA8,0xB9,0xAA,0xBB,0xA3,0xBD,0xBD,0xAF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF}

#elif _LIBFAT_CODE_PAGE == 1252       /* Latin 1 (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0xAd,0x9B,0x8C,0x9D,0xAE,0x9F, \
                0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0x9F}

#elif _LIBFAT_CODE_PAGE == 1253       /* Greek (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xA2,0xB8,0xB9,0xBA, \
                0xE0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xF2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xFB,0xBC,0xFD,0xBF,0xFF}

#elif _LIBFAT_CODE_PAGE == 1254       /* Turkish (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x9D,0x9E,0x9F, \
                0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0x9F}

#elif _LIBFAT_CODE_PAGE == 1255       /* Hebrew (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 1256       /* Arabic (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x8C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0x41,0xE1,0x41,0xE3,0xE4,0xE5,0xE6,0x43,0x45,0x45,0x45,0x45,0xEC,0xED,0x49,0x49,0xF0,0xF1,0xF2,0xF3,0x4F,0xF5,0xF6,0xF7,0xF8,0x55,0xFA,0x55,0x55,0xFD,0xFE,0xFF}

#elif _LIBFAT_CODE_PAGE == 1257       /* Baltic (Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F, \
                0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xA8,0xB9,0xAA,0xBB,0xBC,0xBD,0xBE,0xAF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xFF}

#elif _LIBFAT_CODE_PAGE == 1258       /* Vietnam (OEM, Windows) */
#define _DF1S        0
#define _EXCVT {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0xAC,0x9D,0x9E,0x9F, \
                0xA0,0x21,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, \
                0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xEC,0xCD,0xCE,0xCF,0xD0,0xD1,0xF2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xFE,0x9F}

#elif _LIBFAT_CODE_PAGE == 1          /* ASCII (for only non-LFN cfg) */
#       if _LIBFAT_USE_LFN
#               error Cannot use LFN feature without valid code page.
#       endif
#define _DF1S        0

#else
#       error Unknown code page
#endif

/* Character code support macros */
#define IsUpper(c)              (((c) >= 'A') && ((c) <= 'Z'))
#define IsLower(c)              (((c) >= 'a') && ((c) <= 'z'))
#define IsDigit(c)              (((c) >= '0') && ((c) <= '9'))

/* Code page is DBCS */
#if _DF1S
#       ifdef _DF2S /* Two 1st byte areas */
#               define IsDBCS1(c)       (((uint8_t)(c) >= _DF1S && (uint8_t)(c) <= _DF1E) || ((uint8_t)(c) >= _DF2S && (uint8_t)(c) <= _DF2E))
#       else                        /* One 1st byte area */
#               define IsDBCS1(c)       ((uint8_t)(c) >= _DF1S && (uint8_t)(c) <= _DF1E)
#       endif
#
#       ifdef _DS3S        /* Three 2nd byte areas */
#               define IsDBCS2(c)       (((uint8_t)(c) >= _DS1S && (uint8_t)(c) <= _DS1E) || ((uint8_t)(c) >= _DS2S && (uint8_t)(c) <= _DS2E) || ((uint8_t)(c) >= _DS3S && (uint8_t)(c) <= _DS3E))
#       else                        /* Two 2nd byte areas */
#               define IsDBCS2(c)       (((uint8_t)(c) >= _DS1S && (uint8_t)(c) <= _DS1E) || ((uint8_t)(c) >= _DS2S && (uint8_t)(c) <= _DS2E))
#       endif
#else /* Code page is SBCS */
#       define IsDBCS1(c)               0
#       define IsDBCS2(c)               0
#endif

/* Name status flags */
#define NS                              11          /* Index of name status byte in fn[] */
#define NS_LOSS                         0x01        /* Out of 8.3 format */
#define NS_LFN                          0x02        /* Force to create LFN entry */
#define NS_LAST                         0x04        /* Last segment */
#define NS_BODY                         0x08        /* Lower case flag (body) */
#define NS_EXT                          0x10        /* Lower case flag (ext) */
#define NS_DOT                          0x20        /* Dot entry */

/* FAT sub-type boundaries */
/* Note that the FAT spec by Microsoft says 4085 but Windows works with 4087! */
#define MIN_FAT16                       4086        /* Minimum number of clusters for FAT16 */
#define MIN_FAT32                       65526       /* Minimum number of clusters for FAT32 */

/* FatFs refers the members in the FAT structures as byte array instead of
/ structure member because the structure is not binary compatible between
/ different platforms */
#define BS_jmpBoot                      0           /* Jump instruction (3) */
#define BS_OEMName                      3           /* OEM name (8) */
#define BPB_BytsPerSec                  11          /* Sector size [byte] (2) */
#define BPB_SecPerClus                  13          /* Cluster size [sector] (1) */
#define BPB_RsvdSecCnt                  14          /* Size of reserved area [sector] (2) */
#define BPB_NumFATs                     16          /* Number of FAT copies (1) */
#define BPB_RootEntCnt                  17          /* Number of root dir entries for FAT12/16 (2) */
#define BPB_TotSec16                    19          /* Volume size [sector] (2) */
#define BPB_Media                       21          /* Media descriptor (1) */
#define BPB_FATSz16                     22          /* FAT size [sector] (2) */
#define BPB_SecPerTrk                   24          /* Track size [sector] (2) */
#define BPB_NumHeads                    26          /* Number of heads (2) */
#define BPB_HiddSec                     28          /* Number of special hidden sectors (4) */
#define BPB_TotSec32                    32          /* Volume size [sector] (4) */
#define BS_DrvNum                       36          /* Physical drive number (2) */
#define BS_BootSig                      38          /* Extended boot signature (1) */
#define BS_VolID                        39          /* Volume serial number (4) */
#define BS_VolLab                       43          /* Volume label (8) */
#define BS_FilSysType                   54          /* File system type (1) */
#define BPB_FATSz32                     36          /* FAT size [sector] (4) */
#define BPB_ExtFlags                    40          /* Extended flags (2) */
#define BPB_FSVer                       42          /* File system version (2) */
#define BPB_RootClus                    44          /* Root dir first cluster (4) */
#define BPB_FSInfo                      48          /* Offset of FSInfo sector (2) */
#define BPB_BkBootSec                   50          /* Offset of backup boot sector (2) */
#define BS_DrvNum32                     64          /* Physical drive number (2) */
#define BS_BootSig32                    66          /* Extended boot signature (1) */
#define BS_VolID32                      67          /* Volume serial number (4) */
#define BS_VolLab32                     71          /* Volume label (8) */
#define BS_FilSysType32                 82          /* File system type (1) */
#define FSI_LeadSig                     0           /* FSI: Leading signature (4) */
#define FSI_StrucSig                    484         /* FSI: Structure signature (4) */
#define FSI_Free_Count                  488         /* FSI: Number of free clusters (4) */
#define FSI_Nxt_Free                    492         /* FSI: Last allocated cluster (4) */
#define MBR_Table                       446         /* MBR: Partition table offset (2) */
#define SZ_PTE                          16          /* MBR: Size of a partition table entry */
#define BS_55AA                         510         /* Boot sector signature (2) */

#define DIR_Name                        0           /* Short file name (11) */
#define DIR_Attr                        11          /* Attribute (1) */
#define DIR_NTres                       12          /* NT flag (1) */
#define DIR_CrtTimeTenth                13          /* Created time sub-second (1) */
#define DIR_CrtTime                     14          /* Created time (2) */
#define DIR_CrtDate                     16          /* Created date (2) */
#define DIR_LstAccDate                  18          /* Last accessed date (2) */
#define DIR_FstClusHI                   20          /* Higher 16-bit of first cluster (2) */
#define DIR_WrtTime                     22          /* Modified time (2) */
#define DIR_WrtDate                     24          /* Modified date (2) */
#define DIR_FstClusLO                   26          /* Lower 16-bit of first cluster (2) */
#define DIR_FileSize                    28          /* File size (4) */
#define LDIR_Ord                        0           /* LFN entry order and LLE flag (1) */
#define LDIR_Attr                       11          /* LFN attribute (1) */
#define LDIR_Type                       12          /* LFN type (1) */
#define LDIR_Chksum                     13          /* Sum of corresponding SFN entry */
#define LDIR_FstClusLO                  26          /* Filled by zero (0) */
#define SZ_DIR                          32          /* Size of a directory entry */
#define LLE                             0x40        /* Last long entry flag in LDIR_Ord */
#define DDE                             0xE5        /* Deleted directory entry mark in DIR_Name[0] */
#define NDDE                            0x05        /* Replacement of the character collides with DDE */

#define LOAD_UINT16(ptr)                (uint16_t)(((uint16_t)*((uint8_t*)(ptr)+1)<<8)|(uint16_t)*(uint8_t*)(ptr))
#define LOAD_UINT32(ptr)                (uint32_t)(((uint32_t)*((uint8_t*)(ptr)+3)<<24)|((uint32_t)*((uint8_t*)(ptr)+2)<<16)|((uint16_t)*((uint8_t*)(ptr)+1)<<8)|*(uint8_t*)(ptr))
#define STORE_UINT16(ptr,val)           *(uint8_t*)(ptr)=(uint8_t)(val); *((uint8_t*)(ptr)+1)=(uint8_t)((uint16_t)(val)>>8)
#define STORE_UINT32(ptr,val)           *(uint8_t*)(ptr)=(uint8_t)(val); *((uint8_t*)(ptr)+1)=(uint8_t)((uint16_t)(val)>>8); *((uint8_t*)(ptr)+2)=(uint8_t)((uint32_t)(val)>>16); *((uint8_t*)(ptr)+3)=(uint8_t)((uint32_t)(val)>>24)

#if _LIBFAT_USE_LFN == 0
#       define DEF_NAMEBUF              uint8_t sfn[12]
#       define INIT_BUF(dobj)           (dobj).fn = sfn
#       define FREE_BUF()
#elif _LIBFAT_USE_LFN == 1
#       define DEF_NAMEBUF              uint8_t sfn[12]; wchar_t lbuf[_LIBFAT_MAX_LFN+1]
#       define INIT_BUF(dobj)           {(dobj).fn = sfn; (dobj).lfn = lbuf;}
#       define FREE_BUF()
#elif _LIBFAT_USE_LFN == 2
#       define DEF_NAMEBUF              uint8_t sfn[12]; wchar_t *lfn
#       define INIT_BUF(dobj)           {lfn = _libfat_malloc((_LIBFAT_MAX_LFN + 1) * 2);  \
                                                if (!lfn) LEAVE_FF((dobj).fs, FR_NOT_ENOUGH_CORE); \
                                                (dobj).lfn = lfn; (dobj).fn = sfn;}
#       define  FREE_BUF()              _libfat_free(lfn)
#else
#       error Wrong LFN configuration.
#endif

/*==============================================================================
  Local types, enums definitions
==============================================================================*/
/* File access control feature */
#if _LIBFAT_FS_LOCK
typedef struct {
        FATFS   *fs;    /* File ID 1, volume (NULL:blank entry) */
        uint32_t clu;   /* File ID 2, directory */
        uint16_t idx;   /* File ID 3, directory index */
        uint16_t ctr;   /* File open counter, 0:none, 0x01..0xFF:read open count, 0x100:write mode */
} FILESEM;
#endif

#if _LIBFAT_FS_LOCK /* FIXME this shall be inserted in the FAT container */
static
FILESEM Files[_LIBFAT_FS_LOCK];        /* File lock semaphores */
#endif

#ifdef _EXCVT
static
const uint8_t ExCvt[] = _EXCVT;        /* Upper conversion table for extended chars */
#endif

/*==============================================================================
  Local function prototypes
==============================================================================*/
#if _LIBFAT_USE_LFN
extern wchar_t _libfat_convert  (wchar_t chr, uint dir);
extern wchar_t _libfat_wtoupper (wchar_t chr);
#endif

static int      lock_fs         (FATFS *fs);
static void     unlock_fs       (FATFS *fs, FRESULT res);
#if _LIBFAT_FS_LOCK
static FRESULT  chk_lock        (FATDIR *dj, int acc);
static int      enq_lock        (void);
static uint     inc_lock        (FATDIR *dj, int acc);
static FRESULT  dec_lock        (uint i)
static void     clear_lock      (FATFS *fs);
#endif

static FRESULT  sync_window     (FATFS *fs);
static FRESULT  move_window     (FATFS *fs, uint32_t sector);

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
 * @brief Request grant to access the volume
 *
 * @param[in] *fs       File system object
 *
 * @retval 1: Function succeeded
 * @retval 0: Could not create due to any error
 */
//==============================================================================
static int lock_fs(FATFS *fs)
{
        return _libfat_lock_access(fs->sobj);
}

//==============================================================================
/**
 * @brief Release grant to access the volume
 *
 * @param[in] *fs       File system object
 * @param[in]  res      Result code to be returned
 */
//==============================================================================
static void unlock_fs(FATFS *fs, FRESULT res)
{
        if (fs
           && res != FR_NOT_ENABLED
           && res != FR_INVALID_DRIVE
           && res != FR_INVALID_OBJECT
           && res != FR_TIMEOUT) {
                _libfat_unlock_access(fs->sobj);
        }
}

#if _LIBFAT_FS_LOCK
//==============================================================================
/**
 * @brief Check if the file can be accessed
 *
 * @param[in] *dj       Directory object pointing the file to be checked
 * @param[in]  acc      Desired access (0:Read, 1:Write, 2:Delete/Rename)
 *
 * @retval FR_OK
 * @retval FR_LOCKED
 */
//==============================================================================
static FRESULT chk_lock(FATDIR *dj, int acc)
{
        uint i, be;

        /* Search file semaphore table */
        for (i = be = 0; i < _LIBFAT_FS_LOCK; i++) {
                /* Existing entry */
                if (Files[i].fs) {
                        /* Check if the file matched with an open file */
                        if (Files[i].fs == dj->fs && Files[i].clu == dj->sclust && Files[i].idx == dj->index)
                                break;
                } else { /* Blank entry */
                        be++;
                }
        }

        /* The file is not opened */
        if (i == _LIBFAT_FS_LOCK) {
                /* Is there a blank entry for new file? */
                return (be || acc == 2) ? FR_OK : FR_TOO_MANY_OPEN_FILES;
        }

        /* The file has been opened. Reject any open against writing file and all write mode open */
        return (acc || Files[i].ctr == 0x100) ? FR_LOCKED : FR_OK;
}

//==============================================================================
/**
 * @brief Check if an entry is available for a new file
 *
 * @retval 0 unlocked
 * @retval 1 locked
 */
//==============================================================================
static int enq_lock(void)
{
        uint i;

        for (i = 0; i < _LIBFAT_FS_LOCK && Files[i].fs; i++);

        return (i == _LIBFAT_FS_LOCK) ? 0 : 1;
}

//==============================================================================
/**
 * @brief Increment file open counter and returns its index
 *
 * @param[in] *dj       Directory object pointing the file to be checked
 * @param[in]  acc      Desired access mode (0:Read, !0:Write)
 *
 * @retval 0 internal error
 */
//==============================================================================
static uint inc_lock(FATDIR *dj, int acc)
{
        uint i;

        /* Find the file */
        for (i = 0; i < _LIBFAT_FS_LOCK; i++) {
                if (Files[i].fs == dj->fs && Files[i].clu == dj->sclust && Files[i].idx == dj->index)
                        break;
        }

        /* Not opened. Register it as new. */
        if (i == _LIBFAT_FS_LOCK) {
                for (i = 0; i < _LIBFAT_FS_LOCK && Files[i].fs; i++);

                /* No space to register (int err) */
                if (i == _LIBFAT_FS_LOCK)
                        return 0;

                Files[i].fs  = dj->fs;
                Files[i].clu = dj->sclust;
                Files[i].idx = dj->index;
                Files[i].ctr = 0;
        }

        /* Access violation (int err) */
        if (acc && Files[i].ctr)
                return 0;

        /* Set semaphore value */
        Files[i].ctr = acc ? 0x100 : Files[i].ctr + 1;

        return i + 1;
}

//==============================================================================
/**
 * @brief Decrement file open counter
 *
 * @param[in] i         Semaphore index
 *
 * @retval FR_OK
 * @retval FR_INT_ERR
 */
//==============================================================================
static FRESULT dec_lock(uint i)
{
        uint16_t n;
        FRESULT res;

        if (--i < _LIBFAT_FS_LOCK) {
                n = Files[i].ctr;

                if (n == 0x100)
                        n = 0;

                if (n)
                        n--;

                Files[i].ctr = n;

                if (!n)
                        Files[i].fs = 0;

                res = FR_OK;
        } else {
                res = FR_INT_ERR;
        }
        return res;
}

//==============================================================================
/**
 * @brief Clear lock entries of the volume
 *
 * @param[in] *fs       File system object
 */
//==============================================================================
static void clear_lock(FATFS *fs)
{
        uint i;

        for (i = 0; i < _LIBFAT_FS_LOCK; i++) {
                if (Files[i].fs == fs)
                        Files[i].fs = 0;
        }
}
#endif

//==============================================================================
/**
 * @brief Flush disk access window
 *
 * @param[in] *fs       File system object
 *
 * @retval FR_OK        success
 */
//==============================================================================
static FRESULT sync_window(FATFS *fs)
{
        uint32_t wsect;
        uint nf;

        /* Write back the sector if it is dirty */
        if (fs->wflag) {
                wsect = fs->winsect;

                /* Current sector number */
                if (_libfat_disk_write(fs->srcfile, fs->win, wsect, 1) != RES_OK)
                        return FR_DISK_ERR;

                fs->wflag = 0;

                /* In FAT area? */
                if (wsect >= fs->fatbase && wsect < (fs->fatbase + fs->fsize)) {
                        /* Reflect the change to all FAT copies */
                        for (nf = fs->n_fats; nf >= 2; nf--) {
                                wsect += fs->fsize;
                                _libfat_disk_write(fs->srcfile, fs->win, wsect, 1);
                        }
                }
        }

        return FR_OK;
}

//==============================================================================
/**
 * @brief Move disk access window
 *
 * @param[in] *fs       File system object
 * @param[in]  sector   Sector number to make appearance in the fs->win[]
 *
 * @retval FR_OK        success
 */
//==============================================================================
static FRESULT move_window(FATFS *fs, uint32_t sector)
{
        /* Changed current window */
        if (sector != fs->winsect) {
                if (sync_window(fs) != FR_OK)
                        return FR_DISK_ERR;

                if (_libfat_disk_read(fs->srcfile, fs->win, sector, 1) != RES_OK)
                        return FR_DISK_ERR;

                fs->winsect = sector;
        }

        return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Synchronize file system and storage device                             */
/*-----------------------------------------------------------------------*/
static FRESULT sync_fs( /* FR_OK: successful, FR_DISK_ERR: failed */
FATFS *fs /* File system object */
)
{
        FRESULT res;

        res = sync_window(fs);
        if (res == FR_OK) {
                /* Update FSInfo sector if needed */
                if (fs->fs_type == LIBFAT_FS_FAT32 && fs->fsi_flag) {
                        fs->winsect = 0;
                        /* Create FSInfo structure */
                        memset(fs->win, 0, 512);
                        STORE_UINT16(fs->win+BS_55AA, 0xAA55);
                        STORE_UINT32(fs->win+FSI_LeadSig, 0x41615252);
                        STORE_UINT32(fs->win+FSI_StrucSig, 0x61417272);
                        STORE_UINT32(fs->win+FSI_Free_Count, fs->free_clust);
                        STORE_UINT32(fs->win+FSI_Nxt_Free, fs->last_clust);
                        /* Write it into the FSInfo sector */
                        _libfat_disk_write(fs->srcfile, fs->win, fs->fsi_sector, 1);
                        fs->fsi_flag = 0;
                }
                /* Make sure that no pending write process in the physical drive */
                if (_libfat_disk_ioctl(fs->srcfile, CTRL_SYNC, 0) != RES_OK)
                        res = FR_DISK_ERR;
        }

        return res;
}


/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/
uint32_t clust2sect (        /* !=0: Sector number, 0: Failed - invalid cluster# */
        FATFS *fs,                /* File system object */
        uint32_t clst                /* Cluster# to be converted */
)
{
        clst -= 2;
        if (clst >= (fs->n_fatent - 2)) return 0;                /* Invalid cluster# */
        return clst * fs->csize + fs->database;
}




/*-----------------------------------------------------------------------*/
/* FAT access - Read value of a FAT entry                                */
/*-----------------------------------------------------------------------*/


uint32_t get_fat (        /* 0xFFFFFFFF:Disk error, 1:Internal error, Else:Cluster status */
        FATFS *fs,        /* File system object */
        uint32_t clst        /* Cluster# to get the link information */
)
{
        uint wc, bc;
        uint8_t *p;


        if (clst < 2 || clst >= fs->n_fatent)        /* Check range */
                return 1;

        switch (fs->fs_type) {
        case LIBFAT_FS_FAT12 :
                bc = (uint)clst; bc += bc / 2;
                if (move_window(fs, fs->fatbase + (bc / SS(fs)))) break;
                wc = fs->win[bc % SS(fs)]; bc++;
                if (move_window(fs, fs->fatbase + (bc / SS(fs)))) break;
                wc |= fs->win[bc % SS(fs)] << 8;
                return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);

        case LIBFAT_FS_FAT16 :
                if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)))) break;
                p = &fs->win[clst * 2 % SS(fs)];
                return LOAD_UINT16(p);

        case LIBFAT_FS_FAT32 :
                if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)))) break;
                p = &fs->win[clst * 4 % SS(fs)];
                return LOAD_UINT32(p) & 0x0FFFFFFF;
        }

        return 0xFFFFFFFF;        /* An error occurred at the disk I/O layer */
}




/*-----------------------------------------------------------------------*/
/* FAT access - Change value of a FAT entry                              */
/*-----------------------------------------------------------------------*/
FRESULT put_fat (
        FATFS *fs,        /* File system object */
        uint32_t clst,        /* Cluster# to be changed in range of 2 to fs->n_fatent - 1 */
        uint32_t val        /* New value to mark the cluster */
)
{
        uint bc;
        uint8_t *p;
        FRESULT res;


        if (clst < 2 || clst >= fs->n_fatent) {        /* Check range */
                res = FR_INT_ERR;

        } else {
                switch (fs->fs_type) {
                case LIBFAT_FS_FAT12 :
                        bc = (uint)clst; bc += bc / 2;
                        res = move_window(fs, fs->fatbase + (bc / SS(fs)));
                        if (res != FR_OK) break;
                        p = &fs->win[bc % SS(fs)];
                        *p = (clst & 1) ? ((*p & 0x0F) | ((uint8_t)val << 4)) : (uint8_t)val;
                        bc++;
                        fs->wflag = 1;
                        res = move_window(fs, fs->fatbase + (bc / SS(fs)));
                        if (res != FR_OK) break;
                        p = &fs->win[bc % SS(fs)];
                        *p = (clst & 1) ? (uint8_t)(val >> 4) : ((*p & 0xF0) | ((uint8_t)(val >> 8) & 0x0F));
                        break;

                case LIBFAT_FS_FAT16 :
                        res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
                        if (res != FR_OK) break;
                        p = &fs->win[clst * 2 % SS(fs)];
                        STORE_UINT16(p, (uint16_t)val);
                        break;

                case LIBFAT_FS_FAT32 :
                        res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
                        if (res != FR_OK) break;
                        p = &fs->win[clst * 4 % SS(fs)];
                        val |= LOAD_UINT32(p) & 0xF0000000;
                        STORE_UINT32(p, val);
                        break;

                default :
                        res = FR_INT_ERR;
                }
                fs->wflag = 1;
        }

        return res;
}



/*-----------------------------------------------------------------------*/
/* FAT handling - Remove a cluster chain                                 */
/*-----------------------------------------------------------------------*/
static
FRESULT remove_chain (
        FATFS *fs,                        /* File system object */
        uint32_t clst                        /* Cluster# to remove a chain from */
)
{
        FRESULT res;
        uint32_t nxt;
#if _LIBFAT_USE_ERASE
        uint32_t scl = clst, ecl = clst, rt[2];
#endif

        if (clst < 2 || clst >= fs->n_fatent) {        /* Check range */
                res = FR_INT_ERR;

        } else {
                res = FR_OK;
                while (clst < fs->n_fatent) {                        /* Not a last link? */
                        nxt = get_fat(fs, clst);                        /* Get cluster status */
                        if (nxt == 0) break;                                /* Empty cluster? */
                        if (nxt == 1) { res = FR_INT_ERR; break; }        /* Internal error? */
                        if (nxt == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }        /* Disk error? */
                        res = put_fat(fs, clst, 0);                        /* Mark the cluster "empty" */
                        if (res != FR_OK) break;
                        if (fs->free_clust != 0xFFFFFFFF) {        /* Update FSInfo */
                                fs->free_clust++;
                                fs->fsi_flag = 1;
                        }
#if _LIBFAT_USE_ERASE
                        if (ecl + 1 == nxt) {        /* Is next cluster contiguous? */
                                ecl = nxt;
                        } else {                                /* End of contiguous clusters */
                                rt[0] = clust2sect(fs, scl);                                        /* Start sector */
                                rt[1] = clust2sect(fs, ecl) + fs->csize - 1;        /* End sector */
                                _libfat_disk_ioctl(fs->drv, CTRL_ERASE_SECTOR, rt);                /* Erase the block */
                                scl = ecl = nxt;
                        }
#endif
                        clst = nxt;        /* Next cluster */
                }
        }

        return res;
}



/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch or Create a cluster chain                      */
/*-----------------------------------------------------------------------*/
static
uint32_t create_chain (        /* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error, >=2:New cluster# */
        FATFS *fs,                        /* File system object */
        uint32_t clst                        /* Cluster# to stretch. 0 means create a new chain. */
)
{
        uint32_t cs, ncl, scl;
        FRESULT res;


        if (clst == 0) {                /* Create a new chain */
                scl = fs->last_clust;                        /* Get suggested start point */
                if (!scl || scl >= fs->n_fatent) scl = 1;
        }
        else {                                        /* Stretch the current chain */
                cs = get_fat(fs, clst);                        /* Check the cluster status */
                if (cs < 2) return 1;                        /* It is an invalid cluster */
                if (cs < fs->n_fatent) return cs;        /* It is already followed by next cluster */
                scl = clst;
        }

        ncl = scl;                                /* Start cluster */
        for (;;) {
                ncl++;                                                        /* Next cluster */
                if (ncl >= fs->n_fatent) {                /* Wrap around */
                        ncl = 2;
                        if (ncl > scl) return 0;        /* No free cluster */
                }
                cs = get_fat(fs, ncl);                        /* Get the cluster status */
                if (cs == 0) break;                                /* Found a free cluster */
                if (cs == 0xFFFFFFFF || cs == 1)/* An error occurred */
                        return cs;
                if (ncl == scl) return 0;                /* No free cluster */
        }

        res = put_fat(fs, ncl, 0x0FFFFFFF);        /* Mark the new cluster "last link" */
        if (res == FR_OK && clst != 0) {
                res = put_fat(fs, clst, ncl);        /* Link it to the previous one if needed */
        }
        if (res == FR_OK) {
                fs->last_clust = ncl;                        /* Update FSINFO */
                if (fs->free_clust != 0xFFFFFFFF) {
                        fs->free_clust--;
                        fs->fsi_flag = 1;
                }
        } else {
                ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1;
        }

        return ncl;                /* Return new cluster number or error code */
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Set directory index                              */
/*-----------------------------------------------------------------------*/

static
FRESULT dir_sdi (
        FATDIR *dj,                /* Pointer to directory object */
        uint16_t idx                /* Index of directory table */
)
{
        uint32_t clst;
        uint16_t ic;


        dj->index = idx;
        clst = dj->sclust;
        if (clst == 1 || clst >= dj->fs->n_fatent)        /* Check start cluster range */
                return FR_INT_ERR;
        if (!clst && dj->fs->fs_type == LIBFAT_FS_FAT32)        /* Replace cluster# 0 with root cluster# if in FAT32 */
                clst = dj->fs->dirbase;

        if (clst == 0) {        /* Static table (root-dir in FAT12/16) */
                dj->clust = clst;
                if (idx >= dj->fs->n_rootdir)                /* Index is out of range */
                        return FR_INT_ERR;
                dj->sect = dj->fs->dirbase + idx / (SS(dj->fs) / SZ_DIR);        /* Sector# */
        }
        else {                                /* Dynamic table (sub-dirs or root-dir in FAT32) */
                ic = SS(dj->fs) / SZ_DIR * dj->fs->csize;        /* Entries per cluster */
                while (idx >= ic) {        /* Follow cluster chain */
                        clst = get_fat(dj->fs, clst);                                /* Get next cluster */
                        if (clst == 0xFFFFFFFF) return FR_DISK_ERR;        /* Disk error */
                        if (clst < 2 || clst >= dj->fs->n_fatent)        /* Reached to end of table or int error */
                                return FR_INT_ERR;
                        idx -= ic;
                }
                dj->clust = clst;
                dj->sect = clust2sect(dj->fs, clst) + idx / (SS(dj->fs) / SZ_DIR);        /* Sector# */
        }

        dj->dir = dj->fs->win + (idx % (SS(dj->fs) / SZ_DIR)) * SZ_DIR;        /* Ptr to the entry in the sector */

        return FR_OK;        /* Seek succeeded */
}




/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory table index next                  */
/*-----------------------------------------------------------------------*/

static
FRESULT dir_next (        /* FR_OK:Succeeded, FR_NO_FILE:End of table, FR_DENIED:Could not stretch */
        FATDIR *dj,                /* Pointer to the directory object */
        int stretch                /* 0: Do not stretch table, 1: Stretch table if needed */
)
{
        uint32_t clst;
        uint16_t i;


//        stretch = stretch;                /* To suppress warning on read-only cfg. */
        i = dj->index + 1;
        if (!i || !dj->sect)        /* Report EOT when index has reached 65535 */
                return FR_NO_FILE;

        if (!(i % (SS(dj->fs) / SZ_DIR))) {        /* Sector changed? */
                dj->sect++;                                        /* Next sector */

                if (dj->clust == 0) {        /* Static table */
                        if (i >= dj->fs->n_rootdir)        /* Report EOT when end of table */
                                return FR_NO_FILE;
                }
                else {                                        /* Dynamic table */
                        if (((i / (SS(dj->fs) / SZ_DIR)) & (dj->fs->csize - 1)) == 0) {        /* Cluster changed? */
                                clst = get_fat(dj->fs, dj->clust);                                /* Get next cluster */
                                if (clst <= 1) return FR_INT_ERR;
                                if (clst == 0xFFFFFFFF) return FR_DISK_ERR;
                                if (clst >= dj->fs->n_fatent) {                                        /* When it reached end of dynamic table */
                                        uint8_t c;
                                        if (!stretch) return FR_NO_FILE;                        /* When do not stretch, report EOT */
                                        clst = create_chain(dj->fs, dj->clust);                /* Stretch cluster chain */
                                        if (clst == 0) return FR_DENIED;                        /* No free cluster */
                                        if (clst == 1) return FR_INT_ERR;
                                        if (clst == 0xFFFFFFFF) return FR_DISK_ERR;
                                        /* Clean-up stretched table */
                                        if (sync_window(dj->fs)) return FR_DISK_ERR;        /* Flush active window */
                                        memset(dj->fs->win, 0, SS(dj->fs));                        /* Clear window buffer */
                                        dj->fs->winsect = clust2sect(dj->fs, clst);        /* Cluster start sector */
                                        for (c = 0; c < dj->fs->csize; c++) {                /* Fill the new cluster with 0 */
                                                dj->fs->wflag = 1;
                                                if (sync_window(dj->fs)) return FR_DISK_ERR;
                                                dj->fs->winsect++;
                                        }
                                        dj->fs->winsect -= c;                                                /* Rewind window address */
                                }
                                dj->clust = clst;                                /* Initialize data for new cluster */
                                dj->sect = clust2sect(dj->fs, clst);
                        }
                }
        }

        dj->index = i;
        dj->dir = dj->fs->win + (i % (SS(dj->fs) / SZ_DIR)) * SZ_DIR;

        return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve directory entry                          */
/*-----------------------------------------------------------------------*/

static
FRESULT dir_alloc (
        FATDIR* dj,        /* Pointer to the directory object */
        uint nent        /* Number of contiguous entries to allocate (1-21) */
)
{
        FRESULT res;
        uint n;


        res = dir_sdi(dj, 0);
        if (res == FR_OK) {
                n = 0;
                do {
                        res = move_window(dj->fs, dj->sect);
                        if (res != FR_OK) break;
                        if (dj->dir[0] == DDE || dj->dir[0] == 0) {        /* Is it a blank entry? */
                                if (++n == nent) break;        /* A block of contiguous entry is found */
                        } else {
                                n = 0;                                        /* Not a blank entry. Restart to search */
                        }
                        res = dir_next(dj, 1);                /* Next entry with table stretch enabled */
                } while (res == FR_OK);
        }
        if (res == FR_NO_FILE) res = FR_DENIED;
        return res;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Load/Store start cluster number                  */
/*-----------------------------------------------------------------------*/

static
uint32_t ld_clust (
        FATFS *fs,        /* Pointer to the fs object */
        uint8_t *dir        /* Pointer to the directory entry */
)
{
        uint32_t cl;

        cl = LOAD_UINT16(dir+DIR_FstClusLO);
        if (fs->fs_type == LIBFAT_FS_FAT32)
                cl |= (uint32_t)LOAD_UINT16(dir+DIR_FstClusHI) << 16;

        return cl;
}


static
void st_clust (
        uint8_t *dir,        /* Pointer to the directory entry */
        uint32_t cl        /* Value to be set */
)
{
        STORE_UINT16(dir+DIR_FstClusLO, cl);
        STORE_UINT16(dir+DIR_FstClusHI, cl >> 16);
}




/*-----------------------------------------------------------------------*/
/* LFN handling - Test/Pick/Fit an LFN segment from/to directory entry   */
/*-----------------------------------------------------------------------*/
#if _LIBFAT_USE_LFN
static
const uint8_t LfnOfs[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};        /* Offset of LFN chars in the directory entry */


static
int cmp_lfn (                        /* 1:Matched, 0:Not matched */
        wchar_t *lfnbuf,                /* Pointer to the LFN to be compared */
        uint8_t *dir                        /* Pointer to the directory entry containing a part of LFN */
)
{
        uint i, s;
        wchar_t wc, uc;


        i = ((dir[LDIR_Ord] & ~LLE) - 1) * 13;        /* Get offset in the LFN buffer */
        s = 0; wc = 1;
        do {
                uc = LOAD_UINT16(dir+LfnOfs[s]);        /* Pick an LFN character from the entry */
                if (wc) {        /* Last char has not been processed */
                        wc = _libfat_wtoupper(uc);                /* Convert it to upper case */
                        if (i >= _LIBFAT_MAX_LFN || wc != _libfat_wtoupper(lfnbuf[i++]))        /* Compare it */
                                return 0;                                /* Not matched */
                } else {
                        if (uc != 0xFFFF) return 0;        /* Check filler */
                }
        } while (++s < 13);                                /* Repeat until all chars in the entry are checked */

        if ((dir[LDIR_Ord] & LLE) && wc && lfnbuf[i])        /* Last segment matched but different length */
                return 0;

        return 1;                                                /* The part of LFN matched */
}



static
int pick_lfn (                        /* 1:Succeeded, 0:Buffer overflow */
        wchar_t *lfnbuf,                /* Pointer to the Unicode-LFN buffer */
        uint8_t *dir                        /* Pointer to the directory entry */
)
{
        uint i, s;
        wchar_t wc, uc;


        i = ((dir[LDIR_Ord] & 0x3F) - 1) * 13;        /* Offset in the LFN buffer */

        s = 0; wc = 1;
        do {
                uc = LOAD_UINT16(dir+LfnOfs[s]);                /* Pick an LFN character from the entry */
                if (wc) {        /* Last char has not been processed */
                        if (i >= _LIBFAT_MAX_LFN) return 0;        /* Buffer overflow? */
                        lfnbuf[i++] = wc = uc;                        /* Store it */
                } else {
                        if (uc != 0xFFFF) return 0;                /* Check filler */
                }
        } while (++s < 13);                                                /* Read all character in the entry */

        if (dir[LDIR_Ord] & LLE) {                                /* Put terminator if it is the last LFN part */
                if (i >= _LIBFAT_MAX_LFN) return 0;                /* Buffer overflow? */
                lfnbuf[i] = 0;
        }

        return 1;
}


static
void fit_lfn (
        const wchar_t *lfnbuf,        /* Pointer to the LFN buffer */
        uint8_t *dir,                                /* Pointer to the directory entry */
        uint8_t ord,                                /* LFN order (1-20) */
        uint8_t sum                                /* SFN sum */
)
{
        uint i, s;
        wchar_t wc;


        dir[LDIR_Chksum] = sum;                        /* Set check sum */
        dir[LDIR_Attr] = LIBFAT_AM_LFN;                /* Set attribute. LFN entry */
        dir[LDIR_Type] = 0;
        STORE_UINT16(dir+LDIR_FstClusLO, 0);

        i = (ord - 1) * 13;                                /* Get offset in the LFN buffer */
        s = wc = 0;
        do {
                if (wc != 0xFFFF) wc = lfnbuf[i++];        /* Get an effective char */
                STORE_UINT16(dir+LfnOfs[s], wc);        /* Put it */
                if (!wc) wc = 0xFFFF;                /* Padding chars following last char */
        } while (++s < 13);
        if (wc == 0xFFFF || !lfnbuf[i]) ord |= LLE;        /* Bottom LFN part is the start of LFN sequence */
        dir[LDIR_Ord] = ord;                        /* Set the LFN order */
}

#endif


/*-----------------------------------------------------------------------*/
/* Create numbered name                                                  */
/*-----------------------------------------------------------------------*/
#if _LIBFAT_USE_LFN
void gen_numname (
        uint8_t *dst,                        /* Pointer to generated SFN */
        const uint8_t *src,        /* Pointer to source SFN to be modified */
        const wchar_t *lfn,        /* Pointer to LFN */
        uint16_t seq                        /* Sequence number */
)
{
        uint8_t ns[8], c;
        uint i, j;


        memcpy(dst, src, 11);

        if (seq > 5) {        /* On many collisions, generate a hash number instead of sequential number */
                do seq = (seq >> 1) + (seq << 15) + (uint16_t)*lfn++; while (*lfn);
        }

        /* itoa (hexdecimal) */
        i = 7;
        do {
                c = (seq % 16) + '0';
                if (c > '9') c += 7;
                ns[i--] = c;
                seq /= 16;
        } while (seq);
        ns[i] = '~';

        /* Append the number */
        for (j = 0; j < i && dst[j] != ' '; j++) {
                if (IsDBCS1(dst[j])) {
                        if (j == i - 1) break;
                        j++;
                }
        }
        do {
                dst[j++] = (i < 8) ? ns[i++] : ' ';
        } while (j < 8);
}
#endif




/*-----------------------------------------------------------------------*/
/* Calculate sum of an SFN                                               */
/*-----------------------------------------------------------------------*/
#if _LIBFAT_USE_LFN
static
uint8_t sum_sfn (
        const uint8_t *dir                /* Ptr to directory entry */
)
{
        uint8_t sum = 0;
        uint n = 11;

        do sum = (sum >> 1) + (sum << 7) + *dir++; while (--n);
        return sum;
}
#endif




/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

static
FRESULT dir_find (
        FATDIR *dj                        /* Pointer to the directory object linked to the file name */
)
{
        FRESULT res;
        uint8_t c, *dir;
#if _LIBFAT_USE_LFN
        uint8_t a, ord, sum;
#endif

        res = dir_sdi(dj, 0);                        /* Rewind directory object */
        if (res != FR_OK) return res;

#if _LIBFAT_USE_LFN
        ord = sum = 0xFF;
#endif
        do {
                res = move_window(dj->fs, dj->sect);
                if (res != FR_OK) break;
                dir = dj->dir;                                        /* Ptr to the directory entry of current index */
                c = dir[DIR_Name];
                if (c == 0) { res = FR_NO_FILE; break; }        /* Reached to end of table */
#if _LIBFAT_USE_LFN        /* LFN configuration */
                a = dir[DIR_Attr] & LIBFAT_AM_MASK;
                if (c == DDE || ((a & LIBFAT_AM_VOL) && a != LIBFAT_AM_LFN)) {        /* An entry without valid data */
                        ord = 0xFF;
                } else {
                        if (a == LIBFAT_AM_LFN) {                        /* An LFN entry is found */
                                if (dj->lfn) {
                                        if (c & LLE) {                /* Is it start of LFN sequence? */
                                                sum = dir[LDIR_Chksum];
                                                c &= ~LLE; ord = c;        /* LFN start order */
                                                dj->lfn_idx = dj->index;
                                        }
                                        /* Check validity of the LFN entry and compare it with given name */
                                        ord = (c == ord && sum == dir[LDIR_Chksum] && cmp_lfn(dj->lfn, dir)) ? ord - 1 : 0xFF;
                                }
                        } else {                                        /* An SFN entry is found */
                                if (!ord && sum == sum_sfn(dir)) break;        /* LFN matched? */
                                ord = 0xFF; dj->lfn_idx = 0xFFFF;        /* Reset LFN sequence */
                                if (!(dj->fn[NS] & NS_LOSS) && !memcmp(dir, dj->fn, 11)) break;        /* SFN matched? */
                        }
                }
#else                /* Non LFN configuration */
                if (!(dir[DIR_Attr] & LIBFAT_AM_VOL) && !memcmp(dir, dj->fn, 11)) /* Is it a valid entry? */
                        break;
#endif
                res = dir_next(dj, 0);                /* Next entry */
        } while (res == FR_OK);

        return res;
}




/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/
static
FRESULT dir_read (
        FATDIR *dj,                /* Pointer to the directory object */
        int vol                        /* Filtered by 0:file/dir or 1:volume label */
)
{
        FRESULT res;
        uint8_t a, c, *dir;
#if _LIBFAT_USE_LFN
        uint8_t ord = 0xFF, sum = 0xFF;
#endif

        res = FR_NO_FILE;
        while (dj->sect) {
                res = move_window(dj->fs, dj->sect);
                if (res != FR_OK) break;
                dir = dj->dir;                                        /* Ptr to the directory entry of current index */
                c = dir[DIR_Name];
                if (c == 0) { res = FR_NO_FILE; break; }        /* Reached to end of table */
                a = dir[DIR_Attr] & LIBFAT_AM_MASK;
#if _LIBFAT_USE_LFN        /* LFN configuration */
                if (c == DDE || (/*!_FS_RPATH && */c == '.') || (a == LIBFAT_AM_VOL) != vol) {        /* An entry without valid data */
                        ord = 0xFF;
                } else {
                        if (a == LIBFAT_AM_LFN) {                        /* An LFN entry is found */
                                if (c & LLE) {                        /* Is it start of LFN sequence? */
                                        sum = dir[LDIR_Chksum];
                                        c &= ~LLE; ord = c;
                                        dj->lfn_idx = dj->index;
                                }
                                /* Check LFN validity and capture it */
                                ord = (c == ord && sum == dir[LDIR_Chksum] && pick_lfn(dj->lfn, dir)) ? ord - 1 : 0xFF;
                        } else {                                        /* An SFN entry is found */
                                if (ord || sum != sum_sfn(dir))        /* Is there a valid LFN? */
                                        dj->lfn_idx = 0xFFFF;                /* It has no LFN. */
                                break;
                        }
                }
#else                /* Non LFN configuration */
                if (c != DDE && (/*_FS_RPATH || */c != '.') && a != LIBFAT_AM_LFN && (a == LIBFAT_AM_VOL) == vol)        /* Is it a valid entry? */
                        break;
#endif
                res = dir_next(dj, 0);                                /* Next entry */
                if (res != FR_OK) break;
        }

        if (res != FR_OK) dj->sect = 0;

        return res;
}


/*-----------------------------------------------------------------------*/
/* Register an object to the directory                                   */
/*-----------------------------------------------------------------------*/
static
FRESULT dir_register (        /* FR_OK:Successful, FR_DENIED:No free entry or too many SFN collision, FR_DISK_ERR:Disk error */
        FATDIR *dj                                /* Target directory with object name to be created */
)
{
        FRESULT res;
#if _LIBFAT_USE_LFN        /* LFN configuration */
        uint16_t n, ne;
        uint8_t sn[12], *fn, sum;
        wchar_t *lfn;


        fn = dj->fn; lfn = dj->lfn;
        memcpy(sn, fn, 12);

//        if (_FS_RPATH && (sn[NS] & NS_DOT))                /* Cannot create dot entry */
//                return FR_INVALID_NAME;

        if (sn[NS] & NS_LOSS) {                        /* When LFN is out of 8.3 format, generate a numbered name */
                fn[NS] = 0; dj->lfn = 0;                        /* Find only SFN */
                for (n = 1; n < 100; n++) {
                        gen_numname(fn, sn, lfn, n);        /* Generate a numbered name */
                        res = dir_find(dj);                                /* Check if the name collides with existing SFN */
                        if (res != FR_OK) break;
                }
                if (n == 100) return FR_DENIED;                /* Abort if too many collisions */
                if (res != FR_NO_FILE) return res;        /* Abort if the result is other than 'not collided' */
                fn[NS] = sn[NS]; dj->lfn = lfn;
        }

        if (sn[NS] & NS_LFN) {                        /* When LFN is to be created, allocate entries for an SFN + LFNs. */
                for (n = 0; lfn[n]; n++) ;
                ne = (n + 25) / 13;
        } else {                                                /* Otherwise allocate an entry for an SFN  */
                ne = 1;
        }
        res = dir_alloc(dj, ne);                /* Allocate entries */

        if (res == FR_OK && --ne) {                /* Set LFN entry if needed */
                res = dir_sdi(dj, (uint16_t)(dj->index - ne));
                if (res == FR_OK) {
                        sum = sum_sfn(dj->fn);        /* Sum value of the SFN tied to the LFN */
                        do {                                        /* Store LFN entries in bottom first */
                                res = move_window(dj->fs, dj->sect);
                                if (res != FR_OK) break;
                                fit_lfn(dj->lfn, dj->dir, (uint8_t)ne, sum);
                                dj->fs->wflag = 1;
                                res = dir_next(dj, 0);        /* Next entry */
                        } while (res == FR_OK && --ne);
                }
        }
#else        /* Non LFN configuration */
        res = dir_alloc(dj, 1);                /* Allocate an entry for SFN */
#endif

        if (res == FR_OK) {                                /* Set SFN entry */
                res = move_window(dj->fs, dj->sect);
                if (res == FR_OK) {
                        memset(dj->dir, 0, SZ_DIR);        /* Clean the entry */
                        memcpy(dj->dir, dj->fn, 11);        /* Put SFN */
#if _LIBFAT_USE_LFN
                        dj->dir[DIR_NTres] = *(dj->fn+NS) & (NS_BODY | NS_EXT);        /* Put NT flag */
#endif
                        dj->fs->wflag = 1;
                }
        }

        return res;
}



/*-----------------------------------------------------------------------*/
/* Remove an object from the directory                                   */
/*-----------------------------------------------------------------------*/
static
FRESULT dir_remove (        /* FR_OK: Successful, FR_DISK_ERR: A disk error */
        FATDIR *dj                                /* Directory object pointing the entry to be removed */
)
{
        FRESULT res;
#if _LIBFAT_USE_LFN        /* LFN configuration */
        uint16_t i;

        i = dj->index;        /* SFN index */
        res = dir_sdi(dj, (uint16_t)((dj->lfn_idx == 0xFFFF) ? i : dj->lfn_idx));        /* Goto the SFN or top of the LFN entries */
        if (res == FR_OK) {
                do {
                        res = move_window(dj->fs, dj->sect);
                        if (res != FR_OK) break;
                        *dj->dir = DDE;                        /* Mark the entry "deleted" */
                        dj->fs->wflag = 1;
                        if (dj->index >= i) break;        /* When reached SFN, all entries of the object has been deleted. */
                        res = dir_next(dj, 0);                /* Next entry */
                } while (res == FR_OK);
                if (res == FR_NO_FILE) res = FR_INT_ERR;
        }

#else                        /* Non LFN configuration */
        res = dir_sdi(dj, dj->index);
        if (res == FR_OK) {
                res = move_window(dj->fs, dj->sect);
                if (res == FR_OK) {
                        *dj->dir = DDE;                        /* Mark the entry "deleted" */
                        dj->fs->wflag = 1;
                }
        }
#endif

        return res;
}


/*-----------------------------------------------------------------------*/
/* Pick a segment and create the object name in directory form           */
/*-----------------------------------------------------------------------*/

static
FRESULT create_name (
        FATDIR *dj,                        /* Pointer to the directory object */
        const TCHAR **path        /* Pointer to pointer to the segment in the path string */
)
{
#if _LIBFAT_USE_LFN        /* LFN configuration */
        uint8_t b, cf;
        wchar_t w, *lfn;
        uint i, ni, si, di;
        const TCHAR *p;

        /* Create LFN in Unicode */
        for (p = *path; *p == '/' || *p == '\\'; p++) ;        /* Strip duplicated separator */
        lfn = dj->lfn;
        si = di = 0;
        for (;;) {
                w = p[si++];                                        /* Get a character */
                if (w < ' ' || w == '/' || w == '\\') break;        /* Break on end of segment */
                if (di >= _LIBFAT_MAX_LFN)                                /* Reject too long name */
                        return FR_INVALID_NAME;
#if !_LIBFAT_LFN_UNICODE
                w &= 0xFF;
                if (IsDBCS1(w)) {                                /* Check if it is a DBC 1st byte (always false on SBCS cfg) */
                        b = (uint8_t)p[si++];                        /* Get 2nd byte */
                        if (!IsDBCS2(b))
                                return FR_INVALID_NAME;        /* Reject invalid sequence */
                        w = (w << 8) + b;                        /* Create a DBC */
                }
                w = _libfat_convert(w, 1);                        /* Convert ANSI/OEM to Unicode */
                if (!w) return FR_INVALID_NAME;        /* Reject invalid code */
#endif
                if (w < 0x80 && strchr("\"*:<>\?|\x7F", w)) /* Reject illegal chars for LFN */
                        return FR_INVALID_NAME;
                lfn[di++] = w;                                        /* Store the Unicode char */
        }
        *path = &p[si];                                                /* Return pointer to the next segment */
        cf = (w < ' ') ? NS_LAST : 0;                /* Set last segment flag if end of path */

        while (di) {                                                /* Strip trailing spaces and dots */
                w = lfn[di-1];
                if (w != ' ' && w != '.') break;
                di--;
        }
        if (!di) return FR_INVALID_NAME;        /* Reject nul string */

        lfn[di] = 0;                                                /* LFN is created */

        /* Create SFN in directory form */
        memset(dj->fn, ' ', 11);
        for (si = 0; lfn[si] == ' ' || lfn[si] == '.'; si++) ;        /* Strip leading spaces and dots */
        if (si) cf |= NS_LOSS | NS_LFN;
        while (di && lfn[di - 1] != '.') di--;        /* Find extension (di<=si: no extension) */

        b = i = 0; ni = 8;
        for (;;) {
                w = lfn[si++];                                        /* Get an LFN char */
                if (!w) break;                                        /* Break on end of the LFN */
                if (w == ' ' || (w == '.' && si != di)) {        /* Remove spaces and dots */
                        cf |= NS_LOSS | NS_LFN; continue;
                }

                if (i >= ni || si == di) {                /* Extension or end of SFN */
                        if (ni == 11) {                                /* Long extension */
                                cf |= NS_LOSS | NS_LFN; break;
                        }
                        if (si != di) cf |= NS_LOSS | NS_LFN;        /* Out of 8.3 format */
                        if (si > di) break;                        /* No extension */
                        si = di; i = 8; ni = 11;        /* Enter extension section */
                        b <<= 2; continue;
                }

                if (w >= 0x80) {                                /* Non ASCII char */
#ifdef _EXCVT
                        w = _libfat_convert(w, 0);                /* Unicode -> OEM code */
                        if (w) w = ExCvt[w - 0x80];        /* Convert extended char to upper (SBCS) */
#else
                        w = _libfat_convert(_libfat_wtoupper(w), 0);        /* Upper converted Unicode -> OEM code */
#endif
                        cf |= NS_LFN;                                /* Force create LFN entry */
                }

                if (_DF1S && w >= 0x100) {                /* Double byte char (always false on SBCS cfg) */
                        if (i >= ni - 1) {
                                cf |= NS_LOSS | NS_LFN; i = ni; continue;
                        }
                        dj->fn[i++] = (uint8_t)(w >> 8);
                } else {                                                /* Single byte char */
                        if (!w || strchr("+,;=[]", w)) {        /* Replace illegal chars for SFN */
                                w = '_'; cf |= NS_LOSS | NS_LFN;/* Lossy conversion */
                        } else {
                                if (IsUpper(w)) {                /* ASCII large capital */
                                        b |= 2;
                                } else {
                                        if (IsLower(w)) {        /* ASCII small capital */
                                                b |= 1; w -= 0x20;
                                        }
                                }
                        }
                }
                dj->fn[i++] = (uint8_t)w;
        }

        if (dj->fn[0] == DDE) dj->fn[0] = NDDE;        /* If the first char collides with deleted mark, replace it with 0x05 */

        if (ni == 8) b <<= 2;
        if ((b & 0x0C) == 0x0C || (b & 0x03) == 0x03)        /* Create LFN entry when there are composite capitals */
                cf |= NS_LFN;
        if (!(cf & NS_LFN)) {                                                /* When LFN is in 8.3 format without extended char, NT flags are created */
                if ((b & 0x03) == 0x01) cf |= NS_EXT;        /* NT flag (Extension has only small capital) */
                if ((b & 0x0C) == 0x04) cf |= NS_BODY;        /* NT flag (Filename has only small capital) */
        }

        dj->fn[NS] = cf;        /* SFN is created */

        return FR_OK;


#else        /* Non-LFN configuration */
        uint8_t b, c, d, *sfn;
        uint ni, si, i;
        const char *p;

        /* Create file name in directory form */
        for (p = *path; *p == '/' || *p == '\\'; p++) ;        /* Strip duplicated separator */
        sfn = dj->fn;
        memset(sfn, ' ', 11);
        si = i = b = 0; ni = 8;
#if _FS_RPATH
        if (p[si] == '.') { /* Is this a dot entry? */
                for (;;) {
                        c = (uint8_t)p[si++];
                        if (c != '.' || si >= 3) break;
                        sfn[i++] = c;
                }
                if (c != '/' && c != '\\' && c > ' ') return FR_INVALID_NAME;
                *path = &p[si];                                                                        /* Return pointer to the next segment */
                sfn[NS] = (c <= ' ') ? NS_LAST | NS_DOT : NS_DOT;        /* Set last segment flag if end of path */
                return FR_OK;
        }
#endif
        for (;;) {
                c = (uint8_t)p[si++];
                if (c <= ' ' || c == '/' || c == '\\') break;        /* Break on end of segment */
                if (c == '.' || i >= ni) {
                        if (ni != 8 || c != '.') return FR_INVALID_NAME;
                        i = 8; ni = 11;
                        b <<= 2; continue;
                }
                if (c >= 0x80) {                                /* Extended char? */
                        b |= 3;                                                /* Eliminate NT flag */
#ifdef _EXCVT
                        c = ExCvt[c - 0x80];                /* To upper extended chars (SBCS cfg) */
#else
#if !_DF1S
                        return FR_INVALID_NAME;                /* Reject extended chars (ASCII cfg) */
#endif
#endif
                }
                if (IsDBCS1(c)) {                                /* Check if it is a DBC 1st byte (always false on SBCS cfg) */
                        d = (uint8_t)p[si++];                        /* Get 2nd byte */
                        if (!IsDBCS2(d) || i >= ni - 1)        /* Reject invalid DBC */
                                return FR_INVALID_NAME;
                        sfn[i++] = c;
                        sfn[i++] = d;
                } else {                                                /* Single byte code */
                        if (strchr("\"*+,:;<=>\?[]|\x7F", c))        /* Reject illegal chrs for SFN */
                                return FR_INVALID_NAME;
                        if (IsUpper(c)) {                        /* ASCII large capital? */
                                b |= 2;
                        } else {
                                if (IsLower(c)) {                /* ASCII small capital? */
                                        b |= 1; c -= 0x20;
                                }
                        }
                        sfn[i++] = c;
                }
        }
        *path = &p[si];                                                /* Return pointer to the next segment */
        c = (c <= ' ') ? NS_LAST : 0;                /* Set last segment flag if end of path */

        if (!i) return FR_INVALID_NAME;                /* Reject nul string */
        if (sfn[0] == DDE) sfn[0] = NDDE;        /* When first char collides with DDE, replace it with 0x05 */

        if (ni == 8) b <<= 2;
        if ((b & 0x03) == 0x01) c |= NS_EXT;        /* NT flag (Name extension has only small capital) */
        if ((b & 0x0C) == 0x04) c |= NS_BODY;        /* NT flag (Name body has only small capital) */

        sfn[NS] = c;                /* Store NT flag, File name is created */

        return FR_OK;
#endif
}




/*-----------------------------------------------------------------------*/
/* Get file information from directory entry                             */
/*-----------------------------------------------------------------------*/
static
void get_fileinfo (                /* No return code */
        FATDIR *dj,                        /* Pointer to the directory object */
        FILINFO *fno                 /* Pointer to the file information to be filled */
)
{
        uint i;
        uint8_t nt, *dir;
        TCHAR *p, c;


        p = fno->fname;
        if (dj->sect) {
                dir = dj->dir;
                nt = dir[DIR_NTres];                /* NT flag */
                for (i = 0; i < 8; i++) {        /* Copy name body */
                        c = dir[i];
                        if (c == ' ') break;
                        if (c == NDDE) c = (TCHAR)DDE;
                        if (_LIBFAT_USE_LFN && (nt & NS_BODY) && IsUpper(c)) c += 0x20;
#if _LIBFAT_LFN_UNICODE
                        if (IsDBCS1(c) && i < 7 && IsDBCS2(dir[i+1]))
                                c = (c << 8) | dir[++i];
                        c = _libfat_convert(c, 1);
                        if (!c) c = '?';
#endif
                        *p++ = c;
                }
                if (dir[8] != ' ') {                /* Copy name extension */
                        *p++ = '.';
                        for (i = 8; i < 11; i++) {
                                c = dir[i];
                                if (c == ' ') break;
                                if (_LIBFAT_USE_LFN && (nt & NS_EXT) && IsUpper(c)) c += 0x20;
#if _LIBFAT_LFN_UNICODE
                                if (IsDBCS1(c) && i < 10 && IsDBCS2(dir[i+1]))
                                        c = (c << 8) | dir[++i];
                                c = _libfat_convert(c, 1);
                                if (!c) c = '?';
#endif
                                *p++ = c;
                        }
                }
                fno->fattrib = dir[DIR_Attr];                                /* Attribute */
                fno->fsize = LOAD_UINT32(dir+DIR_FileSize);        /* Size */
                fno->fdate = LOAD_UINT16(dir+DIR_WrtDate);                /* Date */
                fno->ftime = LOAD_UINT16(dir+DIR_WrtTime);                /* Time */
        }
        *p = 0;                /* Terminate SFN str by a \0 */

#if _LIBFAT_USE_LFN
        if (fno->lfname && fno->lfsize) {
                TCHAR *tp = fno->lfname;
                wchar_t w, *lfn;

                i = 0;
                if (dj->sect && dj->lfn_idx != 0xFFFF) {/* Get LFN if available */
                        lfn = dj->lfn;
                        while ((w = *lfn++) != 0) {                        /* Get an LFN char */
#if !_LIBFAT_LFN_UNICODE
                                w = _libfat_convert(w, 0);                        /* Unicode -> OEM conversion */
                                if (!w) { i = 0; break; }                /* Could not convert, no LFN */
                                if (_DF1S && w >= 0x100)                /* Put 1st byte if it is a DBC (always false on SBCS cfg) */
                                        tp[i++] = (TCHAR)(w >> 8);
#endif
                                if (i >= fno->lfsize - 1) { i = 0; break; }        /* Buffer overflow, no LFN */
                                tp[i++] = (TCHAR)w;
                        }
                }
                tp[i] = 0;        /* Terminate the LFN str by a \0 */
        }
#endif
}


/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

static
FRESULT follow_path (        /* FR_OK(0): successful, !=0: error code */
        FATDIR *dj,                        /* Directory object to return last directory and found object */
        const TCHAR *path        /* Full-path string to find a file or directory */
)
{
        FRESULT res;
        uint8_t *dir, ns;

        if (*path == '/' || *path == '\\')        /* Strip heading separator if exist */
                path++;
        dj->sclust = 0;                                                /* Start from the root dir */

        if ((uint)*path < ' ') {                        /* Nul path means the start directory itself */
                res = dir_sdi(dj, 0);
                dj->dir = 0;
        } else {                                                        /* Follow path */
                for (;;) {
                        res = create_name(dj, &path);        /* Get a segment */
                        if (res != FR_OK) break;
                        res = dir_find(dj);                                /* Find it */
                        ns = *(dj->fn+NS);
                        if (res != FR_OK) {                                /* Failed to find the object */
                                if (res != FR_NO_FILE) break;        /* Abort if any hard error occurred */
                                /* Object not found */
//                                if (_FS_RPATH && (ns & NS_DOT)) {        /* If dot entry is not exit */
//                                        dj->sclust = 0; dj->dir = 0;        /* It is the root dir */
//                                        res = FR_OK;
//                                        if (!(ns & NS_LAST)) continue;
//                                } else {                                                        /* Could not find the object */
                                        if (!(ns & NS_LAST)) res = FR_NO_PATH;
//                                }
                                break;
                        }
                        if (ns & NS_LAST) break;                        /* Last segment match. Function completed. */
                        dir = dj->dir;                                                /* There is next segment. Follow the sub directory */
                        if (!(dir[DIR_Attr] & LIBFAT_AM_DIR)) {        /* Cannot follow because it is a file */
                                res = FR_NO_PATH; break;
                        }
                        dj->sclust = ld_clust(dj->fs, dir);
                }
        }

        return res;
}




/*-----------------------------------------------------------------------*/
/* Load a sector and check if it is an FAT Volume Boot Record            */
/*-----------------------------------------------------------------------*/

static
uint8_t check_fs (        /* 0:FAT-VBR, 1:Any BR but not FAT, 2:Not a BR, 3:Disk error */
        FATFS *fs,        /* File system object */
        uint32_t sect        /* Sector# (lba) to check if it is an FAT boot record or not */
)
{
        if (_libfat_disk_read(fs->srcfile, fs->win, sect, 1) != RES_OK)        /* Load boot record */
                return 3;
        if (LOAD_UINT16(&fs->win[BS_55AA]) != 0xAA55)                /* Check record signature (always placed at offset 510 even if the sector size is >512) */
                return 2;

        if ((LOAD_UINT32(&fs->win[BS_FilSysType]) & 0xFFFFFF) == 0x544146)        /* Check "FAT" string */
                return 0;
        if ((LOAD_UINT32(&fs->win[BS_FilSysType32]) & 0xFFFFFF) == 0x544146)
                return 0;

        return 1;
}




/*-----------------------------------------------------------------------*/
/* Check if the file system object is valid or not                       */
/*-----------------------------------------------------------------------*/

static
FRESULT chk_mounted (        /* FR_OK(0): successful, !=0: any error occurred */
        FATFS *fs
)
{
        uint8_t fmt;
        uint8_t b;
        uint32_t bsect, fasize, tsect, sysect, nclst, szbfat;
        uint16_t nrsv;

#if _LIBFAT_MAX_SS != 512                                                /* Get disk sector size (variable sector size cfg only) */
        if (_libfat_disk_ioctl(fs->drv, GET_SECTOR_SIZE, &fs->ssize) != RES_OK)
                return FR_DISK_ERR;
#endif

        /* Search FAT partition on the drive. Supports only generic partitions, FDISK and SFD. */
        fmt = check_fs(fs, bsect = 0);                /* Load sector 0 and check if it is an FAT-VBR (in SFD) */

        if (fmt == 1) {                                                /* Not an FAT-VBR, the physical drive can be partitioned */
                return FR_NO_FILESYSTEM;
        }
        if (fmt == 3) return FR_DISK_ERR;
        if (fmt) return FR_NO_FILESYSTEM;                /* No FAT volume is found */

        /* An FAT volume is found. Following code initializes the file system object */

        if (LOAD_UINT16(fs->win+BPB_BytsPerSec) != SS(fs))                /* (BPB_BytsPerSec must be equal to the physical sector size) */
                return FR_NO_FILESYSTEM;

        fasize = LOAD_UINT16(fs->win+BPB_FATSz16);                                /* Number of sectors per FAT */
        if (!fasize) fasize = LOAD_UINT32(fs->win+BPB_FATSz32);
        fs->fsize = fasize;

        fs->n_fats = b = fs->win[BPB_NumFATs];                                /* Number of FAT copies */
        if (b != 1 && b != 2) return FR_NO_FILESYSTEM;                /* (Must be 1 or 2) */
        fasize *= b;                                                                                /* Number of sectors for FAT area */

        fs->csize = b = fs->win[BPB_SecPerClus];                        /* Number of sectors per cluster */
        if (!b || (b & (b - 1))) return FR_NO_FILESYSTEM;        /* (Must be power of 2) */

        fs->n_rootdir = LOAD_UINT16(fs->win+BPB_RootEntCnt);        /* Number of root directory entries */
        if (fs->n_rootdir % (SS(fs) / SZ_DIR)) return FR_NO_FILESYSTEM;        /* (BPB_RootEntCnt must be sector aligned) */

        tsect = LOAD_UINT16(fs->win+BPB_TotSec16);                                /* Number of sectors on the volume */
        if (!tsect) tsect = LOAD_UINT32(fs->win+BPB_TotSec32);

        nrsv = LOAD_UINT16(fs->win+BPB_RsvdSecCnt);                                /* Number of reserved sectors */
        if (!nrsv) return FR_NO_FILESYSTEM;                                        /* (BPB_RsvdSecCnt must not be 0) */

        /* Determine the FAT sub type */
        sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZ_DIR);        /* RSV+FAT+DIR */
        if (tsect < sysect) return FR_NO_FILESYSTEM;                /* (Invalid volume size) */
        nclst = (tsect - sysect) / fs->csize;                                /* Number of clusters */
        if (!nclst) return FR_NO_FILESYSTEM;                                /* (Invalid volume size) */
        fmt = LIBFAT_FS_FAT12;
        if (nclst >= MIN_FAT16) fmt = LIBFAT_FS_FAT16;
        if (nclst >= MIN_FAT32) fmt = LIBFAT_FS_FAT32;

        /* Boundaries and Limits */
        fs->n_fatent = nclst + 2;                                                        /* Number of FAT entries */
        fs->volbase = bsect;                                                                /* Volume start sector */
        fs->fatbase = bsect + nrsv;                                                 /* FAT start sector */
        fs->database = bsect + sysect;                                                /* Data start sector */
        if (fmt == LIBFAT_FS_FAT32) {
                if (fs->n_rootdir) return FR_NO_FILESYSTEM;                /* (BPB_RootEntCnt must be 0) */
                fs->dirbase = LOAD_UINT32(fs->win+BPB_RootClus);        /* Root directory start cluster */
                szbfat = fs->n_fatent * 4;                                                /* (Required FAT size) */
        } else {
                if (!fs->n_rootdir)        return FR_NO_FILESYSTEM;        /* (BPB_RootEntCnt must not be 0) */
                fs->dirbase = fs->fatbase + fasize;                                /* Root directory start sector */
                szbfat = (fmt == LIBFAT_FS_FAT16) ?                                        /* (Required FAT size) */
                        fs->n_fatent * 2 : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
        }
        if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs))        /* (BPB_FATSz must not be less than required) */
                return FR_NO_FILESYSTEM;

        /* Initialize cluster allocation information */
        fs->free_clust = 0xFFFFFFFF;
        fs->last_clust = 0;

        /* Get fsinfo if available */
        if (fmt == LIBFAT_FS_FAT32) {
                 fs->fsi_flag = 0;
                fs->fsi_sector = bsect + LOAD_UINT16(fs->win+BPB_FSInfo);
                if (_libfat_disk_read(fs->srcfile, fs->win, fs->fsi_sector, 1) == RES_OK &&
                        LOAD_UINT16(fs->win+BS_55AA) == 0xAA55 &&
                        LOAD_UINT32(fs->win+FSI_LeadSig) == 0x41615252 &&
                        LOAD_UINT32(fs->win+FSI_StrucSig) == 0x61417272) {
                                fs->last_clust = LOAD_UINT32(fs->win+FSI_Nxt_Free);
                                fs->free_clust = LOAD_UINT32(fs->win+FSI_Free_Count);
                }
        }

        fs->fs_type = fmt;                /* FAT sub-type */
        fs->winsect = 0;                /* Invalidate sector cache */
        fs->wflag = 0;
#if _LIBFAT_FS_LOCK                                /* Clear file lock semaphores */
        clear_lock(fs);
#endif

        return FR_OK;
}




/*-----------------------------------------------------------------------*/
/* Check if the file/dir object is valid or not                          */
/*-----------------------------------------------------------------------*/

static
FRESULT validate (        /* FR_OK(0): The object is valid, !=0: Invalid */
        void* obj                /* Pointer to the object FIL/DIR to check validity */
)
{
        FATFILE *fil = (FATFILE*)obj;        /* Assuming offset of fs and id in the FIL/DIR is identical */


        if (!fil || !fil->fs || !fil->fs->fs_type || fil->fs->id != fil->id)
                return FR_INVALID_OBJECT;

        ENTER_FF(fil->fs);                /* Lock file system */

        return FR_OK;
}




/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Logical Drive                                         */
/*-----------------------------------------------------------------------*/
//==============================================================================
/**
 * @brief Mount a Logical Drive
 *
 * @param[in] *fsfile           pointer to opened source file
 * @param[in] *fs               pointer to existing library instance
 */
//==============================================================================
FRESULT libfat_mount(FILE *fsfile, FATFS *fs)
{
        if (fs) {
                fs->srcfile = fsfile;

                /* Create sync object for the new volume */
                if (!_libfat_create_mutex(&fs->sobj))
                        return FR_INT_ERR;

                if (chk_mounted(fs) == FR_OK)
                        return FR_OK;
        }

        return FR_DISK_ERR;
}

//==============================================================================
/**
 * @brief Function unmount drive
 *
 * @param[in] *fs               pointer to existing library instance
 */
//==============================================================================
FRESULT libfat_umount(FATFS *fs)
{
        if (fs) {
                _libfat_delete_mutex(fs->sobj);
                return FR_OK;
        }

        return FR_DISK_ERR;
}


/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

FRESULT libfat_open (
        FATFS *fs,
        FATFILE *fp,                        /* Pointer to the blank file object */
        const TCHAR *path,        /* Pointer to the file name */
        uint8_t mode                        /* Access mode and file open mode flags */
)
{
        FRESULT res;
        FATDIR dj;
        uint8_t *dir;
        DEF_NAMEBUF;


        if (!fp) return FR_INVALID_OBJECT;
        fp->fs = fs;

        mode &= LIBFAT_FA_READ | LIBFAT_FA_WRITE | LIBFAT_FA_CREATE_ALWAYS | LIBFAT_FA_OPEN_ALWAYS | LIBFAT_FA_CREATE_NEW;

        ENTER_FF(fs);
        dj.fs = fs;


                INIT_BUF(dj);
                res = follow_path(&dj, path);        /* Follow the file path */
                dir = dj.dir;

                if (res == FR_OK) {
                        if (!dir)        /* Current dir itself */
                                res = FR_INVALID_NAME;
#if _LIBFAT_FS_LOCK
                        else
                                res = chk_lock(&dj, (mode & ~LIBFAT_FA_READ) ? 1 : 0);
#endif
                }
                /* Create or Open a file */
                if (mode & (LIBFAT_FA_CREATE_ALWAYS | LIBFAT_FA_OPEN_ALWAYS | LIBFAT_FA_CREATE_NEW)) {
                        uint32_t dw, cl;

                        if (res != FR_OK) {                                        /* No file, create new */
                                if (res == FR_NO_FILE)                        /* There is no file to open, create a new entry */
#if _LIBFAT_FS_LOCK
                                        res = enq_lock() ? dir_register(&dj) : FR_TOO_MANY_OPEN_FILES;
#else
                                        res = dir_register(&dj);
#endif
                                mode |= LIBFAT_FA_CREATE_ALWAYS;                /* File is created */
                                dir = dj.dir;                                        /* New entry */
                        }
                        else {                                                                /* Any object is already existing */
                                if (dir[DIR_Attr] & (LIBFAT_AM_RDO | LIBFAT_AM_DIR)) {        /* Cannot overwrite it (R/O or DIR) */
                                        res = FR_DENIED;
                                } else {
                                        if (mode & LIBFAT_FA_CREATE_NEW)        /* Cannot create as new file */
                                                res = FR_EXIST;
                                }
                        }
                        if (res == FR_OK && (mode & LIBFAT_FA_CREATE_ALWAYS)) {        /* Truncate it if overwrite mode */
                                dw = _libfat_get_fattime();                                        /* Created time */
                                STORE_UINT32(dir+DIR_CrtTime, dw);
                                dir[DIR_Attr] = 0;                                        /* Reset attribute */
                                STORE_UINT32(dir+DIR_FileSize, 0);                /* size = 0 */
                                cl = ld_clust(dj.fs, dir);                        /* Get start cluster */
                                st_clust(dir, 0);                                        /* cluster = 0 */
                                dj.fs->wflag = 1;
                                if (cl) {                                                        /* Remove the cluster chain if exist */
                                        dw = dj.fs->winsect;
                                        res = remove_chain(dj.fs, cl);
                                        if (res == FR_OK) {
                                                dj.fs->last_clust = cl - 1;        /* Reuse the cluster hole */
                                                res = move_window(dj.fs, dw);
                                        }
                                }
                        }
                }
                else {        /* Open an existing file */
                        if (res == FR_OK) {                                                /* Follow succeeded */
                                if (dir[DIR_Attr] & LIBFAT_AM_DIR) {                /* It is a directory */
                                        res = FR_NO_FILE;
                                } else {
                                        if ((mode & LIBFAT_FA_WRITE) && (dir[DIR_Attr] & LIBFAT_AM_RDO)) /* R/O violation */
                                                res = FR_DENIED;
                                }
                        }
                }
                if (res == FR_OK) {
                        if (mode & LIBFAT_FA_CREATE_ALWAYS)                        /* Set file change flag if created or overwritten */
                                mode |= LIBFAT_FA__WRITTEN;
                        fp->dir_sect = dj.fs->winsect;                        /* Pointer to the directory entry */
                        fp->dir_ptr = dir;
#if _LIBFAT_FS_LOCK
                        fp->lockid = inc_lock(&dj, (mode & ~LIBFAT_FA_READ) ? 1 : 0);
                        if (!fp->lockid) res = FR_INT_ERR;
#endif
                }

                FREE_BUF();

                if (res == FR_OK) {
                        fp->flag = mode;                                        /* File access mode */
                        fp->sclust = ld_clust(dj.fs, dir);        /* File start cluster */
                        fp->fsize = LOAD_UINT32(dir+DIR_FileSize);        /* File size */
                        fp->fptr = 0;                                                /* File pointer */
                        fp->dsect = 0;

                        /* Validate file object */
                        fp->fs = dj.fs;
                        fp->id = dj.fs->id;
                }

        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

FRESULT libfat_read (
        FATFILE *fp,                 /* Pointer to the file object */
        void *buff,                /* Pointer to data buffer */
        uint btr,                /* Number of bytes to read */
        uint *br                /* Pointer to number of bytes read */
)
{
        FRESULT res;
        uint32_t clst, sect, remain;
        uint rcnt, cc;
        uint8_t csect, *rbuff = (uint8_t*)buff;


        *br = 0;        /* Clear read byte counter */

        res = validate(fp);                                                        /* Check validity */
        if (res != FR_OK) LEAVE_FF(fp->fs, res);
        if (fp->flag & LIBFAT_FA__ERROR)                                        /* Aborted file? */
                LEAVE_FF(fp->fs, FR_INT_ERR);
        if (!(fp->flag & LIBFAT_FA_READ))                                         /* Check access mode */
                LEAVE_FF(fp->fs, FR_DENIED);
        remain = fp->fsize - fp->fptr;
        if (btr > remain) btr = (uint)remain;                /* Truncate btr by remaining bytes */

        for ( ;  btr;                                                                /* Repeat until all data read */
                rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) {
                if ((fp->fptr % SS(fp->fs)) == 0) {                /* On the sector boundary? */
                        csect = (uint8_t)(fp->fptr / SS(fp->fs) & (fp->fs->csize - 1));        /* Sector offset in the cluster */
                        if (!csect) {                                                /* On the cluster boundary? */
                                if (fp->fptr == 0) {                        /* On the top of the file? */
                                        clst = fp->sclust;                        /* Follow from the origin */
                                } else {                                                /* Middle or end of the file */
                                        clst = get_fat(fp->fs, fp->clust);        /* Follow cluster chain on the FAT */
                                }
                                if (clst < 2) ABORT(fp->fs, FR_INT_ERR);
                                if (clst == 0xFFFFFFFF) ABORT(fp->fs, FR_DISK_ERR);
                                fp->clust = clst;                                /* Update current cluster */
                        }
                        sect = clust2sect(fp->fs, fp->clust);        /* Get current sector */
                        if (!sect) ABORT(fp->fs, FR_INT_ERR);
                        sect += csect;
                        cc = btr / SS(fp->fs);                                /* When remaining bytes >= sector size, */
                        if (cc) {                                                        /* Read maximum contiguous sectors directly */
                                if (csect + cc > fp->fs->csize)        /* Clip at cluster boundary */
                                        cc = fp->fs->csize - csect;
                                if (_libfat_disk_read(fp->fs->srcfile, rbuff, sect, (uint8_t)cc) != RES_OK)
                                        ABORT(fp->fs, FR_DISK_ERR);
#if _LIBFAT_FS_TINY
                                if (fp->fs->wflag && fp->fs->winsect - sect < cc)
                                        memcpy(rbuff + ((fp->fs->winsect - sect) * SS(fp->fs)), fp->fs->win, SS(fp->fs));
#else
                                if ((fp->flag & LIBFAT_FA__DIRTY) && fp->dsect - sect < cc)
                                        memcpy(rbuff + ((fp->dsect - sect) * SS(fp->fs)), fp->buf, SS(fp->fs));
#endif
                                rcnt = SS(fp->fs) * cc;                        /* Number of bytes transferred */
                                continue;
                        }
#if !_LIBFAT_FS_TINY
                        if (fp->dsect != sect) {                        /* Load data sector if not in cache */

                                if (fp->flag & LIBFAT_FA__DIRTY) {                /* Write-back dirty sector cache */
                                        if (_libfat_disk_write(fp->fs->srcfile, fp->buf, fp->dsect, 1) != RES_OK)
                                                ABORT(fp->fs, FR_DISK_ERR);
                                        fp->flag &= ~LIBFAT_FA__DIRTY;
                                }

                                if (_libfat_disk_read(fp->fs->srcfile, fp->buf, sect, 1) != RES_OK)        /* Fill sector cache */
                                        ABORT(fp->fs, FR_DISK_ERR);
                        }
#endif
                        fp->dsect = sect;
                }
                rcnt = SS(fp->fs) - ((uint)fp->fptr % SS(fp->fs));        /* Get partial sector data from sector buffer */
                if (rcnt > btr) rcnt = btr;
#if _LIBFAT_FS_TINY
                if (move_window(fp->fs, fp->dsect))                /* Move sector window */
                        ABORT(fp->fs, FR_DISK_ERR);
                memcpy(rbuff, &fp->fs->win[fp->fptr % SS(fp->fs)], rcnt);        /* Pick partial sector */
#else
                memcpy(rbuff, &fp->buf[fp->fptr % SS(fp->fs)], rcnt);        /* Pick partial sector */
#endif
        }

        LEAVE_FF(fp->fs, FR_OK);
}





/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT libfat_write (
        FATFILE *fp,                        /* Pointer to the file object */
        const void *buff,        /* Pointer to the data to be written */
        uint btw,                        /* Number of bytes to write */
        uint *bw                        /* Pointer to number of bytes written */
)
{
        FRESULT res;
        uint32_t clst, sect;
        uint wcnt, cc;
        const uint8_t *wbuff = (const uint8_t*)buff;
        uint8_t csect;


        *bw = 0;        /* Clear write byte counter */

        res = validate(fp);                                                /* Check validity */
        if (res != FR_OK) LEAVE_FF(fp->fs, res);
        if (fp->flag & LIBFAT_FA__ERROR)                                /* Aborted file? */
                LEAVE_FF(fp->fs, FR_INT_ERR);
        if (!(fp->flag & LIBFAT_FA_WRITE))                                /* Check access mode */
                LEAVE_FF(fp->fs, FR_DENIED);
        if ((uint32_t)(fp->fsize + btw) < fp->fsize) btw = 0;        /* File size cannot reach 4GB */

        for ( ;  btw;                                                        /* Repeat until all data written */
                wbuff += wcnt, fp->fptr += wcnt, *bw += wcnt, btw -= wcnt) {
                if ((fp->fptr % SS(fp->fs)) == 0) {        /* On the sector boundary? */
                        csect = (uint8_t)(fp->fptr / SS(fp->fs) & (fp->fs->csize - 1));        /* Sector offset in the cluster */
                        if (!csect) {                                        /* On the cluster boundary? */
                                if (fp->fptr == 0) {                /* On the top of the file? */
                                        clst = fp->sclust;                /* Follow from the origin */
                                        if (clst == 0)                        /* When no cluster is allocated, */
                                                fp->sclust = clst = create_chain(fp->fs, 0);        /* Create a new cluster chain */
                                } else {                                        /* Middle or end of the file */
                                        clst = create_chain(fp->fs, fp->clust);        /* Follow or stretch cluster chain on the FAT */
                                }
                                if (clst == 0) break;                /* Could not allocate a new cluster (disk full) */
                                if (clst == 1) ABORT(fp->fs, FR_INT_ERR);
                                if (clst == 0xFFFFFFFF) ABORT(fp->fs, FR_DISK_ERR);
                                fp->clust = clst;                        /* Update current cluster */
                        }
#if _LIBFAT_FS_TINY
                        if (fp->fs->winsect == fp->dsect && sync_window(fp->fs))        /* Write-back sector cache */
                                ABORT(fp->fs, FR_DISK_ERR);
#else
                        if (fp->flag & LIBFAT_FA__DIRTY) {                /* Write-back sector cache */
                                if (_libfat_disk_write(fp->fs->srcfile, fp->buf, fp->dsect, 1) != RES_OK)
                                        ABORT(fp->fs, FR_DISK_ERR);
                                fp->flag &= ~LIBFAT_FA__DIRTY;
                        }
#endif
                        sect = clust2sect(fp->fs, fp->clust);        /* Get current sector */
                        if (!sect) ABORT(fp->fs, FR_INT_ERR);
                        sect += csect;
                        cc = btw / SS(fp->fs);                        /* When remaining bytes >= sector size, */
                        if (cc) {                                                /* Write maximum contiguous sectors directly */
                                if (csect + cc > fp->fs->csize)        /* Clip at cluster boundary */
                                        cc = fp->fs->csize - csect;
                                if (_libfat_disk_write(fp->fs->srcfile, wbuff, sect, (uint8_t)cc) != RES_OK)
                                        ABORT(fp->fs, FR_DISK_ERR);
#if _LIBFAT_FS_TINY
                                if (fp->fs->winsect - sect < cc) {        /* Refill sector cache if it gets invalidated by the direct write */
                                        memcpy(fp->fs->win, wbuff + ((fp->fs->winsect - sect) * SS(fp->fs)), SS(fp->fs));
                                        fp->fs->wflag = 0;
                                }
#else
                                if (fp->dsect - sect < cc) { /* Refill sector cache if it gets invalidated by the direct write */
                                        memcpy(fp->buf, wbuff + ((fp->dsect - sect) * SS(fp->fs)), SS(fp->fs));
                                        fp->flag &= ~LIBFAT_FA__DIRTY;
                                }
#endif
                                wcnt = SS(fp->fs) * cc;                /* Number of bytes transferred */
                                continue;
                        }
#if _LIBFAT_FS_TINY
                        if (fp->fptr >= fp->fsize) {        /* Avoid silly cache filling at growing edge */
                                if (sync_window(fp->fs)) ABORT(fp->fs, FR_DISK_ERR);
                                fp->fs->winsect = sect;
                        }
#else
                        if (fp->dsect != sect) {                /* Fill sector cache with file data */
                                if (fp->fptr < fp->fsize &&
                                        _libfat_disk_read(fp->fs->srcfile, fp->buf, sect, 1) != RES_OK)
                                                ABORT(fp->fs, FR_DISK_ERR);
                        }
#endif
                        fp->dsect = sect;
                }
                wcnt = SS(fp->fs) - ((uint)fp->fptr % SS(fp->fs));/* Put partial sector into file I/O buffer */
                if (wcnt > btw) wcnt = btw;
#if _LIBFAT_FS_TINY
                if (move_window(fp->fs, fp->dsect))        /* Move sector window */
                        ABORT(fp->fs, FR_DISK_ERR);
                memcpy(&fp->fs->win[fp->fptr % SS(fp->fs)], wbuff, wcnt);        /* Fit partial sector */
                fp->fs->wflag = 1;
#else
                memcpy(&fp->buf[fp->fptr % SS(fp->fs)], wbuff, wcnt);        /* Fit partial sector */
                fp->flag |= LIBFAT_FA__DIRTY;
#endif
        }

        if (fp->fptr > fp->fsize) fp->fsize = fp->fptr;        /* Update file size if needed */
        fp->flag |= LIBFAT_FA__WRITTEN;                                                /* Set file change flag */

        LEAVE_FF(fp->fs, FR_OK);
}




/*-----------------------------------------------------------------------*/
/* Synchronize the File Object                                           */
/*-----------------------------------------------------------------------*/

FRESULT libfat_sync (
        FATFILE *fp                /* Pointer to the file object */
)
{
        FRESULT res;
        uint32_t tm;
        uint8_t *dir;


        res = validate(fp);                                        /* Check validity of the object */
        if (res == FR_OK) {
                if (fp->flag & LIBFAT_FA__WRITTEN) {        /* Has the file been written? */
#if !_LIBFAT_FS_TINY        /* Write-back dirty buffer */
                        if (fp->flag & LIBFAT_FA__DIRTY) {
                                if (_libfat_disk_write(fp->fs->srcfile, fp->buf, fp->dsect, 1) != RES_OK)
                                        LEAVE_FF(fp->fs, FR_DISK_ERR);
                                fp->flag &= ~LIBFAT_FA__DIRTY;
                        }
#endif
                        /* Update the directory entry */
                        res = move_window(fp->fs, fp->dir_sect);
                        if (res == FR_OK) {
                                dir = fp->dir_ptr;
                                dir[DIR_Attr] |= LIBFAT_AM_ARC;                                        /* Set archive bit */
                                STORE_UINT32(dir+DIR_FileSize, fp->fsize);                /* Update file size */
                                st_clust(dir, fp->sclust);                                        /* Update start cluster */
                                tm = _libfat_get_fattime();                                                        /* Update updated time */
                                STORE_UINT32(dir+DIR_WrtTime, tm);
                                STORE_UINT16(dir+DIR_LstAccDate, 0);
                                fp->flag &= ~LIBFAT_FA__WRITTEN;
                                fp->fs->wflag = 1;
                                res = sync_fs(fp->fs);
                        }
                }
        }

        LEAVE_FF(fp->fs, res);
}


/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

FRESULT libfat_close (
        FATFILE *fp                /* Pointer to the file object to be closed */
)
{
        FRESULT res;

        res = libfat_sync(fp);                /* Flush cached data */
#if _LIBFAT_FS_LOCK
        if (res == FR_OK) {                /* Decrement open counter */
                FATFS *fs = fp->fs;;
                res = validate(fp);
                if (res == FR_OK) {
                        res = dec_lock(fp->lockid);
                        unlock_fs(fs, FR_OK);
                }
        }
#endif
        if (res == FR_OK) fp->fs = 0;        /* Discard file object */
        return res;
}


/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/
FRESULT libfat_lseek (
        FATFILE *fp,                /* Pointer to the file object */
        uint32_t ofs                /* File pointer from top of file */
)
{
        FRESULT res;


        res = validate(fp);                                        /* Check validity of the object */
        if (res != FR_OK) LEAVE_FF(fp->fs, res);
        if (fp->flag & LIBFAT_FA__ERROR)                        /* Check abort flag */
                LEAVE_FF(fp->fs, FR_INT_ERR);

                uint32_t clst, bcs, nsect, ifptr;

                if (ofs > fp->fsize                                        /* In read-only mode, clip offset with the file size */
                         && !(fp->flag & LIBFAT_FA_WRITE)
                        ) ofs = fp->fsize;

                ifptr = fp->fptr;
                fp->fptr = nsect = 0;
                if (ofs) {
                        bcs = (uint32_t)fp->fs->csize * SS(fp->fs);        /* Cluster size (byte) */
                        if (ifptr > 0 &&
                                (ofs - 1) / bcs >= (ifptr - 1) / bcs) {        /* When seek to same or following cluster, */
                                fp->fptr = (ifptr - 1) & ~(bcs - 1);        /* start from the current cluster */
                                ofs -= fp->fptr;
                                clst = fp->clust;
                        } else {                                                                        /* When seek to back cluster, */
                                clst = fp->sclust;                                                /* start from the first cluster */

                                if (clst == 0) {                                                /* If no cluster chain, create a new chain */
                                        clst = create_chain(fp->fs, 0);
                                        if (clst == 1) ABORT(fp->fs, FR_INT_ERR);
                                        if (clst == 0xFFFFFFFF) ABORT(fp->fs, FR_DISK_ERR);
                                        fp->sclust = clst;
                                }

                                fp->clust = clst;
                        }
                        if (clst != 0) {
                                while (ofs > bcs) {                                                /* Cluster following loop */

                                        if (fp->flag & LIBFAT_FA_WRITE) {                        /* Check if in write mode or not */
                                                clst = create_chain(fp->fs, clst);        /* Force stretch if in write mode */
                                                if (clst == 0) {                                /* When disk gets full, clip file size */
                                                        ofs = bcs; break;
                                                }
                                        } else

                                                clst = get_fat(fp->fs, clst);        /* Follow cluster chain if not in write mode */
                                        if (clst == 0xFFFFFFFF) ABORT(fp->fs, FR_DISK_ERR);
                                        if (clst <= 1 || clst >= fp->fs->n_fatent) ABORT(fp->fs, FR_INT_ERR);
                                        fp->clust = clst;
                                        fp->fptr += bcs;
                                        ofs -= bcs;
                                }
                                fp->fptr += ofs;
                                if (ofs % SS(fp->fs)) {
                                        nsect = clust2sect(fp->fs, clst);        /* Current sector */
                                        if (!nsect) ABORT(fp->fs, FR_INT_ERR);
                                        nsect += ofs / SS(fp->fs);
                                }
                        }
                }
                if (fp->fptr % SS(fp->fs) && nsect != fp->dsect) {        /* Fill sector cache if needed */
#if !_LIBFAT_FS_TINY

                        if (fp->flag & LIBFAT_FA__DIRTY) {                        /* Write-back dirty sector cache */
                                if (_libfat_disk_write(fp->fs->srcfile, fp->buf, fp->dsect, 1) != RES_OK)
                                        ABORT(fp->fs, FR_DISK_ERR);
                                fp->flag &= ~LIBFAT_FA__DIRTY;
                        }

                        if (_libfat_disk_read(fp->fs->srcfile, fp->buf, nsect, 1) != RES_OK)        /* Fill sector cache */
                                ABORT(fp->fs, FR_DISK_ERR);
#endif
                        fp->dsect = nsect;
                }

                if (fp->fptr > fp->fsize) {                        /* Set file change flag if the file size is extended */
                        fp->fsize = fp->fptr;
                        fp->flag |= LIBFAT_FA__WRITTEN;
                }

        LEAVE_FF(fp->fs, res);
}


/*-----------------------------------------------------------------------*/
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/

FRESULT libfat_opendir (
        FATFS *fs,
        FATDIR *dj,                        /* Pointer to directory object to create */
        const TCHAR *path        /* Pointer to the directory path */
)
{
        FRESULT res;

        DEF_NAMEBUF;


        if (!dj) return FR_INVALID_OBJECT;

        ENTER_FF(fs);
        dj->fs = fs;

                INIT_BUF(*dj);
                res = follow_path(dj, path);                        /* Follow the path to the directory */
                FREE_BUF();
                if (res == FR_OK) {                                                /* Follow completed */
                        if (dj->dir) {                                                /* It is not the root dir */
                                if (dj->dir[DIR_Attr] & LIBFAT_AM_DIR) {        /* The object is a directory */
                                        dj->sclust = ld_clust(fs, dj->dir);
                                } else {                                                /* The object is not a directory */
                                        res = FR_NO_PATH;
                                }
                        }
                        if (res == FR_OK) {
                                dj->id = fs->id;
                                res = dir_sdi(dj, 0);                        /* Rewind dir */
                        }
                }
                if (res == FR_NO_FILE) res = FR_NO_PATH;
                if (res != FR_OK) dj->fs = 0;                        /* Invalidate the dir object if function faild */

        LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Read Directory Entry in Sequence                                      */
/*-----------------------------------------------------------------------*/

FRESULT libfat_readdir (
        FATDIR *dj,                        /* Pointer to the open directory object */
        FILINFO *fno                /* Pointer to file information to return */
)
{
        FRESULT res;
        DEF_NAMEBUF;


        res = validate(dj);                                                /* Check validity of the object */
        if (res == FR_OK) {
                if (!fno) {
                        res = dir_sdi(dj, 0);                        /* Rewind the directory object */
                } else {
                        INIT_BUF(*dj);
                        res = dir_read(dj, 0);                        /* Read an item */
                        if (res == FR_NO_FILE) {                /* Reached end of dir */
                                dj->sect = 0;
                                res = FR_OK;
                        }
                        if (res == FR_OK) {                                /* A valid entry is found */
                                get_fileinfo(dj, fno);                /* Get the object information */
                                res = dir_next(dj, 0);                /* Increment index for next */
                                if (res == FR_NO_FILE) {
                                        dj->sect = 0;
                                        res = FR_OK;
                                }
                        }
                        FREE_BUF();
                }
        }

        LEAVE_FF(dj->fs, res);
}


/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

FRESULT libfat_stat (
        FATFS *fs,
        const TCHAR *path,        /* Pointer to the file path */
        FILINFO *fno                /* Pointer to file information to return */
)
{
        FRESULT res;
        FATDIR dj;
        DEF_NAMEBUF;

        ENTER_FF(fs);
        dj.fs = fs;

                INIT_BUF(dj);
                res = follow_path(&dj, path);        /* Follow the file path */
                if (res == FR_OK) {                                /* Follow completed */
                        if (dj.dir)                /* Found an object */
                                get_fileinfo(&dj, fno);
                        else                        /* It is root dir */
                                res = FR_INVALID_NAME;
                }
                FREE_BUF();


        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

FRESULT libfat_getfree (
        uint32_t *nclst,                /* Pointer to a variable to return number of free clusters */
        FATFS *fs                /* Pointer to return pointer to corresponding file system object */
)
{
        FRESULT res = FR_OK;
//        FATFS *fs;
        uint32_t n, clst, sect, stat;
        uint i;
        uint8_t fat, *p;


        /* Get drive number */
//        res = chk_mounted(*fatfs, &path, fatfs, 0);
        ENTER_FF(fs);
//        res = FR_OK;

//        if (res == FR_OK) {
                /* If free_clust is valid, return it without full cluster scan */
                if (fs->free_clust <= fs->n_fatent - 2) {
                        *nclst = fs->free_clust;
                } else {
                        /* Get number of free clusters */
                        fat = fs->fs_type;
                        n = 0;
                        if (fat == LIBFAT_FS_FAT12) {
                                clst = 2;
                                do {
                                        stat = get_fat(fs, clst);
                                        if (stat == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
                                        if (stat == 1) { res = FR_INT_ERR; break; }
                                        if (stat == 0) n++;
                                } while (++clst < fs->n_fatent);
                        } else {
                                clst = fs->n_fatent;
                                sect = fs->fatbase;
                                i = 0; p = 0;
                                do {
                                        if (!i) {
                                                res = move_window(fs, sect++);
                                                if (res != FR_OK) break;
                                                p = fs->win;
                                                i = SS(fs);
                                        }
                                        if (fat == LIBFAT_FS_FAT16) {
                                                if (LOAD_UINT16(p) == 0) n++;
                                                p += 2; i -= 2;
                                        } else {
                                                if ((LOAD_UINT32(p) & 0x0FFFFFFF) == 0) n++;
                                                p += 4; i -= 4;
                                        }
                                } while (--clst);
                        }
                        fs->free_clust = n;
                        if (fat == LIBFAT_FS_FAT32) fs->fsi_flag = 1;
                        *nclst = n;
                }
//        }
        LEAVE_FF(fs, res);
}




/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

FRESULT libfat_truncate (
        FATFILE *fp                /* Pointer to the file object */
)
{
        FRESULT res;
        uint32_t ncl;


        res = validate(fp);                                                /* Check validity of the object */
        if (res == FR_OK) {
                if (fp->flag & LIBFAT_FA__ERROR) {                        /* Check abort flag */
                        res = FR_INT_ERR;
                } else {
                        if (!(fp->flag & LIBFAT_FA_WRITE))                /* Check access mode */
                                res = FR_DENIED;
                }
        }
        if (res == FR_OK) {
                if (fp->fsize > fp->fptr) {
                        fp->fsize = fp->fptr;        /* Set file size to current R/W point */
                        fp->flag |= LIBFAT_FA__WRITTEN;
                        if (fp->fptr == 0) {        /* When set file size to zero, remove entire cluster chain */
                                res = remove_chain(fp->fs, fp->sclust);
                                fp->sclust = 0;
                        } else {                                /* When truncate a part of the file, remove remaining clusters */
                                ncl = get_fat(fp->fs, fp->clust);
                                res = FR_OK;
                                if (ncl == 0xFFFFFFFF) res = FR_DISK_ERR;
                                if (ncl == 1) res = FR_INT_ERR;
                                if (res == FR_OK && ncl < fp->fs->n_fatent) {
                                        res = put_fat(fp->fs, fp->clust, 0x0FFFFFFF);
                                        if (res == FR_OK) res = remove_chain(fp->fs, ncl);
                                }
                        }
                }
                if (res != FR_OK) fp->flag |= LIBFAT_FA__ERROR;
        }

        LEAVE_FF(fp->fs, res);
}




/*-----------------------------------------------------------------------*/
/* Delete a File or Directory                                            */
/*-----------------------------------------------------------------------*/

FRESULT libfat_unlink (
        FATFS *fs,
        const TCHAR *path                /* Pointer to the file or directory path */
)
{
        FRESULT res;
        FATDIR dj, sdj;
        uint8_t *dir;
        uint32_t dclst;
        DEF_NAMEBUF;

//        res = chk_mounted(fs, &path, &dj.fs, 1);
        ENTER_FF(fs);
        dj.fs = fs;
//        if (res == FR_OK) {
                INIT_BUF(dj);
                res = follow_path(&dj, path);                /* Follow the file path */
//                if (_FS_RPATH && res == FR_OK && (dj.fn[NS] & NS_DOT))
//                        res = FR_INVALID_NAME;                        /* Cannot remove dot entry */
#if _LIBFAT_FS_LOCK
                if (res == FR_OK) res = chk_lock(&dj, 2);        /* Cannot remove open file */
#endif
                if (res == FR_OK) {                                        /* The object is accessible */
                        dir = dj.dir;
                        if (!dir) {
                                res = FR_INVALID_NAME;                /* Cannot remove the start directory */
                        } else {
                                if (dir[DIR_Attr] & LIBFAT_AM_RDO)
                                        res = FR_DENIED;                /* Cannot remove R/O object */
                        }
                        dclst = ld_clust(dj.fs, dir);
                        if (res == FR_OK && (dir[DIR_Attr] & LIBFAT_AM_DIR)) {        /* Is it a sub-dir? */
                                if (dclst < 2) {
                                        res = FR_INT_ERR;
                                } else {
                                        memcpy(&sdj, &dj, sizeof (FATDIR));        /* Check if the sub-dir is empty or not */
                                        sdj.sclust = dclst;
                                        res = dir_sdi(&sdj, 2);                /* Exclude dot entries */
                                        if (res == FR_OK) {
                                                res = dir_read(&sdj, 0);        /* Read an item */
                                                if (res == FR_OK                /* Not empty dir */
                                                ) res = FR_DENIED;
                                                if (res == FR_NO_FILE) res = FR_OK;        /* Empty */
                                        }
                                }
                        }
                        if (res == FR_OK) {
                                res = dir_remove(&dj);                /* Remove the directory entry */
                                if (res == FR_OK) {
                                        if (dclst)                                /* Remove the cluster chain if exist */
                                                res = remove_chain(dj.fs, dclst);
                                        if (res == FR_OK) res = sync_fs(dj.fs);
                                }
                        }
                }
                FREE_BUF();
//        }

        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

FRESULT libfat_mkdir(
        FATFS *fs,
        const TCHAR *path                /* Pointer to the directory path */
)
{
        FRESULT res;
        FATDIR dj;
        uint8_t *dir, n;
        uint32_t dsc, dcl, pcl, tm = _libfat_get_fattime();
        DEF_NAMEBUF;

//        res = chk_mounted(fs, &path, &dj.fs, 1);
        ENTER_FF(fs);
        dj.fs = fs;
//        if (res == FR_OK) {
                INIT_BUF(dj);
                res = follow_path(&dj, path);                        /* Follow the file path */
                if (res == FR_OK) res = FR_EXIST;                /* Any object with same name is already existing */
//                if (_FS_RPATH && res == FR_NO_FILE && (dj.fn[NS] & NS_DOT))
//                        res = FR_INVALID_NAME;
                if (res == FR_NO_FILE) {                                /* Can create a new directory */
                        dcl = create_chain(dj.fs, 0);                /* Allocate a cluster for the new directory table */
                        res = FR_OK;
                        if (dcl == 0) res = FR_DENIED;                /* No space to allocate a new cluster */
                        if (dcl == 1) res = FR_INT_ERR;
                        if (dcl == 0xFFFFFFFF) res = FR_DISK_ERR;
                        if (res == FR_OK)                                        /* Flush FAT */
                                res = sync_window(dj.fs);
                        if (res == FR_OK) {                                        /* Initialize the new directory table */
                                dsc = clust2sect(dj.fs, dcl);
                                dir = dj.fs->win;
                                memset(dir, 0, SS(dj.fs));
                                memset(dir+DIR_Name, ' ', 11);        /* Create "." entry */
                                dir[DIR_Name] = '.';
                                dir[DIR_Attr] = LIBFAT_AM_DIR;
                                STORE_UINT32(dir+DIR_WrtTime, tm);
                                st_clust(dir, dcl);
                                memcpy(dir+SZ_DIR, dir, SZ_DIR);         /* Create ".." entry */
                                dir[33] = '.'; pcl = dj.sclust;
                                if (dj.fs->fs_type == LIBFAT_FS_FAT32 && pcl == dj.fs->dirbase)
                                        pcl = 0;
                                st_clust(dir+SZ_DIR, pcl);
                                for (n = dj.fs->csize; n; n--) {        /* Write dot entries and clear following sectors */
                                        dj.fs->winsect = dsc++;
                                        dj.fs->wflag = 1;
                                        res = sync_window(dj.fs);
                                        if (res != FR_OK) break;
                                        memset(dir, 0, SS(dj.fs));
                                }
                        }
                        if (res == FR_OK) res = dir_register(&dj);        /* Register the object to the directoy */
                        if (res != FR_OK) {
                                remove_chain(dj.fs, dcl);                        /* Could not register, remove cluster chain */
                        } else {
                                dir = dj.dir;
                                dir[DIR_Attr] = LIBFAT_AM_DIR;                                /* Attribute */
                                STORE_UINT32(dir+DIR_WrtTime, tm);                /* Created time */
                                st_clust(dir, dcl);                                        /* Table start cluster */
                                dj.fs->wflag = 1;
                                res = sync_fs(dj.fs);
                        }
                }
                FREE_BUF();
//        }

        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/

FRESULT libfat_chmod (
        FATFS *fs,
        const TCHAR *path,        /* Pointer to the file path */
        uint8_t value,                        /* Attribute bits */
        uint8_t mask                        /* Attribute mask to change */
)
{
        FRESULT res;
        FATDIR dj;
        uint8_t *dir;
        DEF_NAMEBUF;

//        res = chk_mounted(fs, &path, &dj.fs, 1);
        ENTER_FF(fs);
        dj.fs = fs;
//        if (res == FR_OK) {
                INIT_BUF(dj);
                res = follow_path(&dj, path);                /* Follow the file path */
                FREE_BUF();
//                if (_FS_RPATH && res == FR_OK && (dj.fn[NS] & NS_DOT))
//                        res = FR_INVALID_NAME;
                if (res == FR_OK) {
                        dir = dj.dir;
                        if (!dir) {                                                /* Is it a root directory? */
                                res = FR_INVALID_NAME;
                        } else {                                                /* File or sub directory */
                                mask &= LIBFAT_AM_RDO|LIBFAT_AM_HID|LIBFAT_AM_SYS|LIBFAT_AM_ARC;        /* Valid attribute mask */
                                dir[DIR_Attr] = (value & mask) | (dir[DIR_Attr] & (uint8_t)~mask);        /* Apply attribute change */
                                dj.fs->wflag = 1;
                                res = sync_fs(dj.fs);
                        }
                }
//        }

        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

FRESULT libfat_utime (
        FATFS *fs,
        const TCHAR *path,        /* Pointer to the file/directory name */
        const FILINFO *fno        /* Pointer to the time stamp to be set */
)
{
        FRESULT res;
        FATDIR dj;
        uint8_t *dir;
        DEF_NAMEBUF;


//        res = chk_mounted(fs, &path, &dj.fs, 1);
        ENTER_FF(fs);
        dj.fs = fs;
//        if (res == FR_OK) {
                INIT_BUF(dj);
                res = follow_path(&dj, path);        /* Follow the file path */
                FREE_BUF();
//                if (_FS_RPATH && res == FR_OK && (dj.fn[NS] & NS_DOT))
//                        res = FR_INVALID_NAME;
                if (res == FR_OK) {
                        dir = dj.dir;
                        if (!dir) {                                        /* Root directory */
                                res = FR_INVALID_NAME;
                        } else {                                        /* File or sub-directory */
                                STORE_UINT16(dir+DIR_WrtTime, fno->ftime);
                                STORE_UINT16(dir+DIR_WrtDate, fno->fdate);
                                dj.fs->wflag = 1;
                                res = sync_fs(dj.fs);
                        }
                }
//        }

        LEAVE_FF(dj.fs, res);
}




/*-----------------------------------------------------------------------*/
/* Rename File/Directory                                                 */
/*-----------------------------------------------------------------------*/

FRESULT libfat_rename (
        FATFS *fs,
        const TCHAR *path_old,        /* Pointer to the old name */
        const TCHAR *path_new        /* Pointer to the new name */
)
{
        FRESULT res;
        FATDIR djo, djn;
        uint8_t buf[21], *dir;
        uint32_t dw;
        DEF_NAMEBUF;


//        res = chk_mounted(fs, &path_old, &djo.fs, 1);
        ENTER_FF(fs);
        djo.fs = fs;
//        if (res == FR_OK) {
                djn.fs = djo.fs;
                INIT_BUF(djo);
                res = follow_path(&djo, path_old);                /* Check old object */
//                if (_FS_RPATH && res == FR_OK && (djo.fn[NS] & NS_DOT))
//                        res = FR_INVALID_NAME;
#if _LIBFAT_FS_LOCK
                if (res == FR_OK) res = chk_lock(&djo, 2);
#endif
                if (res == FR_OK) {                                                /* Old object is found */
                        if (!djo.dir) {                                                /* Is root dir? */
                                res = FR_NO_FILE;
                        } else {
                                memcpy(buf, djo.dir+DIR_Attr, 21);                /* Save the object information except for name */
                                memcpy(&djn, &djo, sizeof (FATDIR));                /* Check new object */
                                res = follow_path(&djn, path_new);
                                if (res == FR_OK) res = FR_EXIST;                /* The new object name is already existing */
                                if (res == FR_NO_FILE) {                                 /* Is it a valid path and no name collision? */
/* Start critical section that any interruption can cause a cross-link */
                                        res = dir_register(&djn);                        /* Register the new entry */
                                        if (res == FR_OK) {
                                                dir = djn.dir;                                        /* Copy object information except for name */
                                                memcpy(dir+13, buf+2, 19);
                                                dir[DIR_Attr] = buf[0] | LIBFAT_AM_ARC;
                                                djo.fs->wflag = 1;
                                                if (djo.sclust != djn.sclust && (dir[DIR_Attr] & LIBFAT_AM_DIR)) {                /* Update .. entry in the directory if needed */
                                                        dw = clust2sect(djo.fs, ld_clust(djo.fs, dir));
                                                        if (!dw) {
                                                                res = FR_INT_ERR;
                                                        } else {
                                                                res = move_window(djo.fs, dw);
                                                                dir = djo.fs->win+SZ_DIR;        /* .. entry */
                                                                if (res == FR_OK && dir[1] == '.') {
                                                                        dw = (djo.fs->fs_type == LIBFAT_FS_FAT32 && djn.sclust == djo.fs->dirbase) ? 0 : djn.sclust;
                                                                        st_clust(dir, dw);
                                                                        djo.fs->wflag = 1;
                                                                }
                                                        }
                                                }
                                                if (res == FR_OK) {
                                                        res = dir_remove(&djo);                /* Remove old entry */
                                                        if (res == FR_OK)
                                                                res = sync_fs(djo.fs);
                                                }
                                        }
/* End critical section */
                                }
                        }
                }
                FREE_BUF();
//        }

        LEAVE_FF(djo.fs, res);
}

#ifdef __cplusplus
}
#endif

/*==============================================================================
  End of file
==============================================================================*/
