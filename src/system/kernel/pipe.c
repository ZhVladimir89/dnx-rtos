/*=========================================================================*//**
@file    pipe.c

@author  Daniel Zorychta

@brief   File support creating of pipies in file systems.

@note    Copyright (C) 2014-2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include "config.h"
#include <stdbool.h>
#include <sys/types.h>
#include "stdc/errno.h"
#include "kernel/kwrapper.h"
#include "kernel/pipe.h"

/*==============================================================================
  Local macros
==============================================================================*/

/*==============================================================================
  Local object types
==============================================================================*/
struct pipe {
        queue_t     *queue;
        struct pipe *self;
        bool         closed;
};

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local objects
==============================================================================*/
static const uint pipe_read_timeout  = MAX_DELAY_MS;
static const uint pipe_write_timeout = MAX_DELAY_MS;

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  External objects
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief  Check if pipe is valid
 * @param  this         pipe object
 * @return true if valid, otherwise false
 */
//==============================================================================
static bool is_valid(pipe_t *this)
{
        return this && this->self == this;
}

//==============================================================================
/**
 * @brief Create pipe object
 *
 * @return pointer to pipe object
 */
//==============================================================================
pipe_t *_pipe_new()
{
        pipe_t  *pipe  = _kmalloc(sizeof(pipe_t));
        queue_t *queue = _queue_new(CONFIG_PIPE_LENGTH, sizeof(u8_t));

        if (pipe && queue) {

                pipe->queue  = queue;
                pipe->self   = pipe;
                pipe->closed = false;

        } else {
                if (queue) {
                        _queue_delete(queue);
                }

                if (pipe) {
                        _kfree(pipe);
                        pipe = NULL;
                }
        }

        return pipe;
}

//==============================================================================
/**
 * @brief Destroy pipe object
 *
 * @param pipe          a pipe object
 */
//==============================================================================
void _pipe_delete(pipe_t *pipe)
{
        if (is_valid(pipe)) {
                _queue_delete(pipe->queue);
                pipe->self = NULL;
                _kfree(pipe);
        }
}

//==============================================================================
/**
 * @brief Return length of pipe
 *
 * @param pipe          a pipe object
 *
 * @return length or -1 if error
 */
//==============================================================================
int _pipe_get_length(pipe_t *pipe)
{
        if (is_valid(pipe)) {
                return _queue_get_number_of_items(pipe->queue);
        } else {
                return -1;
        }
}

//==============================================================================
/**
 * @brief Read data from pipe
 *
 * @param pipe          a pipe object
 * @param buf           a destination buffer
 * @param count         a count of bytes to read
 * @param rdcnt         a number of read bytes
 * @param non_blocking  a non-blocking access mode
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
int _pipe_read(pipe_t *pipe, u8_t *buf, size_t count, size_t *rdcnt, bool non_blocking)
{
        if (is_valid(pipe) && buf && count) {

                size_t n = 0;
                for (; n < count; n++) {

                        if (pipe->closed && _queue_get_number_of_items(pipe->queue) <= 0) {
                                u8_t null = '\0';
                                _queue_send(pipe->queue, &null, pipe_write_timeout);
                                break;
                        }

                        if (!_queue_receive(pipe->queue, &buf[n], non_blocking ? 0 : pipe_read_timeout)) {
                                break;
                        }
                }

                *rdcnt = n;
                return ESUCC;
        } else {
                return EINVAL;
        }
}

//==============================================================================
/**
 * @brief Read data from pipe
 *
 * @param pipe          a pipe object
 * @param buf           a destination buffer
 * @param count         a count of bytes to read
 * @param wrcnt         a number of written bytes
 * @param non_blocking  a non-blocking access mode
 *
 * @return One of errno value (errno.h)
 */
//==============================================================================
int _pipe_write(pipe_t *pipe, const u8_t *buf, size_t count, size_t *wrcnt, bool non_blocking)
{
        if (is_valid(pipe) && buf && count) {

                size_t n = 0;
                for (; n < count; n++) {

                        if (pipe->closed && _queue_get_number_of_items(pipe->queue) <= 0) {
                                break;
                        }

                        if (!_queue_send(pipe->queue, &buf[n], non_blocking ? 0 : pipe_write_timeout)) {
                                break;
                        }
                }

                *wrcnt = n;
                return ESUCC;
        } else {
                return EINVAL;
        }
}

//==============================================================================
/**
 * @brief Close pipe
 *
 * @param pipe          a pipe object
 *
 * @return true if pipe closed, otherwise false
 */
//==============================================================================
bool _pipe_close(pipe_t *pipe)
{
        if (is_valid(pipe)) {
                pipe->closed = true;

                const u8_t nul = '\0';
                return _queue_send(pipe->queue, &nul, pipe_write_timeout);
        } else {
                return false;
        }
}

//==============================================================================
/**
 * @brief  Clear pipe
 *
 * @param  pipe         a pipe object
 *
 * @return On success true is returned, otherwise false.
 */
//==============================================================================
bool _pipe_clear(pipe_t *pipe)
{
        if (is_valid(pipe)) {
                _queue_reset(pipe->queue);
                return true;
        } else {
                return false;
        }
}

/*==============================================================================
  End of file
==============================================================================*/
