#ifndef _SDSPI_CFG_H_
#define _SDSPI_CFG_H_
/*=========================================================================*//**
@file    sdspi_cfg.h

@author  Daniel Zorychta

@brief   This file support configuration for SD in SPI mode

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
#include "stm32f1/stm32f10x.h"
#include "drivers/gpio_def.h"

/*==============================================================================
  Exported symbolic constants/macros
==============================================================================*/
/* GPIO file */
#define SDSPI_GPIO_FILE         "/dev/gpio"

/* card waiting timeout [ms] */
#define SDSPI_WAIT_TIMEOUT      500

/* device numbers */
#define SDSPI_DEV_NO            0
#define SDSPI_DEV_PART          0

/* supported SPI peripheral */
#define SDSPI_PORT              SPI3

/* enable (1) or disable (0) DMA support */
#define SDSPI_ENABLE_DMA        0

#if (SDSPI_ENABLE_DMA != 0)
/* Tx DMA channel */
#define SDSPI_DMA_TX_CHANNEL    DMA2_Channel4

/* Rx DMA channel */
#define SDSPI_DMA_RX_CHANNEL    DMA2_Channel5
#endif

/*==============================================================================
  Exported types, enums definitions
==============================================================================*/

/*==============================================================================
  Exported object declarations
==============================================================================*/

/*==============================================================================
  Exported function prototypes
==============================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _SDSPI_CFG_H_ */
/*==============================================================================
  End of file
==============================================================================*/
