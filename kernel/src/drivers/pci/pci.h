#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct{
    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor;
    uint16_t device_id;

    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
} pci_device_t;

void pci_init(void);

pci_device_t *pci_find(uint8_t class, uint8_t subclass);
pci_device_t *pci_find_vendor(uint16_t vendor, uint16_t device_id);
uint64_t pci_bar(pci_device_t *device, uint8_t bar);

uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

void pci_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);

void pci_enable_device(pci_device_t *device);