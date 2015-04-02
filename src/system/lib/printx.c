/*=========================================================================*//**
@file    printx.c

@author  Daniel Zorychta

@brief   Basic print functions

@note    Copyright (C) 2013, 2014 Daniel Zorychta <daniel.zorychta@gmail.com>

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
#include "lib/printx.h"
#include "kernel/process.h"
#include "lib/conv.h"
#include "kernel/kwrapper.h"
#include "dnx/misc.h"
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/*==============================================================================
  Local macros
==============================================================================*/
#define TO_STR(str)                     #str
#define NUMBER_TO_STR(val)              TO_STR(val)

/*==============================================================================
  Local object types
==============================================================================*/

/*==============================================================================
  Local function prototypes
==============================================================================*/

/*==============================================================================
  Local objects
==============================================================================*/
#if ((CONFIG_SYSTEM_MSG_ENABLE > 0) && (CONFIG_PRINTF_ENABLE > 0))
static FILE *sys_printk_file;
#endif

#if (CONFIG_PRINTF_ENABLE > 0)
/** buffer used to store converted time to string */
static char timestr[32];

/** days of week */
static const char *week_day_abbr[] = {
        "Sun", "Mon", "Tue",
        "Wed", "Thu", "Fri",
        "Sat"
};

static const char *week_day_full[] = {
        "Sunday",    "Monday", "Tuesday",
        "Wednesday", "Thrusday",
        "Friday",    "Saturday"
};

/** month names */
static const char *month_abbr[] = {
        "Jan", "Feb", "Mar",
        "Apr", "May", "Jun",
        "Jul", "Aug", "Sep",
        "Oct", "Nov", "Dec"
};

static const char *month_full[] = {
        "January", "February", "March",
        "April",   "May",      "June",
        "July",    "August",   "September",
        "October", "November", "December"
};

#endif

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Function definitions
==============================================================================*/

//==============================================================================
/**
 * @brief  Reverse selected buffer
 * @param  begin        beggining of the buffer
 * @param  end          end of the buffer
 * @return None
 */
//==============================================================================
#if (CONFIG_PRINTF_ENABLE > 0)
static void reverse_buffer(char *begin, char *end)
{
        while (end > begin) {
                char tmp = *end;
                *end--   = *begin;
                *begin++ = tmp;
        }
}
#endif

//==============================================================================
/**
 * @brief Function convert value to the character
 *
 * @param  val          converted value
 * @param *buf          result buffer
 * @param  base         conversion base
 * @param  usign_val    unsigned value conversion
 * @param  zeros_req    zeros required (added zeros to conversion)
 *
 * @return pointer in the buffer
 */
//==============================================================================
#if (CONFIG_PRINTF_ENABLE > 0)
static char *itoa(i32_t val, char *buf, u8_t base, bool usign_val, u8_t zeros_req)
{
        static const char digits[]  = "0123456789ABCDEF";
        char             *buf_start = buf;

        if (base >= 2 && base <= 16) {
                bool sign     = val < 0 && !usign_val;
                u8_t zero_cnt = 0;

                if (base == 10 && sign) {
                        val = -val;
                }

                i32_t quot, rem;
                do {
                        if (usign_val) {
                                quot = static_cast(u32_t, val) / base;
                                rem  = static_cast(u32_t, val) % base;
                        } else {
                                quot = val / base;
                                rem  = val % base;
                        }

                        *buf++ = digits[rem];
                        zero_cnt++;

                } while ((val = quot));

                while (zeros_req > zero_cnt) {
                        *buf++ = '0';
                        zero_cnt++;
                }

                if (sign) {
                        *buf++ = '-';
                }

                reverse_buffer(buf_start, buf - 1);
        }

        *buf = '\0';
        return buf_start;
}
#endif

//==============================================================================
/**
 * @brief Function convert double to string
 *
 * @note used software: nickgsuperstar@gmail.com & nickg@client9.com
 *                      https://code.google.com/p/stringencoders/
 *
 * @param[in]   value           input value
 * @param[out] *str             string - result
 * @param[in]   prec            precision
 * @param[in]   n               buffer size
 *
 * @return number of characters
 */
