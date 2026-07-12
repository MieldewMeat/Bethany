#include "vmm.h"
#include "../print_and_stuff/print.h"

static page_table_t *pml4 = NULL;
static uint64_t hhdm_offset = 0;

uint64_t vmm_read_cr3(void){
    uint64_t value;

    __asm__ volatile(
        "mov %%cr3, %0"
        : "=r" (value)
    );

    return value;
}

static void invalidate_page(uint64_t virt){
    __asm__ volatile(
        "invlpg (%0)"
        :
        : "r"(virt)
        : "memory"
    );
}

static bool entry_present(page_entry_t entry){
    return entry & 1;
}

static uint64_t entry_address(page_entry_t entry){
    return entry & 0x000FFFFFFFFFF000ULL;
}


static uint16_t pml4_index(uint64_t addr){
    return (addr >> 39) & 0x1FF;
}

static uint16_t pdpt_index(uint64_t addr){
    return (addr >> 30) & 0x1FF;
}

static uint16_t pd_index(uint64_t addr){
    return (addr >> 21) & 0x1FF;
}

static uint16_t pt_index(uint64_t addr){
    return (addr >> 12) & 0x1FF;
}

static uint16_t page_offset(uint64_t addr){
    return addr & 0xFFF;
}

static page_table_t *next_table(page_table_t * table, uint16_t index){
    page_entry_t entry = (*table)[index];

    if(!entry_present(entry)) return NULL;

    return (page_table_t*)(entry_address(entry) + hhdm_offset);
}

static void print_table(page_table_t *table, const char *name){
    if(table == NULL) {
        print_string(name);
        print_string(": NULL\n");
        return;
    }

    print_string(name);
    print_string(": ");

    print_hex((uint64_t)table);

    print_string("\n\n");


    for(size_t i = 0; i < 512; i++){
        page_entry_t entry = (*table)[i];

        if(!entry_present(entry)) continue;

        print_string(name);
        print_string(" [");
        print_uint(i);
        print_string("] -> ");
        print_hex(entry_address(entry));
        print_char('\n');
    }
}


void vmm_init(uint64_t hhdm){
    hhdm_offset = hhdm;

    uint64_t cr3 = vmm_read_cr3();

    pml4 = (page_table_t *) (cr3 + hhdm_offset);
}

void vmm_print(void){

    print_string("Virtual memory manager\n\n");

    print_string("CR3: ");
    print_hex(vmm_read_cr3());

    print_string("\nPML4: ");
    print_hex((uint64_t)pml4);

    print_char('\n');
}

void vmm_debug_address(uint64_t addr){

    print_string("Virtual address: ");
    print_hex(addr);

    print_string("\nPML4 index: ");
    print_uint((uint64_t)pml4_index(addr));

    print_string("\nPDPT index: ");
    print_uint((uint64_t)pdpt_index(addr));

    print_string("\nPD index: ");
    print_uint((uint64_t)pd_index(addr));

    print_string("\nPT index: ");
    print_uint((uint64_t)pt_index(addr));

    print_string("\nOffset index: ");
    print_uint((uint64_t)page_offset(addr));

    print_char('\n');
}

void vmm_walk(uint64_t addr){
    print_string("Walking address: ");
    print_hex(addr);
    print_char('\n');

    page_table_t * pdpt = next_table(pml4, pml4_index(addr));

    print_table(pdpt, "PDPT");

    page_table_t * pd = next_table(pdpt, pdpt_index(addr));

    print_table(pd, "PD");

    page_table_t * pt = next_table(pd, pd_index(addr));

    print_table(pt, "PT");

    if(pt == NULL) return;

    page_entry_t page = (*pt)[pt_index(addr)];

    if(!entry_present(page)){
        print_string("Page not present\n");
        return;
    }

    print_string("Page entry: ");
    print_hex(page);

    print_string("\nPhysical page: ");
    print_hex(entry_address(page));

    print_char('\n');
}

uint64_t vmm_translate(uint64_t virt){

    page_table_t * pdpt = next_table(pml4, pml4_index(virt));

    if(pdpt == NULL) return 0;

    page_table_t * pd = next_table(pdpt, pdpt_index(virt));

    if(pd == NULL) return 0;

    page_table_t * pt = next_table(pd, pd_index(virt));

    if(pt == NULL) return 0;

    page_entry_t pte = (*pt)[pt_index(virt)];

    if(!entry_present(pte)){
        return 0;
    }

    return entry_address(pte) + page_offset(virt);
}


static void clear_table(page_table_t *table){
    for(size_t i = 0; i < 512; i++){
        (*table)[i] = 0;
    }
}

static page_table_t *ensure_table(page_table_t *table, uint16_t index){
    page_entry_t entry = (*table)[index];

    if(entry_present(entry))
        return next_table(table, index);

    void *phys = pmm_alloc_page();

    if(phys == NULL) return NULL;

    page_table_t *new_table = (page_table_t*)((uint64_t)phys + hhdm_offset);

    clear_table(new_table);

    (*table)[index] = (uint64_t) phys | PAGE_PRESENT | PAGE_WRITABLE;

    return new_table;
}

void vmm_map(uint64_t virt, uint64_t phys, uint64_t flags){
    page_table_t *pdpt = ensure_table(pml4, pml4_index(virt));

    if(pdpt == NULL) return;

    page_table_t *pd = ensure_table(pdpt, pdpt_index(virt));

    if(pd == NULL) return;

    page_table_t *pt = ensure_table(pd, pd_index(virt));

    if(pt == NULL) return;

    (*pt)[pt_index(virt)] = phys | flags;

    invalidate_page(virt);
}

void vmm_unmap(uint64_t virt){
    page_table_t *pdpt = ensure_table(pml4, pml4_index(virt));

    if(pdpt == NULL) return;

    page_table_t *pd = ensure_table(pdpt, pdpt_index(virt));

    if(pd == NULL) return;

    page_table_t *pt = ensure_table(pd, pd_index(virt));

    if(pt == NULL) return;

    (*pt)[pt_index(virt)] = 0;

    invalidate_page(virt);
}

bool vmm_is_mapped(uint64_t virt){
    page_table_t *pdpt = ensure_table(pml4, pml4_index(virt));

    if(pdpt == NULL) return 0;

    page_table_t *pd = ensure_table(pdpt, pdpt_index(virt));

    if(pd == NULL) return 0;

    page_table_t *pt = ensure_table(pd, pd_index(virt));

    if(pt == NULL) return 0;

    page_entry_t pte = (*pt)[pt_index(virt)];

    return entry_present(pte);
}

bool vmm_alloc_page(uint64_t virt, uint64_t flags){
    void *phys = pmm_alloc_page();

    if(phys == NULL) return false;

    vmm_map(virt, (uint64_t)phys, flags);
    return true;
}

void vmm_free_page(uint64_t virt){
    uint64_t phys = vmm_translate(virt);

    if(phys == 0) return;

    vmm_unmap(virt);

    pmm_free_page((void *)(phys & ~0xFFFULL));
}