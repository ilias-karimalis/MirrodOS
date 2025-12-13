#include "kvspace.h"
#include <assert.h>
#include <devices/virtio/block.h>
#include <devices/virtio/mmio.h>
#include <fmt/print.h>
#include <types/error.h>

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

#define VIRTIO_MAGIC_VALUE 0x74726976
#define VIRTIO_MODERN_VERSION 0x2

#define countof(Arr) (sizeof(Arr) / sizeof(Arr[0]))

error_t
virtio_mmio_device_init(struct VirtioDevice* dev, void* device_base, size_t device_size)
{
        volatile struct MMIO_DeviceRegister* regs = device_base;

        if (regs->magic != 0x74726976) {
                return EC_VIRTIO_INVALID_MAGIC;
        }
        if (regs->version != VIRTIO_MODERN_VERSION) {
                return EC_VIRTIO_UNSUPPORTED_VERSION;
        }
        if (regs->device_id == 0) {
                return EC_VIRTIO_INVALID_DEVICE;
        }

        // Setup the feature bitmask so that it's ready for feature negotiation.
        for (size_t i = 0; i < 32; i++) {
                dev->features[i] = false;
        }

        // First step of initialization, reset the device.
        regs->status = VIRTIO_STATUS_PERFORM_RESET;
        regs->status |= VIRTIO_STATUS_ACK;
        regs->status |= VIRTIO_STATUS_DRIVER;

        // Delegate the device setup to the device type sepecific initialization functions.
        dev->regs = regs;
        switch (regs->device_id) {
                case VIRTIO_DEVICE_BLOCK:
                        return virtio_block_device_init(dev, device_base, device_size);
                        break;
                default:
                        kprintln(S("Unsupported virtio device ID: {V}"), VIRTIO_DEVICE_ID_STRINGS[regs->device_id]);
                        return EC_VIRTIO_INVALID_DEVICE;
        }
        __builtin_unreachable();
}

error_t
virtio_mmio_negotiate_features(struct VirtioDevice* dev,
                               const struct VirtioDevice_Feature* features,
                               size_t features_count)
{
        // We only support the low 32 bits, so we only need to read from the lowest bank of device features;
        dev->regs->device_feat_sel = 0;
        u32 device_features = dev->regs->device_feat;
        u32 driver_features = 0;

        // Loop through the provided features and the generic device features, checking if the device supports them and
        // marking them as on or off in the device features bitmap.
        for (size_t i = 0; i < features_count; i++) {
                const struct VirtioDevice_Feature feat = features[i];
                if (feat.driver_support && (device_features & (1 << feat.feature_bit))) {
                        kprintln(feat.feature_name);
                        driver_features |= 1 << feat.feature_bit;
                        dev->features[feat.feature_bit] = true;
                }
        }

        for (size_t i = 0; i < countof(VIRTIO_GEN_DEVICE_FEATURES); i++) {
                const struct VirtioDevice_Feature feat = VIRTIO_GEN_DEVICE_FEATURES[i];
                if (feat.driver_support && (device_features & (1 << feat.feature_bit))) {
                        kprintln(feat.feature_name);
                        driver_features |= 1 << feat.feature_bit;
                        dev->features[feat.feature_bit] = true;
                }
        }
        return EC_SUCCESS;
}

error_t
virtio_mmio_virtqueue_init(struct VirtioDevice* vdev, struct VirtQueue* queue, u32 queue_index, u32 queue_size)
{
        // Assumes that feature negotiation has already happened.
        // The specification for this is 4.2.3.2

        vdev->regs->queue_sel = queue_index;
        if (vdev->regs->queue_ready != 0) {
                return EC_VIRTQUEUE_IN_USE;
        }
        u32 max_queue_size = vdev->regs->queue_num_max;
        if (max_queue_size == 0) {
                return EC_VIRTQUEUE_UNAVAIL;
        } else if (max_queue_size < queue_size) {
                kprintln(S("[Warning]: VirtQueue requested size ({X}) is larger than device supported size ({X}). "
                           "Defaulting to device maximum."),
                         queue_size,
                         max_queue_size);
                queue_size = max_queue_size;
        }

        error_t err = virtqueue_create(queue, queue_size);
        if (error_is_err(err)) {
                return err;
        }

        paddr_t queue_desc_pa = kernel_hhdm_virt_to_phys((void*)queue->descriptor_table);
        paddr_t avail_ring_pa = kernel_hhdm_virt_to_phys((void*)queue->avail_ring);
        paddr_t used_ring_pa = kernel_hhdm_virt_to_phys((void*)queue->used_ring);

        vdev->regs->queue_num = queue_size;
        vdev->regs->queue_desc_lo = (u32)(queue_desc_pa & 0xFFFFFFFF);
        vdev->regs->queue_desc_hi = (u32)((queue_desc_pa >> 32) & 0xFFFFFFFF);
        vdev->regs->queue_avail_lo = (u32)(avail_ring_pa & 0xFFFFFFFF);
        vdev->regs->queue_avail_hi = (u32)((avail_ring_pa >> 32) & 0xFFFFFFFF);
        vdev->regs->queue_used_lo = (u32)(used_ring_pa & 0xFFFFFFFF);
        vdev->regs->queue_used_hi = (u32)((used_ring_pa >> 32) & 0xFFFFFFFF);

        vdev->regs->queue_ready = 0x1;
        return EC_SUCCESS;
}