//==============================================================================
#if (CONFIG_PRINTF_ENABLE > 0)
static int dtoa(double value, char* str, int prec, int n)
{
        const double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000,
                                10000000, 100000000, 1000000000};

        int   conv = 0;
        char *wstr = str;

        void push_char(const char c)
        {
                if (conv < n) {
                        *wstr++ = c;
                        conv++;
                }
        }

        /*
         * Hacky test for NaN
         * under -fast-math this won't work, but then you also won't
         * have correct nan values anyways.  The alternative is
         * to link with libmath (bad) or hack IEEE double bits (bad)
         */
        if (!(value == value)) {
                push_char('n');
                push_char('a');
                push_char('n');
                return conv;
        }

        /* if input is larger than thres_max, revert to exponential */
        const double thres_max = reinterpret_cast(double, 0x7FFFFFFF);

        double diff  = 0.0;

        if (prec < 0) {
                prec = 0;
        } else if (prec > 9) {
                /* precision of >= 10 can lead to overflow errors */
                prec = 9;
        }

        /* we'll work in positive values and deal with the negative sign issue later */
        int neg = 0;
        if (value < 0) {
                neg = 1;
                value = -value;
        }

        int    whole = (int) value;
        double tmp   = (value - whole) * pow10[prec];
        u32_t  frac  = (u32_t)tmp;

        diff = tmp - frac;

        if (diff > 0.5) {
                ++frac;

                /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
                if (frac >= pow10[prec]) {
                        frac = 0;
                        ++whole;
                }
        } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
                /* if halfway, round up if odd, OR if last digit is 0.  That last part is strange */
                ++frac;
        }

        /* for very large numbers switch back to native sprintf for exponentials.
         anyone want to write code to replace this? */
        /*
         normal printf behavior is to print EVERY whole number digit
         which can be 100s of characters overflowing your buffers == bad
         */
        if (value > thres_max) {
                push_char('E');
                push_char('?');
                return conv;
        }

        if (prec == 0) {
                diff = value - whole;
                if (diff > 0.5) {
                        /* greater than 0.5, round up, e.g. 1.6 -> 2 */
                        ++whole;
                } else if (diff == 0.5 && (whole & 1)) {
                        /* exactly 0.5 and ODD, then round up */
                        /* 1.5 -> 2, but 2.5 -> 2 */
                        ++whole;
                }
        } else {
                int count = prec;

                /* now do fractional part, as an unsigned number */
                do {
                        --count;
                        push_char((char) (48 + (frac % 10)));
                } while (frac /= 10);

                /* add extra 0s */
                while (count-- > 0) {
                        push_char('0');
                }

                /* add decimal */
                push_char('.');
        }

        /* Do whole part. Take care of sign. Conversion. Number is reversed. */
        do {
                push_char((char) (48 + (whole % 10)));
        } while (whole /= 10);

        if (neg) {
                push_char('-');
        }

        reverse_buffer(str, wstr - 1);

        return conv;
}
#endif

//==============================================================================
/**
 * @brief Enable printk functionality
 *
 * @param filename      path to file used to write kernel log
 */
//==============================================================================
void _printk_enable(char *filename)
{
#if ((CONFIG_SYSTEM_MSG_ENABLE > 0) && (CONFIG_PRINTF_ENABLE > 0))
        /* close file if opened */
        if (sys_printk_file) {
                _vfs_fclose(sys_printk_file);
                sys_printk_file = NULL;
        }

        /* open new file */
        if (sys_printk_file == NULL) {
                sys_printk_file = _vfs_fopen(filename, "w");
        }
#else
        UNUSED_ARG(filename);
#endif
}

//==============================================================================
/**
 * @brief Disable printk functionality
 */
//==============================================================================
void _printk_disable(void)
{
#if ((CONFIG_SYSTEM_MSG_ENABLE > 0) && (CONFIG_PRINTF_ENABLE > 0))
        if (sys_printk_file) {
                _vfs_fclose(sys_printk_file);
                sys_printk_file = NULL;
        }
#endif
}

//==============================================================================
/**
 * @brief Function send kernel message on terminal
 *
 * @param *format             formated text
 * @param ...                 format arguments
 */
//==============================================================================
void _printk(const char *format, ...)
{
#if ((CONFIG_SYSTEM_MSG_ENABLE > 0) && (CONFIG_PRINTF_ENABLE > 0))
        va_list args;

        if (sys_printk_file) {
                va_start(args, format);
                int size = _vsnprintf(NULL, 0, format, args) + 1;
                va_end(args);

                char *buffer = _sysm_syscalloc(size, sizeof(char));
                if (buffer) {
                        va_start(args, format);
                        int n = _vsnprintf(buffer, size, format, args);
                        va_end(args);

                        _vfs_fwrite(buffer, sizeof(char), n, sys_printk_file);

                        if (LAST_CHARACTER(buffer) != '\n') {
                                _vfs_fflush(sys_printk_file);
                        }

                        _sysm_sysfree(buffer);
                }
        }
#else
        UNUSED_ARG(format);
#endif
}

//==============================================================================
/**
 * @brief Function put character into file
 *
 * @param  c                   character
 * @param *stream              file
 *
 * @retval c if OK otherwise EOF
 */
//==============================================================================
int _fputc(int c, FILE *stream)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (stream) {
                char ch = (char)c;
                if (_vfs_fwrite(&ch, sizeof(char), 1, stream) == 1) {
                        return c;
                }
        }
#else
        UNUSED_ARG(c);
        UNUSED_ARG(stream);
#endif
        return EOF;
}

//==============================================================================
/**
 * @brief Function puts string to selected file (fputs & puts)
 *
 * @param[in] *s        string
 * @param[in] *file     file
 * @param[in]  puts     puts functionality (true: add \n at the end of string)
 *
 * @return number of characters written to the stream
 */
