#include "nvme.h"

#include "../../print_and_stuff/print.h"
#include "../../memory/vmm.h"
#include "../../string/string.h"

#define NVME_REG_CAP 0x00
#define NVME_REG_VS 0x08
#define NVME_REG_INTMS 0x0C
#define NVME_REG_CC 0x14
#define NVME_REG_CSTS 0x1C
#define NVME_REG_AQA 0x24
#define NVME_REG_ASQ 0x28
#define NVME_REG_ACQ 0x30
#define NVME_REG_SQ0TDBL 0x1000

static nvme_controller_t controller;


static uint32_t nvme_read32(uint32_t offset){
    return *(volatile uint32_t *)(controller.mmio + offset);
}

static uint64_t nvme_read64(uint32_t offset){
    return *(volatile uint64_t *)(controller.mmio + offset);
}

static void nvme_write32(uint32_t offset, uint32_t value){
    *(volatile uint32_t *)(controller.mmio + offset) = value;
}

static void nvme_write64(uint32_t offset, uint64_t value){
    *(volatile uint64_t *)(controller.mmio + offset) = value;
}

static uint32_t nvme_doorbell_offset(uint16_t qid, bool is_cq){
    uint32_t stride = 4u << controller.caps.doorbell_stride;

    return NVME_REG_SQ0TDBL + (2u * qid + (is_cq ? 1u : 0u)) * stride;
}

static void nvme_parse_cap(void){
    uint64_t cap = controller.cap;

    controller.caps.mqes = (cap & 0xFFFF) + 1;
    controller.caps.timeout = (cap >> 24) & 0xFF;
    controller.caps.doorbell_stride = (cap >> 32) & 0xF;
    controller.caps.min_page_shift = 12 + ((cap >> 48) & 0xF);
    controller.caps.max_page_shift = 12 + ((cap >> 52) & 0xF);
}

static bool nvme_disable_controller(){
    uint32_t cc = nvme_read32(NVME_REG_CC);

    if(!(cc & 1)) return true;

    cc &= ~1;
    nvme_write32(NVME_REG_CC, cc);

    while(nvme_read32(NVME_REG_CSTS) & 1) __asm__ volatile("pause");

    return true;
}

static bool nvme_controller_start(void){
    uint32_t cc = 0;

    cc |= (6 << 16);
    cc |= (4 << 20);
    cc |= 1;

    nvme_write32(NVME_REG_CC, cc);

    while(!(nvme_read32(NVME_REG_CSTS) & 1)) __asm__ volatile("pause");

    return true;
}

static bool nvme_admin_queue_init(void){
    void *sq_phys = pmm_alloc_page();
    void *cq_phys = pmm_alloc_page();

    if(!sq_phys || !cq_phys) return false;

    controller.admin_sq_phys = (uint64_t)sq_phys;
    controller.admin_cq_phys = (uint64_t)cq_phys;

    controller.admin_sq = (nvme_command_t *)vmm_phys_to_virt(controller.admin_sq_phys);
    controller.admin_cq = (nvme_completion_t *)vmm_phys_to_virt(controller.admin_cq_phys);

    memset(controller.admin_sq, 0, PAGE_SIZE);
    memset(controller.admin_cq, 0, PAGE_SIZE);

    controller.sq_tail = 0;
    controller.cq_head = 0;
    controller.admin_queue_size = 64;

    return true;
}



static bool nvme_enable_controller(void){
    if(!nvme_disable_controller()) return false;

    uint32_t aqa = (((64 - 1) & 0xFFF) << 16) | ((64 -1) & 0xFFF);

    nvme_write32(0x24,aqa);
    nvme_write64(0x28,controller.admin_sq_phys);
    nvme_write64(0x30,controller.admin_cq_phys);

    return true;
}

