#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include "video/terminal.h"
#include "memory/memory.h"
#include "memory/pmm.h"
#include "print_and_stuff/printf.h"


__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) hcf();

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) hcf();

    if (memmap_request.response == NULL) hcf();

    if(hhdm_request.response == NULL) hcf();

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    
    graphics_init(framebuffer->address, framebuffer->pitch, framebuffer->height, framebuffer->width);
    terminal_init(0xFFFFFF, 0x000000);

    struct limine_memmap_response *memmap = memmap_request.response;

    uint64_t hhdm = hhdm_request.response->offset;

    memory_region_t regions[256];

    for(size_t i = 0; i < memmap->entry_count; i++){
        regions[i].base = memmap->entries[i]->base;
        regions[i].length = memmap->entries[i]->length;

        switch (memmap->entries[i]->type){
            case LIMINE_MEMMAP_USABLE:
                regions[i].type = MEMORY_USABLE;
                break;

            case LIMINE_MEMMAP_RESERVED:
                regions[i].type = MEMORY_RESERVED;
                break;

            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                regions[i].type = MEMORY_ACPI_RECLAIMABLE;
                break;

            case LIMINE_MEMMAP_ACPI_NVS:
                regions[i].type = MEMORY_ACPI_NVS;
                break;

            case LIMINE_MEMMAP_BAD_MEMORY:
                regions[i].type = MEMORY_BAD_MEMORY;
                break;

            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                regions[i].type = MEMORY_BOOTLOADER_RECLAIMABLE;
                break;

            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                regions[i].type = MEMORY_KERNEL;
                break;

            case LIMINE_MEMMAP_FRAMEBUFFER:
                regions[i].type = MEMORY_FRAMEBUFFER;
                break;

            default:
                regions[i].type = MEMORY_UNKNOWN;
                break;
        }
    }

    memory_init(regions, memmap->entry_count);

    print_string("\n\n\n");

    pmm_init(hhdm);

    pmm_print();

    void* p1 = pmm_alloc_page();
    void* p2 = pmm_alloc_page();
    void* p3 = pmm_alloc_page();

    pmm_free_page(p2);

    void* p4 = pmm_alloc_page();

    if(p4 == p2) {
        print_string("Working Fine");
    }else{
        print_char('\n');
        print_hex((uint64_t)p2);
        print_char('\n');
        print_hex((uint64_t)p4);
    }
    
    hcf();
}
