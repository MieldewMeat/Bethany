#include "block.h"
#include <stddef.h>

static block_device_t *current = NULL;

bool block_register(block_device_t *device){
    if(device == NULL) return false;

    current = device;

    return true;
}

block_device_t *block_device(void){
    return current;
}