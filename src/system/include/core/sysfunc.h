/*=========================================================================*//**
@file    sysfunc.h

@author  Daniel Zorychta

@brief

@note    Copyright (C) 2015 Daniel Zorychta <daniel.zorychta@gmail.com>

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

#ifndef _SYSFUNC_H_
#define _SYSFUNC_H_

/*==============================================================================
  Include files
==============================================================================*/
#include <sys/types.h>
#include <dnx/misc.h>
#include <errno.h>
#include <stdbool.h>
#include "core/conv.h"
#include "core/llist.h"
#include "core/vfs.h"
#include "core/sysmoni.h"
#include "core/modctrl.h"
#include "core/printx.h"
#include "core/scanx.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
  Exported macros
==============================================================================*/
// list foreach iterator
#define _sys_llist_foreach(type, element, _sys_llist_t__list)\
        _llist_foreach(type, element, _sys_llist_t__list)

// list reversed foreach iterator
#define _sys_llist_foreach_reverse(type, element, _sys_llist_t__list)\
        _llist_foreach_reverse(type, element, _sys_llist_t__list)

// list iterator
#define _sys_llist_foreach_iterator _iterator

/*==============================================================================
  Exported object types
==============================================================================*/

/*==============================================================================
  Exported objects
==============================================================================*/

/*==============================================================================
  Exported functions
==============================================================================*/

/*==============================================================================
  Exported inline functions
==============================================================================*/
//==============================================================================
/**
 * @brief Function convert string to double
 *
 * @param[in]  str            string
 * @param[out] end            the pointer to the character when conversion was finished
 *
 * @return converted value
 */
//==============================================================================
static inline double _sys_strtod(const char *str, char **end)
{
        return _strtod(str, end);
}

//==============================================================================
/**
 * @brief Function convert string to integer
 *
 * @param[in] str       string
 *
 * @return converted value
 */
//==============================================================================
static inline i32_t _sys_atoi(const char *str)
{
        return _atoi(str);
}

//==============================================================================
/**
 * @brief Function convert ASCII to the number
 * When function find any other character than number (depended of actual base)
 * immediately finished operation and return pointer when bad character was
 * found. If base is 0 then function recognize type of number used in string.
 * For hex values "0x" is recognized, for octal values "0" at the beginning of
 * string is recognized, for binary "0b" is recognized, and for decimals values
 * none above.
 *
 * @param[in]  string       string to decode
 * @param[in]  base         decode base
 * @param[out] value        pointer to result
 *
 * @return pointer in string when operation was finished
 */
//==============================================================================
static inline char *_sys_strtoi(const char *string, int base, i32_t *value)
{
        return _strtoi(string, base, value);
}

//==============================================================================
/**
 * @brief Function convert string to float
 *
 * @param[in] str      string
 *
 * @return converted value
 */
//==============================================================================
static inline double _sys_atof(const char *str)
{
        return _atof(str);
}

//==============================================================================
/**
 * @brief  List destructor
 * @param  list         list object
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_delete(llist_t *list)
{
        return _llist_delete(list);
}

//==============================================================================
/**
 * @brief  Checks if list is empty
 * @param  list         list object
 * @return If list is empty then true is returned, otherwise false
 */
//==============================================================================
static inline bool _sys_llist_empty(llist_t *list)
{
        return _llist_empty(list);
}

//==============================================================================
/**
 * @brief  Returns a number of elements of the list
 * @param  list         list object
 * @return Number of elements of the list or -1 on error
 */
//==============================================================================
static inline int _sys_llist_size(llist_t *list)
{
        return _llist_size(list);
}

