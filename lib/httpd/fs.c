#include "lwip/opt.h"
#include "lwip/err.h"

#include "fs.h"
#include "fsdata.h"

#include <string.h>

err_t fs_open(struct fs_file *file, const char *name)
{
    const struct fsdata_file *f;

    if (file == NULL || name == NULL) {
        return ERR_ARG;
    }

    for (f = FS_ROOT; f != NULL; f = f->next) {
        if (strcmp(name, (const char *)f->name) == 0) {
            file->data = (const char *)f->data;
            file->len = f->len;
            file->index = f->len;
            file->pextension = NULL;
            file->http_header_included = f->http_header_included;

            return ERR_OK;
        }
    }

    return ERR_VAL;
}

void fs_close(struct fs_file *file)
{
    (void)file;
}

int fs_bytes_left(struct fs_file *file)
{
    return file->len - file->index;
}