//==============================================================================
int _f_puts(const char *s, FILE *file, bool puts)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (file) {
                int n = _vfs_fwrite(s, sizeof(char), strlen(s), file);

                if (puts) {
                        n += _vfs_fwrite("\n", sizeof(char), 1, file);
                }

                if (n != 0)
                        return n;
        }
#else
        UNUSED_ARG(s);
        UNUSED_ARG(file);
        UNUSED_ARG(puts);
#endif
        return EOF;
}

//==============================================================================
/**
 * @brief Function get character from file
 *
 * @param *stream            source file
 *
 * @retval character
 */
//==============================================================================
int _getc(FILE *stream)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (!stream) {
                return EOF;
        }

        int chr = 0;
        if (_vfs_fread(&chr, sizeof(char), 1, stream) != 0) {
                if (_vfs_ferror(stream) || _vfs_feof(stream)) {
                        return EOF;
                }
        } else {
                return EOF;
        }

        return chr;
#else
        UNUSED_ARG(stream);
        return EOF;
#endif
}

//==============================================================================
/**
 * @brief Function gets number of bytes from file
 *
 * @param[out] *str          buffer with string
 * @param[in]   size         buffer size
 * @param[in]  *stream       source stream
 *
 * @retval NULL if error, otherwise pointer to str
 */
//==============================================================================
char *_fgets(char *str, int size, FILE *stream)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (!str || size < 2 || !stream) {
                return NULL;
        }

        struct stat file_stat;
        if (_vfs_fstat(stream, &file_stat) == 0) {
                if (file_stat.st_type == FILE_TYPE_PIPE || file_stat.st_type == FILE_TYPE_DRV) {
                        int n = 0;
                        for (int i = 0; i < size - 1; i++) {
                                int m = _vfs_fread(str + i, sizeof(char), 1, stream);
                                if (m == 0) {
                                        str[i] = '\0';
                                        return str;
                                } else {
                                        n += m;
                                }

                                if (_vfs_ferror(stream) || _vfs_feof(stream)) {
                                        if (n == 0) {
                                                return NULL;
                                        } else {
                                                str[i + 1] = '\0';
                                                return str;
                                        }
                                }

                                if (str[i] == '\n') {
                                        str[i + 1] = '\0';
                                        break;
                                }
                        }

                        return str;
                } else {
                        u64_t fpos = _vfs_ftell(stream);

                        int n;
                        while ((n = _vfs_fread(str, sizeof(char), size - 1, stream)) == 0) {
                                if (_vfs_ferror(stream) || _vfs_feof(stream)) {
                                        return NULL;
                                }
                        }

                        char *end;
                        if ((end = strchr(str, '\n'))) {
                                end++;
                                *end = '\0';
                        } else {
                                str[n] = '\0';
                        }

                        int len = strlen(str);

                        if (len != 0 && len < n && _vfs_feof(stream))
                                _vfs_clearerr(stream);

                        if (len == 0)
                                len = 1;

                        _vfs_fseek(stream, fpos + len, SEEK_SET);

                        return str;
                }
        }
#else
        UNUSED_ARG(str);
        UNUSED_ARG(size);
        UNUSED_ARG(stream);
#endif
        return NULL;
}

//==============================================================================
/**
 * @brief Function send to buffer formated output string
 *
 * @param *bfr                output buffer
 * @param  size               buffer size
 * @param *format             formated text
 * @param  ...                format arguments
 *
 * @retval number of written characters
 */
//==============================================================================
int _snprintf(char *bfr, size_t size, const char *format, ...)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        va_list args;
        int n = 0;

        if (bfr && size && format) {
                va_start(args, format);
                n = _vsnprintf(bfr, size, format, args);
                va_end(args);
        }

        return n;
#else
        UNUSED_ARG(bfr);
        UNUSED_ARG(size);
        UNUSED_ARG(format);
        return 0;
#endif
}

//==============================================================================
/**
 * @brief Function write to file formatted string
 *
 * @param *file               file
 * @param *format             formated text
 * @param ...                 format arguments
 *
 * @retval number of written characters
 */
//==============================================================================
int _fprintf(FILE *file, const char *format, ...)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        int n = 0;

        if (file && format) {
                va_list args;
                va_start(args, format);
                n = _vfprintf(file, format, args);
                va_end(args);
        }

        return n;
#else
        UNUSED_ARG(file);
        UNUSED_ARG(format);
        return 0;
#endif
}

//==============================================================================
/**
 * @brief Function write to file formatted string
 *
 * @param file                file
 * @param format              formated text
 * @param arg                 arguments
 *
 * @retval number of written characters
 */
//==============================================================================
int _vfprintf(FILE *file, const char *format, va_list arg)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        int n = 0;

        if (file && format) {
                va_list carg;
                va_copy(carg, arg);
                u32_t size = _vsnprintf(NULL, 0, format, carg) + 1;

                char *str = _sysm_syscalloc(1, size);
                if (str) {
                        n = _vsnprintf(str, size, format, arg);
                        _vfs_fwrite(str, sizeof(char), n, file);
                        _sysm_sysfree(str);
                }
        }

        return n;