//==============================================================================
/**
 * @brief  Pushes selected data to the front of the list. Creates a new object
 * @param  list         list object
 * @param  size         data size
 * @param  data         data source
 * @return On success allocated memory pointer is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_push_emplace_front(llist_t *list, size_t size, const void *data)
{
        return _llist_push_emplace_front(list, size, data);
}

//==============================================================================
/**
 * @brief  Pushes selected object to the front of the list
 * @param  list         list object
 * @param  object       object to push
 * @return On success pointer to the object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_push_front(llist_t *list, void *object)
{
        return _llist_push_front(list, object);
}

//==============================================================================
/**
 * @brief  Delete first element of the list. This destroys an element
 * @param  list         list object
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_pop_front(llist_t *list)
{
        return _llist_pop_front(list);
}

//==============================================================================
/**
 * @brief  Push selected data to the back of the list. Creates a new object
 * @param  list         list object
 * @param  size         data size
 * @param  data         data source
 * @return On success allocated memory pointer is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_push_emplace_back(llist_t *list, size_t size, const void *data)
{
        return _llist_push_emplace_back(list, size, data);
}

//==============================================================================
/**
 * @brief  Push selected object to the back of the list
 * @param  list         list object
 * @return On success pointer to the object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_push_back(llist_t *list, void *object)
{
        return _llist_push_back(list, object);
}

//==============================================================================
/**
 * @brief  Delete the last element of the list. This destroys element
 * @param  list         list object
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_pop_back(llist_t *list)
{
        return _llist_pop_back(list);
}

//==============================================================================
/**
 * @brief  Allocate and append data at selected position in the list
 * @param  list         list object
 * @param  position     element position
 * @param  size         element's size
 * @param  data         element's data
 * @return On success pointer to the object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_emplace(llist_t *list, int position, size_t size, const void *data)
{
        return _llist_emplace(list, position, size, data);
}

//==============================================================================
/**
 * @brief  Insert an element to the list
 * @param  list         list object
 * @param  position     position to insert
 * @param  object       object to insert
 * @return On success object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_insert(llist_t *list, int position, void *object)
{
        return _llist_insert(list, position, object);
}

//==============================================================================
/**
 * @brief  Erase selected begin of the list. The element is destroyed
 * @param  list         list object
 * @param  position     position to remove
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_erase(llist_t *list, int position)
{
        return _llist_erase(list, position);
}

//==============================================================================
/**
 * @brief  Return selected begin and remove from the list. The element is not destroyed
 * @param  list         list object
 * @param  position     position to take (unlink)
 * @return On success taken object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_take(llist_t *list, int position)
{
        return _llist_take(list, position);
}

//==============================================================================
/**
 * @brief  Return first begin and remove from the list. The element is not destroyed
 * @param  list         list object
 * @return On success taken object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_take_front(llist_t *list)
{
        return _llist_take_front(list);
}

//==============================================================================
/**
 * @brief  Return last begin and remove from the list. The element is not destroyed
 * @param  list         list object
 * @return On success taken object is returned, otherwise NULL
 */
//==============================================================================
static inline void *_sys_llist_take_back(llist_t *list)
{
        return _llist_take_back(list);
}

//==============================================================================
/**
 * @brief  Clear entire list (objects are destroyed)
 * @param  list         list object
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_clear(llist_t *list)
{
        return _llist_clear(list);
}

//==============================================================================
/**
 * @brief  Swap 2 elements
 * @param  list         list object
 * @param  j            position of element a
 * @param  k            position of element b
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_swap(llist_t *list, int j, int k)
{
        return _llist_swap(list, j, k);
}

//==============================================================================
/**
 * @brief  Sort elements of the list
 * @param  list         list object
 * @return None
 */
//==============================================================================
static inline void _sys_llist_sort(llist_t *list)
{
        _llist_sort(list);
}

//==============================================================================
/**
 * @brief  Leave only an unique elements, all not unique are removed (are destroyed)
 * @param  list         list object
 * @return None
 */
//==============================================================================
static inline void _sys_llist_unique(llist_t *list)
{
        _llist_unique(list);
}

//==============================================================================
/**
 * @brief  Reverse entire table
 * @param  list         list object
 * @return None
 */
//==============================================================================
static inline void _sys_llist_reverse(llist_t *list)
{
        _llist_reverse(list);
}

//==============================================================================
/**
 * @brief  Get element from the list at selected position
 * @param  list         list object
 * @param  position     begin position
 * @return Pointer to data, or NULL on error
 */
//==============================================================================
static inline void *_sys_llist_at(llist_t *list, int position)
{
        return _llist_at(list, position);
}

