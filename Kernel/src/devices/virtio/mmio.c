#include <assert.h>
#include <devices/virtio/blk.h>
#include <devices/virtio/mmio.h>
#include <fmt/print.h>

#define VIRTIO_MMIO_MAGIC_VALUE 0x74726976

#define VIRTIO_DEVICE_VERSION_LEGACY 1
#define VIRTIO_DEVICE_VERSION_NEW 2

#define LEGACY_OFFSET_MAGIC 0x000
#define LEGACY_OFFSET_VERSION 0x004
#define LEGACY_OFFSET_DEVICE_ID 0x008
#define LEGACY_OFFSET_VENDOR_ID 0x00C
#define LEGACY_OFFSET_DEVICE_FEATURES 0x010
#define LEGACY_OFFSET_DEVICE_FEATURES_SEL 0x014
#define LEGACY_OFFSET_DRIVER_FEATURES 0x020
#define LEGACY_OFFSET_DRIVER_FEATURES_SEL 0x024
#define LEGACY_OFFSET_QUEUE_SEL 0x030
#define LEGACY_OFFSET_QUEUE_NUM_MAX 0x034
#define LEGACY_OFFSET_QUEUE_NUM 0x038
#define LEGACY_OFFSET_QUEUE_ALIGN 0x03C
#define LEGACY_OFFSET_QUEUE_PFN 0x040
#define LEGACY_OFFSET_QUEUE_NOTIFY 0x050
#define LEGACY_OFFSET_INTERRUPT_STATUS 0x060
#define LEGACY_OFFSET_INTERRUPT_ACK 0x064
#define LEGACY_OFFSET_STATUS 0x070
#define LEGACY_OFFSET_CONFIG 0x100

error_t
virtio_mmio_device_legacy_initialize(struct virtio_driver* driver, void* base)
{
        driver->is_legacy = true;
        driver->regs.legacy = (struct virtio_mmio_legacy_registers*)base;
        kprintln(SV("Device Type: {X}"), driver->regs.legacy->device_id);

        switch (driver->regs.legacy->device_id) {
                case VIRTIO_DEVICE_TYPE_RESERVED:
                        driver->type = VIRTIO_DEVICE_TYPE_RESERVED;
                        return EC_VIRTIO_INVALID_DEVICE;
                case VIRTIO_DEVICE_TYPE_BLOCK_DEVICE:
                        driver->type = VIRTIO_DEVICE_TYPE_BLOCK_DEVICE;
                        block_driver_init(driver, base);
                        TODO("Implement Virtio Block Device support.");
                        break;
                default:
                        kprintln(SV("Unsupported Virtio MMIO device type {X}"), driver->regs.legacy->device_id);
                        driver->type = VIRTIO_DEVICE_TYPE_UNSUPPORTED;
                        return EC_VIRTIO_UNSUPPORTED_DEVICE;
        }
        // TODO("Implement Virtio MMIO Legacy Version support.");
        return EC_SUCCESS;
}

error_t
virtio_mmio_device_regular_initialize(struct virtio_driver* driver, void* base)
{
        TODO("Implement Virtio MMIO New Version support.");
}

error_t
virtio_mmio_driver_init(struct virtio_driver* driver, void* device_base, size_t device_size)
{

        struct virtio_mmio_v2_registers* regs = (struct virtio_mmio_v2_registers*)device_base;
        if (regs->magic_value != VIRTIO_MMIO_MAGIC_VALUE) {
                return EC_VIRTIO_INVALID_MAGIC;
        }
        if (regs->version == VIRTIO_DEVICE_VERSION_LEGACY) {
                return virtio_mmio_device_legacy_initialize(driver, device_base);
        } else if (regs->version == VIRTIO_DEVICE_VERSION_NEW) {
                return virtio_mmio_device_regular_initialize(driver, device_base);
        }
        return EC_VIRTIO_UNSUPPORTED_VERSION;
}
