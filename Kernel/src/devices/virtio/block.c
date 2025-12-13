#include "types/error.h"
#include <assert.h>
#include <devices/virtio/block.h>
#include <devices/virtio/mmio.h>
#include <fmt/print.h>

#define countof(Arr) (sizeof(Arr) / sizeof(Arr[0]))

static const struct VirtioDevice_Feature BLK_DEV_FEATURES[] = {
        { .feature_name = S("VIRTIO_BLK_SIZE_MAX"), .feature_bit = VIRTIO_BLK_SIZE_MAX, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_SEG_MAX"), .feature_bit = VIRTIO_BLK_SEG_MAX, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_GEOMETRY"), .feature_bit = VIRTIO_BLK_GEOMETRY, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_READ_ONLY"), .feature_bit = VIRTIO_BLK_READ_ONLY, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_BLK_SIZE"), .feature_bit = VIRTIO_BLK_BLK_SIZE, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_FLUSH"), .feature_bit = VIRTIO_BLK_FLUSH, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_TOPOLOGY"), .feature_bit = VIRTIO_BLK_TOPOLOGY, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_CONFIG_WCE"), .feature_bit = VIRTIO_BLK_CONFIG_WCE, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_BARRIER"), .feature_bit = VIRTIO_BLK_BARRIER, .driver_support = true },
        { .feature_name = S("VIRTIO_BLK_SCSI"), .feature_bit = VIRTIO_BLK_SCSI, .driver_support = true },
};

error_t
virtio_block_device_init(struct VirtioDevice* dev, void* device_base, size_t device_size)
{
        // volatile struct VirtioBlockDevice_Config* conf = (struct VirtioBlockDevice_Config*)dev->regs->config;

        error_t err = virtio_mmio_negotiate_features(dev, BLK_DEV_FEATURES, countof(BLK_DEV_FEATURES));
        if (error_is_err(err)) {
                return err;
        }

        // Perform feature negotiation. For now we don't support any additional features.
        // virtio_mmio_negotiate_features(dev, VIRTIO_BLOCK_DEVICE_FEATURES,
        // countof(VIRTIO_BLOCK_DEVICE_FEATURES)); dev->regs->status |= VIRTIO_STATUS_FEATURES_OK;

        // if (!(dev->regs->status & VIRTIO_STATUS_FEATURES_OK)) {
        //         return EC_VIRTIO_FEATURES_NOT_ACC;
        // }

        TODO();
        return EC_NOT_IMPLEMENTED;
}