//==============================================================================
/**
 * @brief  Check if list contains selected object
 * @param  list         list object
 * @param  object       object to find
 * @return Number of found objects, or -1 on error
 */
//==============================================================================
static inline int _sys_llist_contains(llist_t *list, const void *object)
{
        return _llist_contains(list, object);
}

//==============================================================================
/**
 * @brief  Find selected object in the list from the beginning
 * @param  list         list object
 * @param  object       object to find
 * @return Object position, or -1 on error
 */
//==============================================================================
static inline int _sys_llist_find_begin(llist_t *list, const void *object)
{
        return _llist_find_begin(list, object);
}

//==============================================================================
/**
 * @brief  Find selected object in the list from the end
 * @param  list         list object
 * @param  object       object to find
 * @return Object position, or -1 on error
 */
//==============================================================================
static inline int _sys_llist_find_end(llist_t *list, const void *object)
{
        return _llist_find_end(list, object);
}

//==============================================================================
/**
 * @brief  Access first element
 * @param  list         list object
 * @return Pointer to data, or NULL on error
 */
//==============================================================================
static inline void *_sys_llist_front(llist_t *list)
{
        return _llist_front(list);
}

//==============================================================================
/**
 * @brief  Access last element
 * @param  list         list object
 * @return Pointer to data, or NULL on error
 */
//==============================================================================
static inline void *_sys_llist_back(llist_t *list)
{
        return _llist_back(list);
}

//==============================================================================
/**
 * @brief  Create an iterator to the list
 * @param  list         list object
 * @return Iterator object
 */
//==============================================================================
static inline llist_iterator_t _sys_llist_iterator(llist_t *list)
{
        return _llist_iterator(list);
}

//==============================================================================
/**
 * @brief  Return first object from list by using iterator
 * @param  iterator     iterator object
 * @return Pointer to data object
 */
//==============================================================================
static inline void *_sys_llist_begin(llist_iterator_t *iterator)
{
        return _llist_begin(iterator);
}

//==============================================================================
/**
 * @brief  Return last object from list by using iterator
 * @param  iterator     iterator object
 * @return Pointer to data object
 */
//==============================================================================
static inline void *_sys_llist_end(llist_iterator_t *iterator)
{
        return _llist_end(iterator);
}

//==============================================================================
/**
 * @brief  Return selected objects from list by using range iterator (forward)
 * @param  iterator     iterator object
 * @param  begin        begin position
 * @param  end          end position
 * @return Pointer to data object
 */
//==============================================================================
static inline void *_sys_llist_range(llist_iterator_t *iterator, int begin, int end)
{
        return _llist_range(iterator, begin, end);
}

//==============================================================================
/**
 * @brief  Return next data object from list by using iterator
 * @param  iterator     iterator object
 * @return Pointer to data object
 */
//==============================================================================
static inline void *_sys_llist_iterator_next(llist_iterator_t *iterator)
{
        return _llist_iterator_next(iterator);
}

//==============================================================================
/**
 * @brief  Return previous data object from list by using iterator
 * @param  iterator     iterator object
 * @return Pointer to data object
 */
//==============================================================================
static inline void *_sys_llist_iterator_prev(llist_iterator_t *iterator)
{
        return _llist_iterator_prev(iterator);
}

//==============================================================================
/**
 * @brief  Erase selected begin of the list. The element is destroyed
 * @param  iterator     position to remove
 * @return On success 1 is returned, otherwise 0
 */
//==============================================================================
static inline int _sys_llist_erase_by_iterator(llist_iterator_t *iterator)
{
        return _llist_erase_by_iterator(iterator);
}

//==============================================================================
/**
 * @brief  Compare functor that compares two pointers (not contents)
 * @param  a    pointer a
 * @param  b    pointer b
 * @return a > b:  1
 *         a = b:  0
 *         a < b: -1
 */
//==============================================================================
static inline int _sys_llist_functor_cmp_pointers(const void *a, const void *b)
{
        return _llist_functor_cmp_pointers(a, b);
}