#else
        UNUSED_ARG(file);
        UNUSED_ARG(format);
        UNUSED_ARG(arg);
        return 0;
#endif
}

//==============================================================================
/**
 * @brief Function returns error string
 *
 * @param errnum        error number
 *
 * @return error number string
 */
//==============================================================================
const char *_strerror(int errnum)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        static const char *errstr[] = {
#if (CONFIG_ERRNO_STRING_LEN == 0)
                /* empty */
#elif (CONFIG_ERRNO_STRING_LEN == 1)
                [ESUCC       ] = NUMBER_TO_STR(ESUCC),
                [EPERM       ] = NUMBER_TO_STR(EPERM),
                [ENOENT      ] = NUMBER_TO_STR(ENOENT),
                [ESRCH       ] = NUMBER_TO_STR(ESRCH),
                [EIO         ] = NUMBER_TO_STR(EIO),
                [ENXIO       ] = NUMBER_TO_STR(ENXIO),
                [E2BIG       ] = NUMBER_TO_STR(E2BIG),
                [ENOEXEC     ] = NUMBER_TO_STR(ENOEXEC),
                [EAGAIN      ] = NUMBER_TO_STR(EAGAIN),
                [ENOMEM      ] = NUMBER_TO_STR(ENOMEM),
                [EACCES      ] = NUMBER_TO_STR(EACCES),
                [EFAULT      ] = NUMBER_TO_STR(EFAULT),
                [EBUSY       ] = NUMBER_TO_STR(EBUSY),
                [EEXIST      ] = NUMBER_TO_STR(EEXIST),
                [ENODEV      ] = NUMBER_TO_STR(ENODEV),
                [ENOTDIR     ] = NUMBER_TO_STR(ENOTDIR),
                [EISDIR      ] = NUMBER_TO_STR(EISDIR),
                [EINVAL      ] = NUMBER_TO_STR(EINVAL),
                [EMFILE      ] = NUMBER_TO_STR(EMFILE),
                [EFBIG       ] = NUMBER_TO_STR(EFBIG),
                [ENOSPC      ] = NUMBER_TO_STR(ENOSPC),
                [ESPIPE      ] = NUMBER_TO_STR(ESPIPE),
                [EROFS       ] = NUMBER_TO_STR(EROFS),
                [EDOM        ] = NUMBER_TO_STR(EDOM),
                [ERANGE      ] = NUMBER_TO_STR(ERANGE),
                [EILSEQ      ] = NUMBER_TO_STR(EILSEQ),
                [ENAMETOOLONG] = NUMBER_TO_STR(ENAMETOOLONG),
                [ENOTEMPTY   ] = NUMBER_TO_STR(ENOTEMPTY),
                [EBADRQC     ] = NUMBER_TO_STR(EBADRQC),
                [ETIME       ] = NUMBER_TO_STR(ETIME),
                [ENONET      ] = NUMBER_TO_STR(ENONET),
                [EUSERS      ] = NUMBER_TO_STR(EUSERS),
                [EADDRINUSE  ] = NUMBER_TO_STR(EADDRINUSE),
                [ENOMEDIUM   ] = NUMBER_TO_STR(ENOMEDIUM),
                [EMEDIUMTYPE ] = NUMBER_TO_STR(EMEDIUMTYPE),
                [ECANCELED   ] = NUMBER_TO_STR(ECANCELED),
                [ENOTSUP     ] = NUMBER_TO_STR(ENOTSUP)
#elif (CONFIG_ERRNO_STRING_LEN == 2)
                [ESUCC       ] = TO_STR(ESUCC),
                [EPERM       ] = TO_STR(EPERM),
                [ENOENT      ] = TO_STR(ENOENT),
                [ESRCH       ] = TO_STR(ESRCH),
                [EIO         ] = TO_STR(EIO),
                [ENXIO       ] = TO_STR(ENXIO),
                [E2BIG       ] = TO_STR(E2BIG),
                [ENOEXEC     ] = TO_STR(ENOEXEC),
                [EAGAIN      ] = TO_STR(EAGAIN),
                [ENOMEM      ] = TO_STR(ENOMEM),
                [EACCES      ] = TO_STR(EACCES),
                [EFAULT      ] = TO_STR(EFAULT),
                [EBUSY       ] = TO_STR(EBUSY),
                [EEXIST      ] = TO_STR(EEXIST),
                [ENODEV      ] = TO_STR(ENODEV),
                [ENOTDIR     ] = TO_STR(ENOTDIR),
                [EISDIR      ] = TO_STR(EISDIR),
                [EINVAL      ] = TO_STR(EINVAL),
                [EMFILE      ] = TO_STR(EMFILE),
                [EFBIG       ] = TO_STR(EFBIG),
                [ENOSPC      ] = TO_STR(ENOSPC),
                [ESPIPE      ] = TO_STR(ESPIPE),
                [EROFS       ] = TO_STR(EROFS),
                [EDOM        ] = TO_STR(EDOM),
                [ERANGE      ] = TO_STR(ERANGE),
                [EILSEQ      ] = TO_STR(EILSEQ),
                [ENAMETOOLONG] = TO_STR(ENAMETOOLONG),
                [ENOTEMPTY   ] = TO_STR(ENOTEMPTY),
                [EBADRQC     ] = TO_STR(EBADRQC),
                [ETIME       ] = TO_STR(ETIME),
                [ENONET      ] = TO_STR(ENONET),
                [EUSERS      ] = TO_STR(EUSERS),
                [EADDRINUSE  ] = TO_STR(EADDRINUSE),
                [ENOMEDIUM   ] = TO_STR(ENOMEDIUM),
                [EMEDIUMTYPE ] = TO_STR(EMEDIUMTYPE),
                [ECANCELED   ] = TO_STR(ECANCELED),
                [ENOTSUP     ] = TO_STR(ENOTSUP)
#elif (CONFIG_ERRNO_STRING_LEN == 3)
                [ESUCC       ] = "Success",
                [EPERM       ] = "Operation not permitted",
                [ENOENT      ] = "No such file or directory",
                [ESRCH       ] = "No such process",
                [EIO         ] = "I/O error",
                [ENXIO       ] = "No such device or address",
                [E2BIG       ] = "Argument list too long",
                [ENOEXEC     ] = "Exec format error",
                [EAGAIN      ] = "Try again",
                [ENOMEM      ] = "Out of memory",
                [EACCES      ] = "Permission denied",
                [EFAULT      ] = "Bad address",
                [EBUSY       ] = "Device or resource busy",
                [EEXIST      ] = "File exists",
                [ENODEV      ] = "No such device",
                [ENOTDIR     ] = "Not a directory",
                [EISDIR      ] = "Is a directory",
                [EINVAL      ] = "Invalid argument",
                [EMFILE      ] = "Too many open files",
                [EFBIG       ] = "File too large",
                [ENOSPC      ] = "No space left on device",
                [ESPIPE      ] = "Illegal seek",
                [EROFS       ] = "Read-only file system",
                [EDOM        ] = "Math argument out of domain of function",
                [ERANGE      ] = "Math result not representable",
                [EILSEQ      ] = "Illegal byte sequence",
                [ENAMETOOLONG] = "File name too long",
                [ENOTEMPTY   ] = "Directory not empty",
                [EBADRQC     ] = "Invalid request code",
                [ETIME       ] = "Timer expired",
                [ENONET      ] = "Machine is not on the network",
                [EUSERS      ] = "Too many users",
                [EADDRINUSE  ] = "Address already in use",
                [ENOMEDIUM   ] = "No medium found",
                [EMEDIUMTYPE ] = "Wrong medium type",
                [ECANCELED   ] = "Operation Canceled",
                [ENOTSUP     ] = "Not supported"
#else
#error "CONFIG_ERRNO_STRING_LEN should be in range 0 - 3!"
#endif
        };

        if (CONFIG_ERRNO_STRING_LEN == 0) {
                return "";
        } else if (errnum < _ENUMBER) {
                return errstr[errnum];
        } else {
                return "Unknown error";
        }