static bool nvme_wait_completion(uint16_t cid){

    nvme_completion_t *cqe = &controller.admin_cq[controller.cq_head];

    while(cqe->command_id != cid) __asm__ volatile("pause");

    uint16_t status = cqe->status >> 1;

    if(status != 0){
        print_string("NVMe Error: ");
        print_hex(status);

        return false;
    }

    controller.cq_head++;
    if(controller.cq_head == controller.admin_queue_size) controller.cq_head = 0;

    nvme_write32(nvme_doorbell_offset(0, true), controller.cq_head);

    return true;
}

static bool nvme_admin_submit(nvme_command_t *command){
    uint16_t slot = controller.sq_tail;

    controller.admin_sq[slot] = *command;

    controller.sq_tail++;

    if(controller.sq_tail == controller.admin_queue_size) controller.sq_tail = 0;

    nvme_write32(nvme_doorbell_offset(0, false), controller.sq_tail);

    return nvme_wait_completion(command->command_id);
}

static bool nvme_alloc_identify_buffer(void){
    if(controller.identify_buffer != NULL) return true;

    void *phys = pmm_alloc_page();
    if(!phys) return false;

    controller.identify_buffer_phys = (uint64_t)phys;
    controller.identify_buffer = vmm_phys_to_virt(controller.identify_buffer_phys);

    return true;
}

static bool nvme_identify_controller(void){

    memset(controller.identify_buffer, 0, PAGE_SIZE);

    nvme_command_t cmd = {0};

    cmd.opcode = 0x06;
    cmd.command_id = 1;
    cmd.nsid = 0;
    cmd.prp1 = controller.identify_buffer_phys;
    cmd.cdw10 = 1;

    return nvme_admin_submit(&cmd);
}

static bool nvme_identify_namespace(uint32_t nsid){
    memset(controller.identify_buffer, 0, PAGE_SIZE);

    nvme_command_t cmd = {0};

    cmd.opcode = 0x06;
    cmd.command_id = 2;
    cmd.nsid = nsid;
    cmd.prp1 = controller.identify_buffer_phys;
    cmd.cdw10 = 0;

    return nvme_admin_submit(&cmd);
}

static bool nvme_io_queue_init(void){
    void *sq= pmm_alloc_page();
    void *cq= pmm_alloc_page();

    if(!sq || !cq) return false;

    controller.io_sq_phys = (uint64_t)sq;
    controller.io_cq_phys = (uint64_t)cq;

    controller.io_sq = vmm_phys_to_virt(controller.io_sq_phys);
    controller.io_cq = vmm_phys_to_virt(controller.io_cq_phys);

    memset(controller.io_sq, 0, PAGE_SIZE);
    memset(controller.io_cq, 0, PAGE_SIZE);

    controller.io_sq_tail = 0;
    controller.io_cq_head = 0;

    controller.io_queue_size = 64;

    return true;
}

static bool nvme_create_io_cq(void){
    nvme_command_t cmd = {0};

    cmd.opcode = 0x05;
    cmd.command_id = 3;
    cmd.prp1 = controller.io_cq_phys;

    cmd.cdw10 = ((controller.io_queue_size - 1) << 16) | 1;

    cmd.cdw11 = 1 | (1 << 1);

    return nvme_admin_submit(&cmd);
}

static bool nvme_create_io_sq(void){
    nvme_command_t cmd = {0};

    cmd.opcode = 0x01;
    cmd.command_id = 4;
    cmd.prp1 = controller.io_sq_phys;

    cmd.cdw10 = ((controller.io_queue_size - 1) << 16) | 1;

    cmd.cdw11 = 1 | (1 << 16);

    return nvme_admin_submit(&cmd);
}

static bool nvme_io_wait_completion(uint16_t cid){
    nvme_completion_t *cqe = &controller.io_cq[controller.io_cq_head];

    while(cqe->command_id != cid) __asm__ volatile("pause");

    uint16_t status = cqe->status >> 1;

    if(status != 0){
        print_string("NVMe IO Error: ");
        print_hex(status);
        print_char('\n');
        return false;
    }

    controller.io_cq_head++;
    if(controller.io_cq_head == controller.io_queue_size) controller.io_cq_head = 0;

    nvme_write32(nvme_doorbell_offset(1, true), controller.io_cq_head);

    return true;
}

