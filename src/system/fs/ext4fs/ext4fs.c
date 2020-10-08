        Copyright (C) 2019 Daniel Zorychta <daniel.zorychta@gmail.com>
        FILE                      *dev;
        mutex_t                   *fs_mutex;
static int bdev_lock(struct ext4_blockdev *bdev);
static int bdev_unlock(struct ext4_blockdev *bdev);
static void mp_lock(void *p_user);
static void mp_unlock(void *p_user);
static const struct ext4_lock EXT4_LOCK = {
        .lock   = mp_lock,
        .unlock = mp_unlock,
};
                bool read_only = sys_stropt_is_flag(opts, "ro");

                err = sys_fopen(src_path, read_only ? "r" : "r+", cast(FILE**, &hdl->dev));
                err = sys_fstat(hdl->dev, &st);
                err = sys_mutex_create(MUTEX_TYPE_RECURSIVE, cast(mutex_t**, &hdl->fs_mutex));
                hdl->bdif.lock     = bdev_lock;
                hdl->bdif.unlock   = bdev_unlock;
                hdl->bdif.p_user   = hdl;
                hdl->bd.part_size   = st.st_size;
                hdl->bd.bdif        = &hdl->bdif;
                err = ext4_mount(&hdl->bd, &hdl->mp, read_only, hdl);
                        ext4_mount_setup_locks(hdl->mp, &EXT4_LOCK);

                        ext4_recover(hdl->mp);
                        ext4_journal_start(hdl->mp);

                        ext4_cache_write_back(__EXT4FS_CFG_WR_BUF_STRATEGY__, hdl->mp);

                        if (read_only) {
                                printk("EXTFS: read only file system");
                        }
                        if (hdl->fs_mutex) {
                                sys_mutex_destroy(hdl->fs_mutex);
                        if (hdl->dev) {
                                sys_fclose(cast(FILE*, hdl->dev));
                err = ext4_cache_write_back(false, hdl->mp);
                ext4_journal_stop(hdl->mp);

                sys_mutex_destroy(hdl->fs_mutex);
                sys_fclose(cast(FILE*, hdl->dev));
                err = ext4_fopen2(file, path, flags, hdl->mp);
                                time_t time = 0;
                                if (sys_gettime(&time) == ESUCC) {

                                        ext4_ctime_set(path, time, hdl->mp);
                                        ext4_mtime_set(path, file, time, hdl->mp);
                                        ext4_atime_set(path, time, hdl->mp);
                        ext4_mtime_set(NULL, fhdl, mtime, hdl->mp);
        int err = ext4_ctime_get(NULL, file, &ctime, hdl->mp);
        err = ext4_mtime_get(NULL, file, &mtime, hdl->mp);
        err = ext4_owner_get(NULL, file, &uid, &gid, hdl->mp);
        err = ext4_mode_get(NULL, file, &mode, hdl->mp);
        int err = ext4_ctime_get(path, NULL, &ctime, hdl->mp);
        err = ext4_mtime_get(path, NULL, &mtime, hdl->mp);
        err = ext4_owner_get(path, NULL, &uid, &gid, hdl->mp);
        err = ext4_mode_get(path, NULL, &mode, hdl->mp);
        memset(&file, 0, sizeof(file));
        if (ext4_fopen2(&file, path, O_RDONLY, hdl->mp) == ESUCC) {
        statfs->f_type   = SYS_FS_TYPE__SOLID;
        statfs->f_fsname = "ext4fs";

        return err;
        int err = ext4_dir_mk(path, hdl->mp);
                err = ext4_mode_set(path, mode, hdl->mp);
                        ext4_mtime_set(path, NULL, ctime, hdl->mp);
                err = ext4_dir_open(dir->d_hdl, path, hdl->mp);
                ext4_file f;
                f.flags = 0;
                f.fpos  = 0;
                f.fsize = 0;
                f.inode = de->inode;
                f.mp    = hdl->mp;

                u32_t mode = 0;
                err = ext4_mode_get(NULL, &f, &mode, hdl->mp);
                        dir->dirent.size   = ext4_fsize(&f);
        u32_t mode;
        int err = ext4_mode_get(path, NULL, &mode, hdl->mp);
        if (!err) {
                if ((mode &= EXT4_INODE_MODE_TYPE_MASK) == EXT4_INODE_MODE_DIRECTORY) {
                        err = ext4_dir_rm(path, hdl->mp);
                } else {
                        err = ext4_fremove(path, hdl->mp);
                }
        return ext4_frename(old_name, new_name, hdl->mp);
        return ext4_mode_set(path, mode, hdl->mp);
        return ext4_owner_set(path, owner, group, hdl->mp);
        int err = ext4_cache_write_back(false, hdl->mp);
                ext4_cache_write_back(__EXT4FS_CFG_WR_BUF_STRATEGY__, hdl->mp);
        if (sys_mutex_lock(hdl->fs_mutex, LOCK_TIMEOUT) == ESUCC) {
                sys_mutex_unlock(hdl->fs_mutex);
        if (sys_mutex_lock(hdl->fs_mutex, LOCK_TIMEOUT) == ESUCC) {
                sys_mutex_unlock(hdl->fs_mutex);
        ext4fs_t *hdl = bdev->bdif->p_user;

        size_t rdcnt = 0;
        sys_fseek(hdl->dev, blk_id * bdev->bdif->ph_bsize, SEEK_SET);
        return sys_fread(buf, bdev->bdif->ph_bsize * blk_cnt, &rdcnt, hdl->dev);
        ext4fs_t *hdl = bdev->bdif->p_user;

        size_t wrcnt = 0;
        sys_fseek(hdl->dev, blk_id * bdev->bdif->ph_bsize, SEEK_SET);
        return sys_fwrite(buf, bdev->bdif->ph_bsize * blk_cnt, &wrcnt, hdl->dev);
static int bdev_lock(struct ext4_blockdev *bdev)
        ext4fs_t *hdl = bdev->bdif->p_user;
        return sys_mutex_lock(hdl->fs_mutex, LOCK_TIMEOUT);
static int bdev_unlock(struct ext4_blockdev *bdev)
        ext4fs_t *hdl = bdev->bdif->p_user;
        return sys_mutex_unlock(hdl->fs_mutex);
}

//==============================================================================
/**
 * @brief  Function lock access to mount point.
 *
 * @param  p_user       user object pointer
 */
//==============================================================================
static void mp_lock(void *p_user)
{
        ext4fs_t *hdl = p_user;
        sys_mutex_lock(hdl->fs_mutex, LOCK_TIMEOUT);
}

//==============================================================================
/**
 * @brief  Function unlock access to mount point.
 *
 * @param  p_user       user object pointer
 */
//==============================================================================
static void mp_unlock(void *p_user)
{
        ext4fs_t *hdl = p_user;
        sys_mutex_unlock(hdl->fs_mutex);
        switch (mode & EXT4_INODE_MODE_TYPE_MASK) {