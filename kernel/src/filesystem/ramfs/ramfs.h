#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../fscontract.h"

typedef struct ramfs_node{
    fs_node_base_t header;

    struct ramfs_node *parent;

    struct ramfs_node *children;
    struct ramfs_node *next;
} ramfs_node_t;

void ramfs_init(void);

ramfs_node_t *ramfs_root(void);


ramfs_node_t *ramfs_find(ramfs_node_t *parent, const char *name);

bool ramfs_exists(ramfs_node_t *parent, const char *name);


ramfs_node_t *ramfs_create(ramfs_node_t *parent, const char *name, fs_node_type_t type);

ramfs_node_t *ramfs_mkdir(ramfs_node_t *parent, const char *name);


size_t ramfs_read(ramfs_node_t *node, void *buffer, size_t size, size_t offset);

bool ramfs_write(ramfs_node_t *node, void *buffer, size_t size);


bool ramfs_rename(ramfs_node_t *node, const char *new_name);

bool ramfs_move(ramfs_node_t *node, ramfs_node_t *new_parent);

ramfs_node_t *ramfs_copy(ramfs_node_t *node, ramfs_node_t *new_parent);


void ramfs_remove(ramfs_node_t *node);

bool ramfs_delete(ramfs_node_t *node);


void ramfs_dump(ramfs_node_t *node);