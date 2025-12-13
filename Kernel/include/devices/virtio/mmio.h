#pragma once

#include "types/str_view.h"
#include <devices/virtio/block.h>
#include <devices/virtio/queue.h>
#include <stdint.h>
#include <types/error.h>
#include <types/number.h>

struct VirtioDevice;

struct MMIO_DeviceRegister
{
        u32 magic;
        u32 version;
        u32 device_id;
        u32 vendor_id;
        u32 device_feat;
        u32 device_feat_sel;
        u32 reserved0[2];
        u32 driver_feat;
        u32 driver_feat_sel;
        u32 reserved1[2];
        u32 queue_sel;
        u32 queue_num_max;
        u32 queue_num;
        u32 reserved2[2];
        u32 queue_ready;
        u32 reserved3[2];
        u32 queue_notify;
        u32 reserved4[3];
        u32 interrupt_status;
        u32 interrupt_ack;
        u32 reserved5[2];
        u32 status;
        u32 reserved6[3];
        u32 queue_desc_lo;
        u32 queue_desc_hi;
        u32 reserved7[2];
        u32 queue_avail_lo;
        u32 queue_avail_hi;
        u32 reserved8[2];
        u32 queue_used_lo;
        u32 queue_used_hi;
        u32 reserved9[21];
        u32 config_generation;
        u32 config[0];
};

// ---------------------------------------------------------------------------------------------------------------------
// Device status field options:
// ---------------------------------------------------------------------------------------------------------------------

/// Indicates that MirrodOS has found the device and recognizes it as a valid virtio device.
static const u32 VIRTIO_STATUS_ACK = 0x1;

/// Indicates that MiirodOS knows how to drive the device.
static const u32 VIRTIO_STATUS_DRIVER = 0x2;

/// Indicates that something went wrong in MirrodOS, and has given up on the device. This could be an internal error, or
/// the driver didn't like the device for some reasonm ir even a fatal error during device operation.
static const u32 VIRITO_STATUS_FAILED = 128;

/// Inidicates that the driver has acknowledged all the features it understands, and feature negotiation is complete.
static const u32 VIRTIO_STATUS_FEATURES_OK = 8;

/// Indicates that the driver is set up and ready to drive the device.
static const u32 VIRTIO_STATUS_DRIVER_OK = 4;

/// Indicates that the device has experienced an error from which it can't recover.
static const u32 VIRTIO_STATUS_DEVICE_NEEDS_RESET = 64;

/// Begins devices initialization by resetting the device.
static const u32 VIRTIO_STATUS_PERFORM_RESET = 0;

// --------------------------------------------------------------------------------------------------------------------
// Device Identifiers:
// --------------------------------------------------------------------------------------------------------------------

enum VirtioDevice_Identifier
{
        VIRTIO_DEVICE_RESERVED = 0,
        VIRTIO_DEVICE_NET = 1,
        VIRTIO_DEVICE_BLOCK = 2,
        VIRTIO_DEVICE_CONSOLE = 3,
        VIRTIO_DEVICE_ENTROPY = 4,
        VIRTIO_DEVICE_BALLOON_TRAD = 5,
        VIRTIO_DEVICE_IOMEM = 6,
        VIRTIO_DEVICE_RPMSG = 7,
        VIRTIO_DEVICE_SCSI = 8,
        VIRTIO_DEVICE_9P = 9,
        VIRTIO_DEVICE_MAC80211 = 10,
        VIRTIO_DEVICE_RPROC = 11,
        VIRTIO_DEVICE_CAIF = 12,
        VIRTIO_DEVICE_BALLOON = 13,
        VIRTIO_DEVICE_GPU = 14,
        VIRTIO_DEVICE_TIMER = 15,
        VIRTIO_DEVICE_INPUT = 16,
};