#else
        (void) errnum;
        return "";
#endif
}

//==============================================================================
/**
 * @brief Function prints error string
 *
 * @param str           string to print or NULL
 */
//==============================================================================
void _perror(const char *str)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (str) {
                _fprintf(stderr, "%s: %s\n", str, _strerror(errno));
        } else {
                _fprintf(stderr, "%s\n", _strerror(errno));
        }
#else
        (void) str;
#endif
}

//==============================================================================
/**
 * @brief  Convert time value (Epoch) to human readable string: Www Mmm dd hh:mm:ss zzzzz yyyy
 *
 * @param  timer        UNIX time value (can be NULL)
 * @param  tm           time structure (can be NULL)
 * @param  buf          buffer where string is filled (at least 32 bytes)
 *
 * @return Pointer to statically allocated string buffer. This function is not
 *         thread safe.
 */
//==============================================================================
char *_ctime_r(const time_t *timer, const struct tm *tm, char *buf)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        if (timer || tm) {
                if (buf == NULL) {
                        buf = timestr;
                }

                struct tm t;

                if (!timer) {
                        t = *tm;
                } else {
                        _localtime_r(timer, &t);
                }

                _strftime(buf, sizeof(timestr), "%a %b %d %X %z %Y%n", &t);

                return buf;
        } else {
                return NULL;
        }
#else
        UNUSED_ARG(timer);
        UNUSED_ARG(tm);
        UNUSED_ARG(buf);
        return NULL;
#endif
}

