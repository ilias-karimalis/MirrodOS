#include <devices/virtio_mmio.h>

#define VIRTIO_MMIO_MAGIC_VALUE 0x74726976
#define VIRTIO_MMIO_VERSION_2 2

error_t
virtio_mmio_initialize_device(void* device_base, struct virtio_device* device)
{
    struct virtio_mmio_control_registers* regs = (struct virtio_mmio_control_registers*)device_base;
    if (regs->magic_value != VIRTIO_MMIO_MAGIC_VALUE) {
        return EC_VIRTIO_INVALID_MAGIC;
    }
    if (regs->version != VIRTIO_MMIO_VERSION_2) {
        return EC_VIRTIO_UNSUPPORTED_VERSION;
    }
    if (regs->device_id == VIRTIO_DEVICE_TYPE_RESERVED) {
        return EC_VIRTIO_INVALID_DEVICE;
    }

    device->type = (enum virtio_device_type)regs->device_id;
    return EC_NOT_IMPLEMENTED;
}