//==============================================================================
/**
 * @brief  Compare functor that compares two strings (contents)
 * @param  a    string a
 * @param  b    string b
 * @return a > b:  1
 *         a = b:  0
 *         a < b: -1
 */
//==============================================================================
static inline int _sys_llist_functor_cmp_strings(const void *a, const void *b)
{
        return _llist_functor_cmp_strings(a, b);
}

//==============================================================================
/**
 * @brief Function create node for driver file
 *
 * @param[in] path              path when driver-file shall be created
 * @param[in] dev               device number
 *
 * @return zero on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_mknod(const char *path, dev_t dev)
{
        return _vfs_mknod(path, dev);
}

//==============================================================================
/**
 * @brief Create directory
 *
 * @param[in] path              path to new directory
 * @param[in] mode              directory mode
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_mkdir(const char *path, mode_t mode)
{
        return _vfs_mkdir(path, mode);
}

//==============================================================================
/**
 * @brief Create pipe
 *
 * @param[in] path              path to pipe
 * @param[in] mode              directory mode
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_mkfifo(const char *path, mode_t mode)
{
        return _vfs_mkfifo(path, mode);
}

//==============================================================================
/**
 * @brief Function open directory
 *
 * @param[in] *path                 directory path
 *
 * @return directory object
 */
//==============================================================================
static inline DIR *_sys_opendir(const char *path)
{
        return _vfs_opendir(path);
}

//==============================================================================
/**
 * @brief Function close opened directory
 *
 * @param[in] *dir                  directory object
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_closedir(DIR *dir)
{
        return _vfs_closedir(dir);
}

//==============================================================================
/**
 * @brief Function read next item of opened directory
 *
 * @param[in] *dir                  directory object
 *
 * @return element attributes
 */
//==============================================================================
static inline dirent_t _sys_readdir(DIR *dir)
{
        return _vfs_readdir(dir);
}

//==============================================================================
/**
 * @brief Remove file
 * Removes file or directory. Removes directory if is not a mount point.
 *
 * @param[in] *path                localization of file/directory
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_remove(const char *path)
{
        return _vfs_remove(path);
}

//==============================================================================
/**
 * @brief Rename file name
 * The implementation of rename can move files only if external FS provide
 * functionality. Local VFS cannot do this. Cross FS move is also not possible.
 *
 * @param[in] *old_name                  old file name
 * @param[in] *new_name                  new file name
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_rename(const char *old_name, const char *new_name)
{
        return _vfs_rename(old_name, new_name);
}

//==============================================================================
/**
 * @brief Function change file mode
 *
 * @param[in] *path         file path
 * @param[in]  mode         file mode
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_chmod(const char *path, mode_t mode)
{
        return _vfs_chmod(path, mode);
}

//==============================================================================
/**
 * @brief Function change file owner and group
 *
 * @param[in] *path         file path
 * @param[in]  owner        file owner
 * @param[in]  group        file group
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_chown(const char *path, uid_t owner, gid_t group)
{
        return _vfs_chown(path, owner, group);
}

//==============================================================================
/**
 * @brief Function returns file/dir status
 *
 * @param[in]  *path            file/dir path
 * @param[out] *stat            pointer to structure
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_stat(const char *path, struct stat *stat)
{
        return _vfs_stat(path, stat);
}

//==============================================================================
/**
 * @brief Function returns file system status
 *
 * @param[in]  *path            fs path
 * @param[out] *statfs          pointer to FS status structure
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_statfs(const char *path, struct statfs *statfs)
{
        return _vfs_statfs(path, statfs);
}

//==============================================================================
/**
 * @brief Function open selected file
 *
 * @param[in] *name             file path
 * @param[in] *mode             file mode
 *
 * @retval NULL if file can't be created
 */
//==============================================================================
static inline FILE *_sys_fopen(const char *name, const char *mode)
{
        return _vfs_fopen(name, mode);
}

//==============================================================================
/**
 * @brief Function close old stream and open new
 *
 * @param[in] *name             file path
 * @param[in] *mode             file mode
 * @param[in] *file             old stream
 *
 * @retval NULL if file can't be created
 */