static bool nvme_io_submit(nvme_command_t *command){
    uint16_t slot = controller.io_sq_tail;

    controller.io_sq[slot] = *command;

    controller.io_sq_tail++;
    if(controller.io_sq_tail == controller.io_queue_size) controller.io_sq_tail = 0;

    nvme_write32(nvme_doorbell_offset(1, false), controller.io_sq_tail);

    return nvme_io_wait_completion(command->command_id);
}

bool nvme_read(uint64_t lba, uint32_t blocks, void *buffer){

    if(buffer == NULL || blocks == 0) return false;

    if(blocks > 0x10000) return false;

    if((uint64_t)blocks * controller.block.block_size > PAGE_SIZE) return false;
    nvme_command_t cmd = {0};

    cmd.opcode = 0x02;
    cmd.command_id = 5;
    cmd.nsid = 1;
    cmd.prp1 = vmm_virt_to_phys(buffer);

    cmd.cdw10 = (uint32_t)lba;
    cmd.cdw11 = (uint32_t)(lba >> 32);
    cmd.cdw12 = blocks - 1;

    return nvme_io_submit(&cmd);
}

bool nvme_write(uint64_t lba, uint32_t blocks, void *buffer){

    if(buffer == NULL || blocks == 0) return false;

    if(blocks > 0x10000) return false;

    if((uint64_t)blocks * controller.block.block_size > PAGE_SIZE) return false;

    nvme_command_t cmd = {0};

    cmd.opcode = 0x01;
    cmd.command_id = 6;
    cmd.nsid = 1;
    cmd.prp1 = vmm_virt_to_phys(buffer);

    cmd.cdw10 = (uint32_t)lba;
    cmd.cdw11 = (uint32_t)(lba >> 32);
    cmd.cdw12 = blocks - 1;

    return nvme_io_submit(&cmd);
}

bool nvme_flush(void){
    nvme_command_t cmd = {0};

    cmd.opcode = 0x00;
    cmd.command_id = 7;

    cmd.nsid = 0xFFFFFFFF;

    return nvme_io_submit(&cmd);
}

nvme_controller_t *nvme_controller(void){
    return &controller;
}

bool nvme_init(void){

    controller.pci = pci_find(0x01, 0x08);
    if(controller.pci == NULL) return false;

    pci_enable_device(controller.pci);
    
    controller.bar0 = pci_bar(controller.pci, 0);

    vmm_map_range(0xFFFF900000000000ULL, controller.bar0, 0x2000, PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE | PAGE_WRITE_TROUGH);

    controller.mmio = (volatile uint8_t *)0xFFFF900000000000ULL;

    controller.cap = nvme_read64(0x00);
    controller.version = nvme_read32(0x08);
    controller.csts = nvme_read32(0x1C);

    nvme_parse_cap();

    nvme_write32(NVME_REG_INTMS, 0xFFFFFFFF);

    if(!nvme_admin_queue_init()) return false;
    if(!nvme_enable_controller()) return false;
    if(!nvme_controller_start()) return false;

    if(!nvme_alloc_identify_buffer())return false;

    if(!nvme_identify_controller()) return false;
    if(!nvme_identify_namespace(1)) return false;

    if(!nvme_io_queue_init()) return false;
    if(!nvme_create_io_cq()) return false;
    if(!nvme_create_io_sq()) return false;

    nvme_identify_namespace_t *ns = (nvme_identify_namespace_t *)controller.identify_buffer;

    uint8_t format = ns->flbas & 0xF;

    controller.block.block_size = 1U << ns->lbaf[format].lbads;
    controller.block.block_count = ns->nsze;

    controller.block.read = nvme_read;
    controller.block.write = nvme_write;
    controller.block.flush = nvme_flush;

    controller.block.driver = &controller;

    if(!block_register(&controller.block)) return false;

    return true;
}