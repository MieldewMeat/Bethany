.SUFFIXES:

QEMUFLAGS := -m 2G

override IMAGE_NAME := bethanys

HOST_CC := cc
HOST_CFLAGS := -g -O2 -pipe
HOST_CPPFLAGS :=
HOST_LDFLAGS :=
HOST_LIBS :=

CODE_POSSITION := code

.PHONY: all
all: $(IMAGE_NAME).iso run

.PHONY: all-hdd
all-hdd: $(IMAGE_NAME).hdd

.PHONY: run rund
run:
	qemu-system-x86_64 \
		-M q35 \
		-cdrom $(IMAGE_NAME).iso \
		-boot d \
		$(QEMUFLAGS)

rund:
	qemu-system-x86_64 \
    	-M q35 \
    	-cdrom $(IMAGE_NAME).iso \
    	-boot d \
    	-no-reboot \
    	-no-shutdown \
    	$(QEMUFLAGS)

edk2-ovmf-bins:
	curl -L https://github.com/osdev0/edk2-ovmf-stable-bins/releases/latest/download/edk2-ovmf-bins.tar.gz | gunzip | tar -xf -

limine-binary/limine:
	rm -rf build/limine-binary
	curl -L https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz | gunzip | tar -xf - -C build
	$(MAKE) -C build/limine-binary \
		CC="$(HOST_CC)" \
		CFLAGS="$(HOST_CFLAGS)" \
		CPPFLAGS="$(HOST_CPPFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		LIBS="$(HOST_LIBS)"

kernel/.deps-obtained:
	./build/kernel/get-deps

.PHONY: kernel
kernel: kernel/.deps-obtained
	$(MAKE) -C build/kernel

move-kernel:
	cp -r kernel build

$(IMAGE_NAME).iso: move-kernel limine-binary/limine kernel

	rm -rf build/iso_root
	mkdir -p build/iso_root/boot
	cp -v build/kernel/bin/kernel build/iso_root/boot/
	mkdir -p build/iso_root/boot/limine
	cp -v limine.conf build/limine-binary/limine-bios.sys build/limine-binary/limine-bios-cd.bin build/limine-binary/limine-uefi-cd.bin build/iso_root/boot/limine/
	mkdir -p build/iso_root/EFI/BOOT
	cp -v build/limine-binary/BOOTX64.EFI build/iso_root/EFI/BOOT/
	cp -v build/limine-binary/BOOTIA32.EFI build/iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		build/iso_root -o $(IMAGE_NAME).iso
	./build/limine-binary/limine bios-install $(IMAGE_NAME).iso

.PHONY: clean
clean:
	$(MAKE) -C kernel clean
	rm -rf iso_root $(IMAGE_NAME).iso $(IMAGE_NAME).hdd

.PHONY: distclean
distclean: clean
	$(MAKE) -C kernel distclean
	rm -rf limine-binary edk2-ovmf-bins