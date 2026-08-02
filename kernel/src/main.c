#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>

#include "video/graphics.h"
#include "video/terminal.h"

#include "print_and_stuff/print.h"

#include "memory/memory.h"
#include "memory/pmm.h"
#include "memory/vmm.h"

#include "interruptions/idt.h"
#include "interruptions/pic.h"

#include "io/io.h"

#include "timer/pit.h"

#include "drivers/keyboard/keyboard.h"

#include "memory/heap.h"

#include "scheduler/task.h"
#include "scheduler/scheduler.h"

#include "sync/mutex.h"
#include "sync/semaphore.h"
#include "sync/event.h"
#include "sync/queue.h"

#include "filesystem/vfs.h"

#define LAPIC_LVT0 (0x350 / 4)

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

    volatile uint32_t *lapic = (volatile uint32_t *)(hhdm + 0xFEE00000);

    memory_init(regions, memmap->entry_count);

    pmm_init(hhdm);

    vmm_init(hhdm);

    idt_init();
    pic_init();
    pit_init(100);

    keyboard_init();

    vmm_map(hhdm + 0xFEE00000, 0xFEE00000, PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE);
    lapic[LAPIC_LVT0] = 0x00008700;

    __asm__ volatile("sti");

    heap_init();

    scheduler_init();

    vfs_init();

    vfs_node_t *home = vfs_create(vfs_root(), "home", FS_DIRECTORY);

    vfs_create(home, "notes.txt", FS_FILE);
    vfs_create(home, "music", FS_DIRECTORY);
    vfs_create(vfs_root(), "bin", FS_DIRECTORY);
    vfs_create(vfs_root(), "etc", FS_DIRECTORY);


    char msg[] = "Hello World!";

    vfs_write(vfs_find(home,"notes.txt"), msg, sizeof(msg));

    vfs_move(home, vfs_root());

    vfs_node_t *chome = vfs_copy(home, vfs_root());

    vfs_delete(vfs_find(home, "notes.txt"));

    vfs_delete(home);

    vfs_rename(chome, "home");

    vfs_dump(vfs_root());
    
    char msg2[] = "Changed";

    vfs_write(vfs_find(chome,"notes.txt"), msg2, sizeof(msg2));

    char buffer[32];

    size_t n = vfs_read(vfs_find(chome, "notes.txt"), buffer, sizeof(buffer), 0);

    print_char('\n');
    print_string(buffer);
    print_string("\nSize: ");
    print_uint(n);

    scheduler_schedule();

    hcf();
}