//==============================================================================
/**
 * @brief  Format time as string
 *
 * Copies into ptr the content of format, expanding its format specifiers into
 * the corresponding values that represent the time described in timeptr, with
 * a limit of maxsize characters.
 *
 * @param  buf          Pointer to the destination array where the resulting
 *                      C string is copied.
 * @param  size         Maximum number of characters to be copied to buf,
 *                      including the terminating null-character.
 * @param  format       C string containing any combination of regular characters
 *                      and special format specifiers. These format specifiers
 *                      are replaced by the function to the corresponding values
 *                      to represent the time specified in timeptr.
 * @param  timeptr      Pointer to a tm structure that contains a calendar time
 *                      broken down into its components (see struct tm).
 *
 * @return If the length of the resulting C string, including the terminating
 *         null-character, doesn't exceed maxsize, the function returns the
 *         total number of characters copied to buf (not including the terminating
 *         null-character).
 *         Otherwise, it returns zero, and the contents of the array pointed by
 *         buf are indeterminate.
 *
 * @note Supported flags:
 *       % - % character
 *       n - new line
 *       H - Hour in 24h format (00-23)
 *       I - Hour in 12h format (01-12)
 *       M - Minute (00-59)
 *       S - Second (00-61)
 *       A - Full weekday name
 *       a - Abbreviated weekday name
 *       B - Full month name
 *       b - Abbreviated month name
 *       h - Abbreviated month name
 *       C - Year divided by 100 and truncated to integer (00-99) (century)
 *       y - Year, last two digits (00-99)
 *       Y - Year
 *       d - Day of the month, zero-padded (01-31)
 *       p - AM or PM designation
 *       j - Day of the year (001-366)
 *       m - Month as a decimal number (01-12)
 *       X - Time representation                                14:55:02
 *       F - Short YYYY-MM-DD date, equivalent to %Y-%m-%d      2001-08-23
 *       D - Short MM/DD/YY date, equivalent to %m/%d/%y        08/23/01
 *       x - Short MM/DD/YY date, equivalent to %m/%d/%y        08/23/01
 *       z - ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100) +0100, -1230
 */
//==============================================================================
size_t _strftime(char *buf, size_t size, const char *format, const struct tm *timeptr)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        size_t n = 0;

        if (buf && size && format && timeptr) {
                size--;

                bool _do = true;
                char ch  = '\0';

                void put_ch(const char c)
                {
                        *buf++ = c;
                        _do    = (--size != 0);
                        n++;
                }

                bool get_fch()
                {
                        ch  = *format++;
                        _do = (ch != '\0');
                        return _do;
                }

                while (_do && size) {
                        if (!get_fch())
                                break;

                        if (ch == '%') {
                                if (!get_fch())
                                        break;

                                size_t m = 0;

                                switch (ch) {
                                case '%':
                                        put_ch(ch);
                                        break;

                                case 'n':
                                        put_ch('\n');
                                        break;

                                case 'H':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_hour);
                                        break;

                                case 'I':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_hour > 12 ? timeptr->tm_hour - 12 : timeptr->tm_hour);
                                        break;

                                case 'M':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_min);
                                        break;

                                case 'S':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_sec);
                                        break;

                                case 'a':
                                        m = _snprintf(buf, size, "%s", week_day_abbr[timeptr->tm_wday]);
                                        break;

                                case 'A':
                                        m = _snprintf(buf, size, "%s", week_day_full[timeptr->tm_wday]);
                                        break;

                                case 'b':
                                case 'h':
                                        m = _snprintf(buf, size, "%s", month_abbr[timeptr->tm_mon]);
                                        break;

                                case 'B':
                                        m = _snprintf(buf, size, "%s", month_full[timeptr->tm_mon]);
                                        break;

                                case 'C':
                                        m = _snprintf(buf, size, "%02d", (timeptr->tm_year + 1900) / 100);
                                        break;

                                case 'y':
                                        m = _snprintf(buf, size, "%02d", (timeptr->tm_year + 1900) % 100);
                                        break;

                                case 'Y':
                                        m = _snprintf(buf, size, "%d", timeptr->tm_year + 1900);
                                        break;

                                case 'd':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_mday);
                                        break;

                                case 'p':
                                        m = _snprintf(buf, size, "%s", timeptr->tm_hour > 12 ? "PM" : "AM");
                                        break;

                                case 'j':
                                        m = _snprintf(buf, size, "%03d", timeptr->tm_yday + 1);
                                        break;

                                case 'm':
                                        m = _snprintf(buf, size, "%02d", timeptr->tm_mon + 1);
                                        break;

                                case 'X':
                                        m = _snprintf(buf, size, "%02d:%02d:%02d", timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
                                        break;

                                case 'F':
                                        m = _snprintf(buf, size, "%d-%02d-%02d", timeptr->tm_year+1900, timeptr->tm_mon+1, timeptr->tm_mday);
                                        break;

                                case 'z': {
                                        i32_t timeoff = timeptr->tm_isutc ? 0 : _ltimeoff;
                                        m = _snprintf(buf, size, "%c%02d%02d",
                                                      (timeoff < 0 ? '-':'+'),
                                                      (timeoff < 0 ? -timeoff : timeoff) / 3600,
                                                      (timeoff < 0 ? -timeoff : timeoff) / 60 % 60);
                                        break;
                                }

                                case 'D':
                                case 'x':
                                        m = _snprintf(buf, size, "%02d/%02d/%02d", timeptr->tm_mon+1, timeptr->tm_mday, (timeptr->tm_year+1900) % 100);
                                        break;
                                }

                                n    += m;
                                buf  += m;
                                size -= m;

                        } else {
                                put_ch(ch);
                        }
                }

                *buf = '\0';
        }

        return n;
