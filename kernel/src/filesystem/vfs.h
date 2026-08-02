#pragma once

#include "fscontract.h"

#define FS_RAMFS 1
#define FS_BETHFS 2

#define FS FS_RAMFS

#if FS == FS_RAMFS

#include "ramfs/ramfs.h"

#define vfs_node_t ramfs_node_t

#define vfs_init ramfs_init

#define vfs_root ramfs_root

#define vfs_find ramfs_find
#define vfs_exists ramfs_exists


#define vfs_create ramfs_create
#define vfs_mkdir ramfs_mkdir

#define vfs_read ramfs_read
#define vfs_write ramfs_write

#define vfs_rename ramfs_rename
#define vfs_move ramfs_move

#define vfs_copy ramfs_copy

#define vfs_remove ramfs_remove
#define vfs_delete ramfs_delete

#define vfs_dump ramfs_dump

#elif FS == FS_BETHFS

#include "bethfs/bethfs.h"
#define vfs_init bfs_init
#define vfs_root bfs_root
#define vfs_create bfs_create
#define vfs_remove bfs_remove
#define vfs_find bfs_find
#define vfs_dump bfs_dump

#endif