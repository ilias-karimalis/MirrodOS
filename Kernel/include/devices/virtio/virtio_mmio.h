#pragma once

#include <types/error.h>
#include <types/number.h>

struct virtio_mmio_control_registers
{
        u32 magic_value;
        u32 version;
        u32 device_id;
        u32 vendor_id;
        u32 device_features;
        u32 device_features_sel;
        u32 driver_features;
        u32 driver_features_sel;
        u32 queue_sel;
        u32 queue_num_max;
        u32 queue_num;
        u32 queue_ready;
        u32 queue_notify;
        u32 interrupt_status;
        u32 interrupt_ack;
        u32 status;
        u32 queue_desc_low;
        u32 queue_desc_high;
        u32 queue_avail_low;
        u32 queue_avail_high;
        u32 queue_used_low;
        u32 queue_used_high;
        u32 config_generation;
        char config[0];
};

enum virtio_device_status
{
        /// Indicates that the guest OS has found the device and recognized it as a valid virtio device.
        VIRTIO_DEVICE_STATUS_ACKNOWLEDGE = 0x1,
        /// Indicates that the guest OS knows how to drive the device.
        VIRTIO_DEVICE_STATUS_DRIVER = 0x2,
        /// Indicates that the driver is set up and ready to drive the device.
        VIRTIO_DEVICE_STATUS_DRIVER_OK = 0x4,
        /// Indicates that the driver has acknowledged all the features it understands, and feature negotiation is
        /// complete.
        VIRTIO_DEVICE_STATUS_FEATURES_OK = 0x8,
        /// Indicates that something went wrong in the guest, and it has given up on the device. This could be an
        /// internal error, or the driver didn't like the device for some reason, or even a fatal error during device
        /// operation. The driver MUST reset the device before attempting to re-initialize it.
        VIRTIO_DEVICE_STATUS_FAILED = 0x80,
        /// TODO: Do we actually need one of these, or can we use STATUS_FAILED for malformed devices too?
        VIRTIO_DEVICE_MALFORMED = 0x81,
};

enum virtio_device_type
{
        VIRTIO_DEVICE_TYPE_RESERVED = 0x0,
        VIRTIO_DEVICE_TYPE_NETWORK = 0x1,
        VIRTIO_DEVICE_TYPE_BLOCK_DEVICE = 0x2,
        VIRTIO_DEVICE_TYPE_CONSOLE = 0x3,
        VIRTIO_DEVICE_TYPE_ENTROPY_SOURCE = 0x4,
        VIRTIO_DEVICE_TYPE_MEMORY_BALLOON_TRAD = 0x5,
        VIRTIO_DEVICE_TYPE_IOMEMORY = 0x6,
        VIRTIO_DEVICE_TYPE_RPMSG = 0x7,
        VIRTIO_DEVICE_TYPE_SCSI_HOST = 0x8,
        VIRTIO_DEVICE_TYPE_9P_TRANSPORT = 0x9,
        VIRTIO_DEVICE_TYPE_MAC80211_WLAN = 0xA,
        VIRTIO_DEVICE_TYPE_RPROC_SERIAL = 0xB,
        VIRTIO_DEVICE_TYPE_CAIF = 0xC,
        VIRTIO_DEVICE_TYPE_MEMORY_BALLOON = 0xD,
        VIRTIO_DEVICE_TYPE_GPU = 0x10,
        VIRTIO_DEVICE_TYPE_TIMER = 0x11,
        VIRTIO_DEVICE_TYPE_INPUT = 0x12,
        VIRTIO_DEVICE_TYPE_SOCKET = 0x13,
        VIRTIO_DEVICE_TYPE_CRYPTO = 0x14,
        VIRTIO_DEVICE_TYPE_SIGNAL_DIST = 0x15,
        VIRTIO_DEVICE_TYPE_PSTORE = 0x16,
        VIRTIO_DEVICE_TYPE_IOMMU = 0x17,
        VIRTIO_DEVICE_TYPE_MEMORY = 0x18,
};

struct virtio_driver
{
        enum virtio_device_status status;
        enum virtio_device_type type;
};

error_t
virtio_mmio_driver_init(struct virtio_driver* device, void* device_base, size_t device_size);
