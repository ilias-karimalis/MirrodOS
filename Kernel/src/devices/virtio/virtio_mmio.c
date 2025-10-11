#include <assert.h>
#include <devices/virtio/virtio_mmio.h>
#include <fmt/print.h>

#define VIRTIO_MMIO_MAGIC_VALUE 0x74726976

#define VIRTIO_DEVICE_VERSION_LEGACY 1
#define VIRTIO_DEVICE_VERSION_NEW 2

error_t
virtio_mmio_device_legacy_initialize(struct virtio_driver* device, void* device_base, size_t device_size)
{
        volatile struct virtio_mmio_control_registers* regs =
          (volatile struct virtio_mmio_control_registers*)device_base;
        kprintln(SV("Device Type: {X}"), regs->device_id);
        TODO("Implement Virtio MMIO Legacy Version support.");
}

error_t
virtio_mmio_device_regular_initialize(struct virtio_driver* device, void* device_base, size_t device_size)
{
        TODO("Implement Virtio MMIO New Version support.");
}

error_t
virtio_mmio_driver_init(struct virtio_driver* device, void* device_base, size_t device_size)
{
        struct virtio_mmio_control_registers* regs = (struct virtio_mmio_control_registers*)device_base;
        if (regs->magic_value != VIRTIO_MMIO_MAGIC_VALUE) {
                return EC_VIRTIO_INVALID_MAGIC;
        }
        if (regs->version == VIRTIO_DEVICE_VERSION_LEGACY) {
                return virtio_mmio_device_legacy_initialize(device, device_base, device_size);
        } else if (regs->version == VIRTIO_DEVICE_VERSION_NEW) {
                return virtio_mmio_device_regular_initialize(device, device_base, device_size);
        } else {
                device->status = VIRTIO_DEVICE_MALFORMED;
                return EC_VIRTIO_UNSUPPORTED_VERSION;
        }
        // if (regs->version != VIRTIO_MMIO_VERSION_2) {
        //         kprintln(SV("Unsupported Virtio MMIO version: {D}"), regs->version);
        //         return EC_VIRTIO_UNSUPPORTED_VERSION;
        // }
        // if (regs->device_id == VIRTIO_DEVICE_TYPE_RESERVED) {
        //         return EC_VIRTIO_INVALID_DEVICE;
        // }

        // device->type = (enum virtio_device_type)regs->device_id;
        // kprintln(SV("Found Virtio MMIO Device: Type {X}, Vendor {X}, Base Address: {X}"),
        //          device->type,
        //          regs->vendor_id,
        //          (u64)device_base);
        // return EC_NOT_IMPLEMENTED;
}