//==============================================================================
static inline FILE *_sys_freopen(const char *name, const char *mode, FILE *file)
{
        return _vfs_freopen(name, mode, file);
}

//==============================================================================
/**
 * @brief Function close opened file
 *
 * @param[in] *file             pinter to file
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_fclose(FILE *file)
{
        return _vfs_fclose(file);
}

//==============================================================================
/**
 * @brief Function write data to file
 *
 * @param[in] *ptr              address to data (src)
 * @param[in]  size             item size
 * @param[in]  count            number of items
 * @param[in] *file             pointer to file object
 *
 * @return the number of items successfully written. If an error occurs, or the
 *         end-of-file is reached, the return value is a short item count (or 0).
 */
//==============================================================================
static inline size_t _sys_fwrite(const void *ptr, size_t size, size_t count, FILE *file)
{
        return _vfs_fwrite(ptr, size, count, file);
}

//==============================================================================
/**
 * @brief Function read data from file
 *
 * @param[out] *ptr             address to data (dst)
 * @param[in]   size            item size
 * @param[in]   count           number of items
 * @param[in]  *file            pointer to file object
 *
 * @return the number of items successfully read. If an error occurs, or the
 *         end-of-file is reached, the return value is a short item count (or 0).
 */
//==============================================================================
static inline size_t _sys_fread(void *ptr, size_t size, size_t count, FILE *file)
{
        return _vfs_fread(ptr, size, count, file);
}

//==============================================================================
/**
 * @brief Function set seek value
 *
 * @param[in] *file             file object
 * @param[in]  offset           seek value
 * @param[in]  mode             seek mode
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_fseek(FILE *file, i64_t offset, int mode)
{
        return _vfs_fseek(file, offset, mode);
}

//==============================================================================
/**
 * @brief Function returns seek value
 *
 * @param[in] *file             file object
 *
 * @return -1 if error, otherwise correct value
 */
//==============================================================================
static inline i64_t _sys_ftell(FILE *file)
{
        return _vfs_ftell(file);
}

//==============================================================================
/**
 * @brief Function support not standard operations on devices
 *
 * @param[in]     *file         file
 * @param[in]      rq           request
 * @param[in,out] ...           additional function arguments
 *
 * @return 0 on success. On error, different from 0 is returned
 */
//==============================================================================
#define _sys_ioctl(FILE__stream, ...) _vfs_ioctl(FILE__stream, __VA_ARGS__)

//==============================================================================
/**
 * @brief Function returns file/dir status
 *
 * @param[in]  *file            file object
 * @param[out] *stat            pointer to stat structure
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_fstat(FILE *file, struct stat *stat)
{
        return _vfs_fstat(file, stat);
}

//==============================================================================
/**
 * @brief Function flush file data
 *
 * @param[in] *file     file to flush
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_fflush(FILE *file)
{
        return _vfs_fflush(file);
}

//==============================================================================
/**
 * @brief Function check end of file
 *
 * @param[in] *file     file
 *
 * @return 0 if there is not a file end, otherwise greater than 0
 */
//==============================================================================
static inline int _sys_feof(FILE *file)
{
        return _vfs_feof(file);
}

//==============================================================================
/**
 * @brief Function clear file's error
 *
 * @param[in] *file     file
 */
//==============================================================================
static inline void _sys_clearerr(FILE *file)
{
        return _vfs_clearerr(file);
}

//==============================================================================
/**
 * @brief Function check that file has no errors
 *
 * @param[in] *file     file
 *
 * @return nonzero value if the file stream has errors occurred, 0 otherwise
 */
//==============================================================================
static inline int _sys_ferror(FILE *file)
{
        return _vfs_ferror(file);
}

//==============================================================================
/**
 * @brief Function rewind file
 *
 * @param[in] *file     file
 *
 * @return 0 on success. On error, -1 is returned
 */
//==============================================================================
static inline int _sys_rewind(FILE *file)
{
        return _vfs_rewind(file);
}

//==============================================================================
/**
 * @brief Synchronize internal buffers of mounted file systems
 *
 * @param None
 *
 * @errors None
 *
 * @return None
 */