static const struct str_view VIRTIO_DEVICE_ID_STRINGS[] = {
        [VIRTIO_DEVICE_RESERVED] = S("VIRTIO_DEVICE_RESERVED"),
        [VIRTIO_DEVICE_NET] = S("VIRTIO_DEVICE_NET"),
        [VIRTIO_DEVICE_BLOCK] = S("VIRTIO_DEVICE_BLOCK"),
        [VIRTIO_DEVICE_CONSOLE] = S("VIRTIO_DEVICE_CONSOLE"),
        [VIRTIO_DEVICE_ENTROPY] = S("VIRTIO_DEVICE_ENTROPY"),
        [VIRTIO_DEVICE_BALLOON_TRAD] = S("VIRTIO_DEVICE_BALLOON_TRAD"),
        [VIRTIO_DEVICE_IOMEM] = S("VIRTIO_DEVICE_IOMEM"),
        [VIRTIO_DEVICE_RPMSG] = S("VIRTIO_DEVICE_RPMSG"),
        [VIRTIO_DEVICE_SCSI] = S("VIRTIO_DEVICE_SCSI"),
        [VIRTIO_DEVICE_9P] = S("VIRTIO_DEVICE_9P"),
        [VIRTIO_DEVICE_MAC80211] = S("VIRTIO_DEVICE_MAC80211"),
        [VIRTIO_DEVICE_RPROC] = S("VIRTIO_DEVICE_RPROC"),
        [VIRTIO_DEVICE_CAIF] = S("VIRTIO_DEVICE_CAIF"),
        [VIRTIO_DEVICE_BALLOON] = S("VIRTIO_DEVICE_BALLOON"),
        [VIRTIO_DEVICE_GPU] = S("VIRTIO_DEVICE_GPU"),
        [VIRTIO_DEVICE_TIMER] = S("VIRTIO_DEVICE_TIMER"),
        [VIRTIO_DEVICE_INPUT] = S("VIRTIO_DEVICE_INPUT"),
};

// --------------------------------------------------------------------------------------------------------------------
// Device features
// --------------------------------------------------------------------------------------------------------------------

struct VirtioDevice_Feature
{
        /// The feature name as defined by the specification.
        struct str_view feature_name;
        /// The bit index representing the feature.
        u8 feature_bit;
        /// Marks whether the driver implementation supports this feature.
        bool driver_support;
};

error_t
virtio_mmio_negotiate_features(struct VirtioDevice* dev,
                               const struct VirtioDevice_Feature* features,
                               size_t features_count);

enum VirtioGenericDevice_FeatureBit
{
        /// Negotiation of this feature indicates that we can use `VirtQueueDescriptor`s with the
        /// `VIRTQUEUE_DESCRIPTOR_INDIRECT` flag set. Specification reference 2.4.5.3
        VIRTIO_GEN_IND_DESC = 28,
        /// This feature enables the `used_event` and `avail_event` fields in the `VirtQueueAvailable` and
        /// `VirtQueueUsed` structures respectively. Specification reference 2.4.7 and 2.4.8
        VIRTIO_GEN_EVENT_IDX = 29,
        /// This indicates compliance with the virtio-v1.0 specification.
        VIRTIO_GEN_VERSION_1 = 32,
};

static const struct VirtioDevice_Feature VIRTIO_GEN_DEVICE_FEATURES[] = {
        { .feature_name = S("VIRTIO_GEN_IND_DESC"), .feature_bit = VIRTIO_GEN_IND_DESC, .driver_support = true },
        { .feature_name = S("VIRTIO_GEN_EVENT_IDX"), .feature_bit = VIRTIO_GEN_EVENT_IDX, .driver_support = true },
        { .feature_name = S("VIRTIO_GEN_VERSION_1"), .feature_bit = VIRTIO_GEN_VERSION_1, .driver_support = true },
};

// --------------------------------------------------------------------------------------------------------------------
//
// --------------------------------------------------------------------------------------------------------------------

struct VirtioDevice
{
        volatile struct MMIO_DeviceRegister* regs;

        /// Technically the virtio specification allows for up to 64 features, but as far as I understand there's no
        /// actual device that uses these extra bits.
        bool features[32];

        enum VirtioDevice_Identifier type;
        union VirtioDeviceType
        {
                struct VirtioBlockDevice blk;
        } u;
};

error_t
virtio_mmio_device_init(struct VirtioDevice* dev, void* device_base, size_t device_size);

error_t
virtio_mmio_virtqueue_init(struct VirtioDevice* vdev, struct VirtQueue* queue, u32 queue_index, u32 queue_size);
