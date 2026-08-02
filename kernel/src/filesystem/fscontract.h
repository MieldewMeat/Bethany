#pragma once
#include <stddef.h>

typedef enum{
    FS_FILE,
    FS_DIRECTORY
} fs_node_type_t;

typedef struct {
    char *name;

    size_t size;

    void *data;

    fs_node_type_t type;
} fs_node_base_t;