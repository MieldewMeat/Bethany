#pragma once
#include <stdint.h>
#include <stddef.h>

#include "memory.h"

#define PAGE_SIZE 4096
#define MAX_PAGES 1048576

typedef struct{
    uint64_t base;
    uint64_t pages;
} pmm_region_t;

static pmm_region_t usable[256];
static size_t usable_count = 0;

static uint8_t *bitmap = NULL;
static uint64_t page_count = 0;
static uint64_t bitmap_size = 0;

static uint64_t bitmap_phys;


static void bitmap_set(uint64_t bit){
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static void bitmap_clear(uint64_t bit){
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static uint8_t bitmap_test(uint64_t bit){
    return bitmap[bit / 8] & (1 << (bit % 8));
}


void pmm_init(uint64_t hhdm){

    usable_count = 0;
    page_count = 0;
    bitmap = NULL;

    for(size_t i = 0; i < memory_region_count(); i++){
        const memory_region_t *r = memory_region(i);
        if(r->type != MEMORY_USABLE) continue;

        usable[usable_count].base = r-> base;
        usable[usable_count].pages = r-> length / PAGE_SIZE;

        usable_count ++;
    }

    for(size_t i = 0; i < usable_count; i++) page_count += usable[i].pages;

    bitmap_size = (page_count + 7) / 8;

    for(size_t i = 0; i < usable_count; i++){

        if(usable[i].pages * PAGE_SIZE){
            bitmap_phys = usable[i].base;
            bitmap = (uint8_t *)(bitmap_phys + hhdm);
            break;
        }
    }
    if(bitmap == NULL){
        print_string("PMM: bitmap allocation failed\n");
        for(;;)
            asm("hlt");
    }

    for(size_t i = 0; i < bitmap_size; i++) bitmap[i] = 0;

    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE -1) / PAGE_SIZE;

    for(uint64_t i = 0; i < bitmap_pages; i++) bitmap_set(i);
}

void* pmm_alloc_page(void){
    for(uint64_t bitmap_index = 0; bitmap_index < page_count; bitmap_index++){
        if(bitmap_test(bitmap_index)) continue;

        bitmap_set(bitmap_index);

        uint64_t index = bitmap_index;

        for(size_t i = 0; i < usable_count; i++){
            if(index < usable[i].pages){
                return (void*)(usable[i].base + index * PAGE_SIZE);
            }
            index -= usable[i].pages;
        }
        
    }
    return NULL;
}

void pmm_free_page(void* page){
    if(page == NULL) return;

    uint64_t addr = (uint64_t)page;
    uint64_t bitmap_index = 0;

    for(size_t i = 0; i < usable_count; i++){
        uint64_t start = usable[i].base;
        uint64_t end = start + usable[i].pages * PAGE_SIZE;

        if(addr >= start && addr <= end){
            bitmap_index += (addr - start) / PAGE_SIZE;
            bitmap_clear(bitmap_index);
            return;
        }
        bitmap_index += usable[i].pages;
    }
    print_string("PMM: invalid free");
}

void pmm_print(void){

    uint64_t free = 0;
    for(uint64_t i = 0; i < page_count; i++){
        if(!bitmap_test(i))
            free++;
    }

    print_string("Bitmap phys: ");
    print_hex(bitmap_phys);

    print_string("\nBitmap virt: ");
    print_hex((uint64_t)bitmap);

    print_string("\nBitmap size: ");
    print_uint(bitmap_size);
    print_string(" bytes\nPages: ");
    print_uint(page_count);

    print_string("\nFree pages: ");
    print_uint(free);

    print_string("\nUsed pages: ");
    print_uint(page_count - free);

    print_string("\nBitmap pages: ");
    print_uint((bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE);

    print_string("\nFree memory: ");
    print_uint((free * PAGE_SIZE) /1024 /1024);
    print_string(" MiB\nUsed memory:");
    print_uint(((page_count - free) * PAGE_SIZE) / 1024 / 1024);

    print_string(" MiB\nUsable regions: ");
    print_uint(usable_count);

    for(size_t i = 0; i < usable_count; i++){
        print_string("\nUsable ");
        print_uint(i);

        print_string(": Base=");
        print_hex(usable[i].base);

        print_string(" Pages=");
        print_uint(usable[i].pages);
    }
}