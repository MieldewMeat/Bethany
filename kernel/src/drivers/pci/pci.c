#include "pci.h"

#include "../../io/io.h"
#include "../../print_and_stuff/print.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_DEVICES 128

#define PCI_CMD_IO_SPACE (1 << 0)
#define PCI_CMD_MEM_SPACE (1 << 1)
#define PCI_CMD_BUS_MASTER (1 << 2)

static pci_device_t devices[PCI_MAX_DEVICES];
static size_t device_count = 0;

static uint32_t pci_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
    return (1U<<31)
    | ((uint32_t)bus << 16)
    | ((uint32_t)device << 11)
    | ((uint32_t)function << 8)
    | (offset & 0xFC);
}

uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
    outl(PCI_CONFIG_ADDRESS, pci_address(bus, device, function, offset));

    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset){
    uint32_t value = pci_read32(bus, device, function, offset);

    return (value >> ((offset & 2) * 8)) & 0xFFFF;
}

void pci_write32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value){
    outl(PCI_CONFIG_ADDRESS, pci_address(bus, device, function, offset));
    outl(PCI_CONFIG_DATA, value);
}

void pci_write16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value){
    uint32_t old = pci_read32(bus, device, function, offset);

    uint32_t shift = (offset & 2) * 8;
    uint32_t mask = 0xFFFFU << shift;

    uint32_t new_value = (old & ~mask) | ((uint32_t)value << shift);

    pci_write32(bus, device, function, offset, new_value);
}

void pci_enable_device(pci_device_t *device){
    if(device == NULL) return;

    uint16_t cmd = pci_read16(device->bus, device->device, device->function, 0x04);

    cmd |= PCI_CMD_MEM_SPACE | PCI_CMD_BUS_MASTER;
    cmd |= (1 << 10);

    pci_write16(device->bus, device->device, device->function, 0x04, cmd);
}

static void pci_scan_device(uint8_t bus, uint8_t device, uint8_t function){

    uint16_t vendor = pci_read16(bus, device, function, 0x00);

    if(vendor == 0xFFFF) return;

    uint16_t device_id = pci_read16(bus, device, function, 0x02);

    uint32_t class_reg = pci_read32(bus, device, function, 0x08);

    uint8_t class_code = (class_reg >> 24) & 0xFF;
    uint8_t subclass = (class_reg >> 16) & 0xFF;
    uint8_t prog_if = (class_reg >> 8) & 0xFF;
    uint8_t revision = class_reg & 0xFF;

    if(device_count >= PCI_MAX_DEVICES) return;

    devices[device_count++] = (pci_device_t){
        .bus = bus,
        .device = device,
        .function = function,

        .vendor = vendor,
        .device_id = device_id,

        .class = class_code,
        .subclass = subclass,
        .prog_if = prog_if,
        .revision = revision,
    };
}

static void pci_scan(void){
    for(uint16_t bus = 0; bus < 256; bus++)
        for(uint8_t device = 0; device < 32; device++)
            for(uint8_t function = 0; function < 8; function++) pci_scan_device(bus,device,function);
}

pci_device_t *pci_find(uint8_t class, uint8_t subclass){
    for(size_t i = 0; i < device_count; i++){
        if(devices[i].class == class && devices[i].subclass == subclass) return &devices[i];
    }
    return NULL;
}

pci_device_t *pci_find_vendor(uint16_t vendor, uint16_t device_id){
    for(size_t i = 0; i < device_count; i++){
        if(devices[i].vendor == vendor && devices[i].device_id == device_id) return &devices[i];
    }
    return NULL;
}

void pci_init(void){
    device_count = 0;

    pci_scan();
}

uint64_t pci_bar(pci_device_t *device, uint8_t index){
    if(device == NULL) return 0;
    if(index >= 6) return 0;

    uint8_t offset = 0x10 + index * 4;

    uint32_t low = pci_read32(device->bus, device->device, device->function, offset);

    if(low & 1) return 0;

    if(((low >> 1) & 0b11) == 0b10){
        uint32_t high = pci_read32(device->bus, device->device, device->function, offset + 4);

        return ((uint64_t)high << 32) | (low & ~0xFULL);
    }

    return low & ~0xFULL;
}

