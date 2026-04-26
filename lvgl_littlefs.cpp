#include "lvgl_littlefs.h"
#include <lvgl.h>
#include <LittleFS.h>

// Each open file is heap-allocated so LVGL gets a stable void* handle.
static void* lfs_open_cb(lv_fs_drv_t*, const char* path, lv_fs_mode_t mode) {
    if (mode != LV_FS_MODE_RD) return nullptr;
    File* f = new File();
    // LVGL strips "S:" and gives us the path starting with '/', e.g. "/screensaver.gif"
    *f = LittleFS.open(path, "r");
    if (!*f) { delete f; return nullptr; }
    return (void*)f;
}

static lv_fs_res_t lfs_close_cb(lv_fs_drv_t*, void* fh) {
    File* f = (File*)fh;
    f->close();
    delete f;
    return LV_FS_RES_OK;
}

static lv_fs_res_t lfs_read_cb(lv_fs_drv_t*, void* fh, void* buf, uint32_t btr, uint32_t* br) {
    File* f = (File*)fh;
    *br = f->read((uint8_t*)buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lfs_seek_cb(lv_fs_drv_t*, void* fh, uint32_t pos, lv_fs_whence_t whence) {
    File* f = (File*)fh;
    SeekMode sm = (whence == LV_FS_SEEK_CUR) ? SeekCur
                : (whence == LV_FS_SEEK_END) ? SeekEnd
                :                              SeekSet;
    f->seek(pos, sm);
    return LV_FS_RES_OK;
}

static lv_fs_res_t lfs_tell_cb(lv_fs_drv_t*, void* fh, uint32_t* pos_p) {
    File* f = (File*)fh;
    *pos_p = f->position();
    return LV_FS_RES_OK;
}

void lvgl_littlefs_init() {
    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);
    drv.letter   = 'S';
    drv.open_cb  = lfs_open_cb;
    drv.close_cb = lfs_close_cb;
    drv.read_cb  = lfs_read_cb;
    drv.seek_cb  = lfs_seek_cb;
    drv.tell_cb  = lfs_tell_cb;
    lv_fs_drv_register(&drv);
}
