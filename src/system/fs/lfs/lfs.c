static inline char      get_first_char                  (const char *str);
static inline char      get_last_char                   (const char *str);
static void             recursive_mutex_force_lock      (mutex_t *mtx);
static node_t          *new_node                        (struct LFS_data *lfs, node_t *nodebase, char *filename, i32_t *item);
static stdret_t         delete_node                     (node_t *base, node_t *target, u32_t baseitemid);
static node_t          *get_node                        (const char *path, node_t *startnode, i32_t deep, i32_t *item);
static uint             get_path_deep                   (const char *path);
static dirent_t         lfs_readdir                     (void *fs_handle, DIR *dir);
static stdret_t         lfs_closedir                    (void *fs_handle, DIR *dir);
static stdret_t         add_node_to_list_of_open_files  (struct LFS_data *lfs, node_t *base_node, node_t *node, i32_t *item);
        lfs->resource_mtx  = mutex_new();
        lfs->root_dir.data = list_new();
        lfs->list_of_opended_files = list_new();
                        mutex_delete(lfs->resource_mtx);
                        list_delete(lfs->root_dir.data);
                        list_delete(lfs->list_of_opended_files);
        errno = EPERM;

        recursive_mutex_force_lock(lfs->resource_mtx);
                                recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                if ((new_dir->data = list_new())) {
                                recursive_mutex_unlock(lfs->resource_mtx);
                                list_delete(new_dir->data);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
        } else {
                errno = ENOENT;
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
        if (get_last_char(path) == '/') {
                        errno = ENOTDIR;
                        recursive_mutex_unlock(lfs->resource_mtx);
                } else {
                        errno = ENOENT;
                recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
        if (get_first_char(old_name) != '/' || get_first_char(new_name) != '/') {
        if (get_last_char(old_name) == '/' || get_last_char(new_name) == '/') {
                recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                if ( (get_last_char(path) == '/' && node->type == NODE_TYPE_DIR)
                   || get_last_char(path) != '/') {
                        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                        recursive_mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;

        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
        recursive_mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
        errno = ENOENT;

        recursive_mutex_unlock(lfs->resource_mtx);
 * @return number of written bytes, -1 if error
        recursive_mutex_force_lock(lfs->resource_mtx);
                errno = ENOENT;
                errno = ENOENT;
                        recursive_mutex_unlock(lfs->resource_mtx);
                                errno = ENOSPC;
        recursive_mutex_unlock(lfs->resource_mtx);
 * @return number of read bytes, -1 if error
        recursive_mutex_force_lock(lfs->resource_mtx);
                        recursive_mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                        recursive_mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        recursive_mutex_unlock(lfs->resource_mtx);
        recursive_mutex_force_lock(lfs->resource_mtx);
                        recursive_mutex_unlock(lfs->resource_mtx);
        errno = ENOENT;
        recursive_mutex_unlock(lfs->resource_mtx);
//==============================================================================
/**
 * @brief Return last character of selected string
 */
//==============================================================================
static inline char get_last_char(const char *str)
{
        return LAST_CHARACTER(str);
}

//==============================================================================
/**
 * @brief Return first character of selected string
 */
//==============================================================================
static inline char get_first_char(const char *str)
{
        return FIRST_CHARACTER(str);
}

//==============================================================================
/**
 * @brief Function force lock mutex
 *
 * @param mtx           mutex
 */
//==============================================================================
static void recursive_mutex_force_lock(mutex_t *mtx)
{
        while (recursive_mutex_lock(mtx, MTX_BLOCK_TIME) != MUTEX_LOCKED);
}

                        list_delete(target->data);
 *        ERRNO: ENOENT
 * @param[in]  path             path
 * @param[in]  startnode        start node
 * @param[out] extPath          external path begin (pointer from path)
 * @param[in]  deep             deep control
 * @param[out] item             node is n-item of list which was found
                errno = ENOENT;
                errno = ENOENT;
                        errno        = ENOENT;
 * ERRNO: ENOENT, ENOTDIR, ENOMEM
                errno = ENOENT;
                errno = ENOTDIR;
                errno = ENOENT;
 *        ERRNO: ENOMEM
        errno = ENOMEM;