#include "heap.h"
#include "vmm.h"
#include <stdint.h>

#define KERNEL_HEAP_START 0xFFFF900000000000ULL

static uint8_t* heap_start;
static uint8_t* heap_end;
static uint8_t* heap_current;

static inline size_t align_up(size_t value, size_t alignment){
    return (value + alignment - 1) & ~(alignment - 1);
}

void heap_init(void){
    heap_start = (uint8_t*)KERNEL_HEAP_START;
    heap_current = heap_start;
    heap_end = heap_start;

    if(vmm_alloc_page((uint64_t)heap_end, PAGE_PRESENT | PAGE_WRITABLE)) heap_end += PAGE_SIZE;
}

void* kmalloc(size_t size){
    if(size == 0) return NULL;

    size = align_up(size, 16);

    while(heap_current + size > heap_end){
        if(!vmm_alloc_page((uint64_t)heap_end, PAGE_PRESENT | PAGE_WRITABLE)) return NULL;

        heap_end += PAGE_SIZE;
    }

    void* ptr = heap_current;
    heap_current += size;

    return ptr;
}

void kfree(void* ptr){
    (void)ptr;
}

void* krealloc(void* ptr, size_t size){

    if(ptr == NULL) return kmalloc(size);

    if(size == 0){
        kfree(ptr);
        return NULL;
    }

    void* new_ptr = kmalloc(size);

    if(new_ptr == NULL) return NULL;

    return new_ptr;
}

void* kcalloc(size_t count, size_t size){
    size_t total = count * size;

    uint8_t* ptr = kmalloc(total);

    if(ptr == NULL) return NULL;

    for(size_t i = 0; i < total; i++) ptr[i] = 0;
    
    return ptr;
}