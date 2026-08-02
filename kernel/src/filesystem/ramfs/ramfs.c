#include "ramfs.h"

#include "../../memory/heap.h"
#include "../../string/string.h"
#include "../../print_and_stuff/print.h"

static ramfs_node_t *root = NULL;

void ramfs_init(void){

    root = kmalloc(sizeof(ramfs_node_t));

    if(root == NULL) return;

    root->header.name = strdup("/");

    root->header.type = FS_DIRECTORY;

    root->parent = NULL;

    root->children = NULL;
    root->next = NULL;

    root->header.data = NULL;
    root->header.size = 0;
}

ramfs_node_t *ramfs_root(void){
    return root;
}

ramfs_node_t *ramfs_find(ramfs_node_t *parent, const char *name){

    if(parent == NULL) return NULL;

    ramfs_node_t *it = parent->children;

    while(it){
        if(strcmp(it->header.name, name) == 0) return it;

        it = it->next;
    }
    return NULL;
}

ramfs_node_t *ramfs_create(ramfs_node_t *parent, const char *name, fs_node_type_t type){

    if(parent == NULL) return NULL;
    if(parent->header.type != FS_DIRECTORY) return NULL;
    if(ramfs_find(parent, name)) return NULL;

    ramfs_node_t *node = kmalloc(sizeof(ramfs_node_t));

    if(node == NULL) return NULL;

    node->header.name = strdup(name);

    node->header.type = type;

    node->parent = parent;

    node->children = NULL;
    node->next = NULL;

    node->header.data = NULL;
    node->header.size = 0;

    if(parent->children == NULL){
        parent->children = node;
        return node;
    }

    ramfs_node_t *it =parent->children;

    while(it->next) it = it->next;

    it->next = node;

    return node;
}

void ramfs_remove(ramfs_node_t *node){

    if(node == NULL) return;

    if(node == root) return;

    if(node->children) return;

    ramfs_node_t *parent = node->parent;

    if(parent == NULL) return;

    if(parent->children == node) parent->children = node->next;
    else{
        ramfs_node_t *it = parent->children;
        while(it && it->next != node) it = it->next;

        if(it) it->next = node->next;
    }

    kfree(node->header.name);
    kfree(node->header.data);
    kfree(node);
}

static void ramfs_dump_node(ramfs_node_t *node, size_t depth){
    if(node == NULL) return;

    for(size_t i = 0; i < depth; i++) print_char('\t');

    print_string(node->header.name);

    if(node->header.type == FS_DIRECTORY) print_char('/');
    print_char('\n');

    ramfs_dump_node(node->children, depth + 1);
    ramfs_dump_node(node->next, depth);
}

void ramfs_dump(ramfs_node_t *node){
    ramfs_dump_node(node, 0);
}

bool ramfs_exists(ramfs_node_t *parent, const char *name){
    return ramfs_find(parent,name) != NULL;
}

ramfs_node_t *ramfs_mkdir(ramfs_node_t *parent, const char *name){
    return ramfs_create(parent, name, FS_DIRECTORY);
}

size_t ramfs_read(ramfs_node_t *node, void *buffer, size_t size, size_t offset){

    if(node == NULL) return 0;

    if(node->header.type != FS_FILE) return 0;

    if(offset >= node->header.size) return 0;

    size_t remaining = node->header.size - offset;

    if(size > remaining) size = remaining;

    memcpy(buffer, (uint8_t *)node->header.data + offset, size);

    return size;
}

bool ramfs_write(ramfs_node_t *node, void *buffer, size_t size){

    if(node == NULL) return false;

    if(node->header.type != FS_FILE) return false;

    if(node->header.data) kfree(node->header.data);

    node->header.data = kmalloc(size);

    if(!node->header.data) return false;

    memcpy(node->header.data, buffer, size);

    node->header.size = size;

    return true;
}

bool ramfs_rename(ramfs_node_t *node, const char *new_name){

    if(node == NULL) return false;

    if(ramfs_find(node->parent, new_name)) return false;

    kfree(node->header.name);

    node->header.name = strdup(new_name);

    if(node->header.name == NULL) return false;

    return true;
}

bool ramfs_move(ramfs_node_t *node, ramfs_node_t *new_parent){

    if(node == NULL) return false;
    if(new_parent == NULL) return false;

    if(node == root) return false;
    if(node == new_parent) return false;

    ramfs_node_t *parent = node->parent;

    if(parent == NULL) return false;

    if(node->children == new_parent) return false;

    if(parent->children == node) parent->children = node->next;
    else{
        ramfs_node_t *it = parent->children;
        while(it && it->next != node) it = it->next;

        if(it) it->next = node->next;
        else return false;
    }

    node->next = NULL;

    if(!new_parent->children) new_parent->children = node;
    else{
        ramfs_node_t *it = new_parent->children;
        while(it->next) it = it->next;

        it->next = node;
    }

    node->parent = new_parent;

    return true;
}

ramfs_node_t *ramfs_copy(ramfs_node_t *node, ramfs_node_t *new_parent){

    if(node == NULL) return NULL;
    if(new_parent == NULL) return NULL;

    const char *name = node->header.name;

    char new_name[strlen(name)+6];

    if(ramfs_find(new_parent, name)){

        size_t len = strlen(name);

        memcpy(new_name, name, len);
        memcpy(new_name + len, "_copy", 5);

        new_name[len+5] = '\0';

        name = new_name;
    }

    ramfs_node_t *copy = ramfs_create(new_parent, name, node->header.type);

    if(copy == NULL) return NULL;

    if(node->header.type == FS_FILE){
        if(node->header.size == 0) return copy;
        copy->header.data = kmalloc(node->header.size);

        if(copy->header.data == NULL) {
            ramfs_delete(copy);
            return NULL;
        }

        memcpy(copy->header.data, node->header.data, node->header.size);

        copy->header.size = node->header.size;
    }
    if(node->header.type == FS_DIRECTORY){
        if(node->children == NULL) return copy;
        ramfs_node_t *child = node->children;

        while(child){
            ramfs_copy(child,copy);
            child = child->next;
        }
    }
    return copy;
}

bool ramfs_delete(ramfs_node_t *node){
    if(node == NULL) return false;

    while(node->children) ramfs_delete(node->children);

    ramfs_remove(node);
    return true;
}