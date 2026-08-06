#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../../drivers/pci/pci.h"
#include "../block/block.h"

typedef struct{
    uint16_t mqes;

    uint8_t timeout;

    uint8_t doorbell_stride;

    uint8_t min_page_shift;
    uint8_t max_page_shift;

}nvme_capabilities_t;

typedef struct{
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;

    uint32_t nsid;

    uint64_t reserved1;

    uint64_t metadata;

    uint64_t prp1;
    uint64_t prp2;

    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;

} __attribute__((packed)) nvme_command_t;

typedef struct{
    uint32_t result;

    uint32_t reserved;

    uint16_t sq_head;
    uint16_t sq_id;

    uint16_t command_id;
    uint16_t status;

} __attribute__((packed)) nvme_completion_t;

typedef struct{
    uint64_t nsze;
    uint64_t ncap;
    uint64_t nuse;

    uint8_t nsfeat;
    uint8_t nlbaf;
    uint8_t flbas;
    uint8_t mc;

    uint8_t dpc;
    uint8_t dps;
    uint8_t nmic;
    uint8_t rescap;

    uint8_t fpi;
    uint8_t dlfeat;

    uint16_t nawun;
    uint16_t nawupf;
    uint16_t nacwu;
    uint16_t nabsn;
    uint16_t nabo;
    uint16_t nabspf;

    uint16_t noiob;

    uint8_t reserved[80];

    struct{
        uint16_t ms;
        uint8_t lbads;
        uint8_t rp;
    } lbaf[16];
} __attribute__((packed)) nvme_identify_namespace_t;

typedef struct{
    pci_device_t *pci;

    uint64_t bar0;
    volatile uint8_t *mmio;

    uint64_t cap;
    uint32_t version;
    uint32_t csts;

    nvme_capabilities_t caps;

    nvme_command_t *admin_sq;
    nvme_completion_t *admin_cq;

    uint64_t admin_sq_phys;
    uint64_t admin_cq_phys;

    uint16_t sq_tail;
    uint16_t cq_head;
    uint8_t cq_phase;
    uint16_t admin_queue_size;

    void *identify_buffer;
    uint64_t identify_buffer_phys;

    block_device_t block;

    nvme_command_t *io_sq;
    nvme_completion_t *io_cq;

    uint64_t io_sq_phys;
    uint64_t io_cq_phys;
    uint16_t io_sq_tail;
    uint16_t io_cq_head;
    uint16_t io_queue_size;
} nvme_controller_t;

bool nvme_init(void);

nvme_controller_t *nvme_controller(void);

bool nvme_read(uint64_t lba, uint32_t blocks, void *buffer);
bool nvme_write(uint64_t lba, uint32_t blocks, void *buffer);