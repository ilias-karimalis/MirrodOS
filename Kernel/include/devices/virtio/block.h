#pragma once

// #include <devices/virtio/mmio.h>
#include <devices/virtio/queue.h>
#include <types/error.h>

struct VirtioDevice;
struct VirtioDevice_Feature;

// --------------------------------------------------------------------------------------------------------------------
// Feature bits:
// --------------------------------------------------------------------------------------------------------------------

enum VirtioBlockDevice_FeatureBit
{
        /// Maximum size of any single segment is in `VirtioBlockDevice_Config.size_max`.
        VIRTIO_BLK_SIZE_MAX = 1,

        /// Maximum number of segments in a request is in `VirtioBlockDevice_Config.seg_max`.
        VIRTIO_BLK_SEG_MAX = 2,

        /// Disk-style geometry is specified in `VirtioBlockDevice_Config.geometry`.
        VIRTIO_BLK_GEOMETRY = 4,

        /// Device is read-only.
        VIRTIO_BLK_READ_ONLY = 5,

        /// Block size of disk is in `VirtioBlockDevice_Config.block_size`.
        VIRTIO_BLK_BLK_SIZE = 6,

        /// Cache flush command support.
        VIRTIO_BLK_FLUSH = 9,

        /// Device exports informatioin on optimal I/O alignment.
        VIRTIO_BLK_TOPOLOGY = 10,

        /// Device can toggle its cache between writeback and writethrough modes.
        VIRTIO_BLK_CONFIG_WCE = 11,

        /// Legacy feature. Device supports request barriers.
        VIRTIO_BLK_BARRIER = 0,

        /// Legacy feature. Device supports scsi packet commands.
        VIRTIO_BLK_SCSI = 7,
};

// static const struct str_view VIRTIO_BLOCK_DEVICE_FEATURE_BIT_STRINGS[] = {
//         [VIRTIO_BLK_SIZE_MAX] = S("VIRTIO_BLK_SIZE_MAX"), [VIRTIO_BLK_SEG_MAX] = S("VIRTIO_BLK_SEG_MAX"),
//         [VIRTIO_BLK_GEOMETRY] = S("VIRTIO_BLK_GEOMETRY"), [VIRTIO_BLK_READ_ONLY] = S("VIRTIO_BLK_READ_ONLY"),
//         [VIRTIO_BLK_BLK_SIZE] = S("VIRTIO_BLK_BLK_SIZE"), [VIRTIO_BLK_FLUSH] = S("VIRTIO_BLK_FLUSH"),
//         [VIRTIO_BLK_TOPOLOGY] = S("VIRTIO_BLK_TOPOLOGY"), [VIRTIO_BLK_CONFIG_WCE] = S("VIRTIO_BLK_CONFIG_WCE"),
//         [VIRTIO_BLK_BARRIER] = S("VIRTIO_BLK_BARRIER"),   [VIRTIO_BLK_SCSI] = S("VIRTIO_BLK_SCSI"),
// };

struct VirtioBlockDevice_Config
{
        /// The total number of 512-byte sectors on the block device.
        u32 capacity;
        u32 size_max;
        u32 seg_max;
        struct VirtioBlockDevice_Geometry
        {
                u16 cylinders;
                u8 heads;
                u8 sectors;
        } geometry;
        u32 block_size;
        struct VirtioBlockDevice_Topology
        {
                u8 physical_block_exp;
                u8 alignment_offset;
                u16 minimum_io_size;
                u32 optimal_io_size;
        } topology;
        u8 writeback;
};

struct VirtioBlockDevice
{
        struct VirtQueue requests;
        // struct VirtioDevice_Feature features[32];
        // u8 feature_count;
};

error_t
virtio_block_device_init(struct VirtioDevice* dev, void* device_base, size_t device_size);
