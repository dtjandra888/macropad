#ifndef FS_H
#define FS_H

#include "lwip/err.h"
#include "lwip/opt.h"

#define FS_FILE_FLAGS_HEADER_INCLUDED    0x01
#define FS_FILE_FLAGS_HEADER_PERSISTENT  0x02
#define FS_FILE_FLAGS_HEADER_HTTPVER_1_1 0x04
#define FS_FILE_FLAGS_SSI                0x08
#define FS_FILE_FLAGS_CUSTOM             0x10

struct fs_file {
    const char *data;
    int len;
    int index;
    void *pextension;
    u8_t http_header_included;
};

err_t fs_open(struct fs_file *file, const char *name);
void fs_close(struct fs_file *file);
int fs_bytes_left(struct fs_file *file);

#endif