#else
        UNUSED_ARG(buf);
        UNUSED_ARG(size);
        UNUSED_ARG(format);
        UNUSED_ARG(timeptr);
        return 0;
#endif
}

//==============================================================================
/**
 * @brief Function convert arguments to stream
 *
 * @param[in] *buf           buffer for stream
 * @param[in]  size          buffer size
 * @param[in] *format        message format
 * @param[in]  arg           argument list
 *
 * @return number of printed characters
 *
 * Supported flags:
 *   %%         - print % character
 *                printf("%%"); => %
 *
 *   %c         - print selected character (the \0 character is skipped)
 *                printf("_%c_", 'x');  => _x_
 *                printf("_%c_", '\0'); => __
 *
 *   %s         - print selected string
 *                printf("%s", "Foobar"); => Foobar
 *
 *   %.*s       - print selected string but only the length passed by argument
 *                printf("%.*s\n", 3, "Foobar"); => Foo
 *
 *   %.ns       - print selected string but only the n length
 *                printf("%.3s\n", "Foobar"); => Foo
 *
 *   %d, %i     - print decimal integer values
 *                printf("%d, %i", -5, 10); => -5, 10
 *
 *   %u         - print unsigned decimal integer values
 *                printf("%u, %u", -1, 10); => 4294967295, 10
 *
 *   %x, %X     - print hexadecimal values ('x' for lower characters, 'X' for upper characters)
 *                printf("0x%x, 0x%X", 0x5A, 0xfa); => 0x5a, 0xFA
 *
 *   %0x?       - print decimal (d, i, u) or hex (x, X) values with leading zeros.
 *                The number of characters (at least) is determined by x. The ?
 *                means d, i, u, x, or X value representations.
 *                printf("0x02X, 0x03X", 0x5, 0x1F43); => 0x05, 0x1F43
 *
 *   %f         - print float number. Note: make sure that input value is the float!
 *                printf("Foobar: %f", 1.0); => Foobar: 1.000000
 *
 *   %l?        - print long long values, where ? means d, i, u, x, or X.
 *                NOTE: not supported
 *
 *   %p         - print pointer
 *                printf("Pointer: %p", main); => Pointer: 0x4028B4
 */
