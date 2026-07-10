#include "memory.h"

#include "../print_and_stuff/print.h"

static memory_region_t regions[256];
static size_t region_count = 0;

void memory_init(memory_region_t *src, size_t count){
    region_count = count;

    for(size_t i = 0; i < count; i++) regions[i] = src[i];
}

size_t memory_region_count(void){
    return region_count;
}

const memory_region_t *memory_region(size_t index){
    if(index >= region_count)
        return NULL;
    return &regions[index];
}

uint64_t memory_total(void){
    uint64_t total = 0;
    for(size_t i = 0; i < region_count; i++)
        total += regions[i].length;
    return total;
}

uint64_t memory_amount(memory_type_t type){
    uint64_t total = 0;
    for(size_t i = 0; i < region_count; i++)
        if(regions[i].type == type)
            total += regions[i].length;
    return total;
}

uint64_t memory_highest_address(void){
    uint64_t highest = 0;

    for(size_t i = 0; i < memory_region_count(); i++){
        const memory_region_t *r = memory_region(i);

        uint64_t end = r-> base + r->length;

                print_string("Region ");
        print_uint(i);
        print_string(": end = 0x");
        print_hex(end);
        print_char('\n');

        if(end > highest)
            highest = end;
    }

    return highest;
}

void memory_print(void){
    print_string("Memory map:\n\n");

    for(size_t i = 0; i < memory_region_count(); i++){
        const memory_region_t *r = memory_region(i);

        print_string("Region ");
        print_uint(i);

        print_string(": Base=0x");
        print_hex(r->base);

        print_string(" Length=0x");
        print_hex(r->length);

        print_string(" Type=");
        switch(r->type){
            case MEMORY_USABLE:
                print_string("Usable");
                break;

            case MEMORY_RESERVED:
                print_string("Reserved");
                break;

            case MEMORY_ACPI_RECLAIMABLE:
                print_string("ACPI reclamable");
                break;

            case MEMORY_ACPI_NVS:
                print_string("ACPI NVS");
                break;

            case MEMORY_BAD_MEMORY:
                print_string("Bad");
                break;

            case MEMORY_BOOTLOADER_RECLAIMABLE:
                print_string("Bootloader");
                break;

            case MEMORY_KERNEL:
                print_string("Kernel");
                break;

            case MEMORY_FRAMEBUFFER:
                print_string("Framebuffer");
                break;
            case MEMORY_UNKNOWN:
                print_string("Unknow");
                break;

            default:
                print_string("Default");
                break;
        }

        print_char('\n');
    }

    print_char('\n');

    print_string("Total memory: ");
    print_uint(memory_total() / 1024 / 1024);
    print_string(" MiB\n");

    print_string("Usable memory: ");
    print_uint(memory_amount(MEMORY_USABLE) / 1024 / 1024);
    print_string(" MiB\n");

    print_string("Region count: ");
    print_uint(region_count);
    print_char('\n');

    print_string("Reserved memory: ");
    print_uint(memory_amount(MEMORY_RESERVED) / 1024 / 1024);
    print_string(" MiB\n");

    print_string("Unknow memory: ");
    print_uint(memory_amount(MEMORY_UNKNOWN) / 1024 / 1024);
    print_string(" MiB\n");
}