//==============================================================================
static inline void _sys_sync()
{
        _vfs_sync();
}

//==============================================================================
/**
 * @brief Function send kernel message on terminal
 *
 * @param *format             formated text
 * @param ...                 format arguments
 */
//==============================================================================
#define _sys_printk(...) _printk(__VA_ARGS__)

//==============================================================================
/**
 * @brief Enable printk functionality
 *
 * @param filename      path to file used to write kernel log
 */
//==============================================================================
static inline void _sys_printk_enable(char *filename)
{
        _printk_enable(filename);
}

//==============================================================================
/**
 * @brief Disable printk functionality
 */
//==============================================================================
static inline void _sys_printk_disable()
{
        _printk_disable();
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
#define _sys_snprintf(char__bfr, size_t__size, ...) _snprintf(char__bfr, size_t__size, __VA_ARGS__)

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
#define _sys_fprintf(FILE__file, ...) _fprintf(FILE__file, __VA_ARGS__)

//==============================================================================
/**
 * @brief Function write to file formatted string
 *
 * @param file                file
 * @param format              formated text
 * @param args                arguments
 *
 * @retval number of written characters
 */
//==============================================================================
static inline int _sys_vfprintf(FILE *file, const char *format, va_list args)
{
        return _vfprintf(file, format, args);
}

//==============================================================================
/**
 * @brief Function convert arguments to stream
 *
 * @param[in] *buf           buffer for stream
 * @param[in]  size          buffer size
 * @param[in] *format        message format
 * @param[in]  args          argument list
 *
 * @return number of printed characters
 */
//==============================================================================
static inline int _sys_vsnprintf(char *buf, size_t size, const char *format, va_list args)
{
        return _vsnprintf(buf, size, format, args);
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
static inline const char *_sys_strerror(int errnum)
{
        return _strerror(errnum);
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
static inline int _sys_fputc(int c, FILE *stream)
{
        return _fputc(c, stream);
}

//==============================================================================
/**
 * @brief Function puts string to selected file
 *
 * @param[in] *s        string
 * @param[in] *file     file
 *
 * @return number of characters written to the stream
 */
//==============================================================================
static inline int _sys_fputs(const char *s, FILE *file)
{
        return _f_puts(s, file, false);
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
static inline int _sys_getc(FILE *stream)
{
        return _getc(stream);
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
static inline char *_sys_fgets(char *str, int size, FILE *stream)
{
        return _fgets(str, size, stream);
}

//==============================================================================
/**
 * @brief Function scan stream
 *
 * @param[in]  *stream        file
 * @param[in]  *format        message format
 * @param[out]  ...           output
 *
 * @return number of scanned elements
 */
//==============================================================================
#define _sys_fscanf(FILE__stream, ...) _fscanf(FILE__stream, __VA_ARGS__)

//==============================================================================
/**
 * @brief Function scan stream
 *
 * @param[in]  *stream        file
 * @param[in]  *format        message format
 * @param[out]  args          output arguments
 *
 * @return number of scanned elements
 */
//==============================================================================
static inline int _sys_vfscanf(FILE *stream, const char *format, va_list args)
{
        return _vfscanf(stream, format, args);
}

//==============================================================================
/**
 * @brief Function scan arguments defined by format (multiple argument version)
 *
 * @param[in]  *str           data buffer
 * @param[in]  *format        scan format
 * @param[out]  ...           output
 *
 * @return number of scanned elements
 */
//==============================================================================
#define _sys_sscanf(const_char__str, ...) _sscanf(const_char__str, __VA_ARGS__)

//==============================================================================
/**
 * @brief Function scan arguments defined by format (argument list version)
 *
 * @param[in]  *str           data buffer
 * @param[in]  *format        scan format
 * @param[out]  args          output
 *
 * @return number of scanned elements
 */
//==============================================================================
static inline int _sys_vsscanf(const char *str, const char *format, va_list args)
{
        return _vsscanf(str, format, args);
}

#ifdef __cplusplus
}
#endif

#endif /* _SYSFUNC_H_ */
/*==============================================================================
  End of file
==============================================================================*/