//==============================================================================
int _vsnprintf(char *buf, size_t size, const char *format, va_list arg)
{
#if (CONFIG_PRINTF_ENABLE > 0)
        char   chr;
        int    arg_size;
        size_t scan_len     = 1;
        bool   leading_zero = false;
        bool   loop_break   = false;
        bool   long_long    = false;
        bool   arg_size_str = false;

        /// @brief  Function break loop
        /// @param  None
        /// @return None
        inline void break_loop()
        {
                loop_break = true;
        }

        /// @brief  Put character to the buffer
        /// @param  c    character to put
        /// @return On success true is returned, otherwise false and loop is break
        bool put_char(const char c)
        {
                if (buf) {
                        if (scan_len < size) {
                                *buf++ = c;
                        } else {
                                break_loop();
                                return false;
                        }
                }

                scan_len++;
                return true;
        }

        /// @brief  Get char from format string
        /// @param  None
        /// @return Load next character from format string
        bool get_format_char()
        {
                chr = *format++;

                if (chr == '\0') {
                        break_loop();
                        return false;
                } else {
                        return true;
                }
        }

        /// @brief  Analyze modifiers (%0, %.*, %<num>, %xxlx)
        /// @param  None
        /// @return If modifiers are set or not then true is returned. On error false.
        bool check_modifiers()
        {
                arg_size     = -1;
                leading_zero = false;
                arg_size_str = false;

                // check leading zero enable
                if (chr == '0') {
                        leading_zero = true;
                        if (!get_format_char()) {
                                return false;
                        }
                }

                // check argument size modifier
                if (chr == '.') {
                        if (!get_format_char()) {
                                return false;
                        }

                        if (chr == '*') {
                                arg_size     = va_arg(arg, int);
                                arg_size_str = true;

                                if (!get_format_char()) {
                                        return false;
                                }
                        } else if (chr >= '0' && chr <= '9') {
                                arg_size     = 0;
                                arg_size_str = true;
                                while (chr >= '0' && chr <= '9') {
                                        arg_size *= 10;
                                        arg_size += chr - '0';

                                        if (!get_format_char()) {
                                                return false;
                                        }
                                }
                        } else {
                                break_loop();
                                return false;
                        }

                // check numeric size modifier
                } else {
                        arg_size = 0;
                        while (chr >= '0' && chr <= '9') {
                                arg_size *= 10;
                                arg_size += chr - '0';

                                if (!get_format_char()) {
                                        return false;
                                }
                        }
                }

                // check long long values
                if (chr == 'l') {
                        long_long = true;

                        if (!get_format_char()) {
                                return false;
                        }
                }

                return true;
        }

        /// @brief  Put percent or character
        /// @param  None
        /// @return If format was found then true is returned, otherwise false.
        bool put_percent_or_char()
        {
                if (chr == '%' || chr == 'c') {
                        if (chr == 'c') {
                                chr = va_arg(arg, int);
                                if (chr != '\0') {
                                        put_char(chr);
                                }
                        } else {
                                put_char(chr);
                        }

                        return true;
                }

                return false;
        }

        /// @brief  Put string
        /// @param  None
        /// @return If format was found then true is returned, otherwise false.
        bool put_string()
        {
                if (chr == 's') {
                        char *str = va_arg(arg, char*);
                        if (!str) {
                                str = "";
                        }

                        if (arg_size == 0 && arg_size_str == true) {
                                return true;
                        }

                        if (arg_size <= 0 || arg_size_str == false) {
                                arg_size = UINT16_MAX;
                        }

                        while ((chr = *str++) && arg_size--) {
                                if (chr == '\0') {
                                        break;
                                }

                                if (!put_char(chr)) {
                                        break;
                                }
                        }

                        return true;
                } else {
                        return false;
                }
        }

        /// @brief  Put integer
        /// @param  None
        /// @return If format was found then true is returned, otherwise false.
        bool put_integer()
        {
                if (chr == 'd' || chr == 'u' || chr == 'i' || chr == 'x' || chr == 'X') {
                        char result[16];
                        bool upper  = chr == 'X';
                        bool spaces = false;
                        bool expand = false;
                        bool unsign = chr == 'u' || chr == 'x' || chr =='X';
                        int  base   = chr == 'x' || chr == 'X' ? 16 : 10;

                        if (arg_size == -1 && leading_zero == false) {
                                expand = false;
                                spaces = false;

                        } else if (arg_size == -1 && leading_zero == true) {
                                expand = false;
                                spaces = false;

                        } else if (arg_size >= 0 && leading_zero == false) {
                                expand = true;
                                spaces = true;

                        } else if (arg_size >= 0 && leading_zero == true) {
                                expand = true;
                                spaces = false;
                        }

                        if (arg_size > static_cast(int, sizeof(result) - 1)) {
                                arg_size = sizeof(result) - 1;

                        }

                        /* NOTE: 64-bit integers are not supported */
                        i32_t val;
                        if (long_long) {
                                val = va_arg(arg, i32_t);
                        } else {
                                val = va_arg(arg, i32_t);
                        }

                        char *result_ptr = itoa(val, result, base, unsign, expand ? arg_size : 0);

                        if (static_cast(int, strlen(result_ptr)) > arg_size) {
                                arg_size = strlen(result_ptr);
                        }

                        while ((chr = *result_ptr++) && arg_size--) {
                                if (spaces && chr == '0' && arg_size > 1) {
                                        chr = ' ';
                                } else {
                                        spaces = false;
                                }

                                if (upper) {
                                        chr = toupper(static_cast(int, chr));
                                } else {
                                        chr = tolower(static_cast(int, chr));
                                }

                                if (!put_char(chr)) {
                                        break;
                                }
                        }

                        return true;
                } else {
                        return false;
                }
        }

        /// @brief  Put float value
        /// @param  None
        /// @return If format was found then true is returned, otherwise false.
        bool put_float()
        {
                if (chr == 'f' || chr == 'F') {
                        char result[24];
                        int  len = dtoa(va_arg(arg, double), result, 6, sizeof(result));

                        for (int i = 0; i < len; i++) {
                                if (!put_char(result[i])) {
                                        break;
                                }
                        }

                        return true;
                } else {
                        return false;
                }
        }

        /// @brief  Put pointer value
        /// @param  None
        /// @return If format was found then true is returned, otherwise false.
        bool put_pointer()
        {
                if (chr == 'p') {
                        int   val = va_arg(arg, int);
                        char  result[16];
                        char *result_ptr = itoa(val, result, 16, true, 0);

                        if (!put_char('0'))
                                return true;

                        if (!put_char('x'))
                                return true;

                        while ((chr = *result_ptr++)) {
                                if (!put_char(chr)) {
                                        break;
                                }
                        }

                        return true;
                } else {
                        return false;
                }
        }

        // read characters from format string
        while (loop_break == false) {

                if (!get_format_char())
                        continue;

                if (chr != '%') {
                        put_char(chr);
                        continue;

                } else {
                        if (!get_format_char())
                                continue;

                        if (!check_modifiers())
                                continue;

                        if (put_percent_or_char())
                                continue;

                        if (put_string())
                                continue;

                        if (put_integer())
                                continue;

                        if (put_float())
                                continue;

                        if (put_pointer())
                                continue;
                }
        }

        if (buf)
                *buf = 0;

        return (scan_len - 1);
#else
        UNUSED_ARG(buf);
        UNUSED_ARG(size);
        UNUSED_ARG(format);
        UNUSED_ARG(arg);
        return 0;
#endif
}

/*==============================================================================
  End of file
==============================================================================*/
