#ifndef FS_H
#define FS_H

// Use lwIP's own struct fs_file. httpd.c is compiled against it, so a local
// copy with a different layout makes httpd read garbage (e.g. `flags`).
#include "lwip/apps/fs.h"

#endif
