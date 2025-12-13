/// VirtQueue implementation, conforming to the virtio 1.0 specification.
/// Currently does not provide support for any additional features such as:
/// 	1. VIRTIO_GEN_EVENT_IDX
///		2. VIRTQUEUE_DESCRIPTOR_INDIRECT
#pragma once

#include <types/error.h>
#include <types/number.h>

static const u16 VIRTQUEUE_QUEUE_SIZE_MAX = 32768;
static const u16 VIRTQUEUE_QUEUE_SIZE_MIN = 32;

///
/// Specification Information:
/// 2.4.5.3.1: A driver must not set both Indirect and Next in flags.
enum VirtQueue_DescriptorFlag
{
        /// The buffer is continued via the next field.
        VIRTQUEUE_DESCRIPTOR_NEXT = 1,
        /// The buffer is device write-only, if thiis buit is not set, it is read-only.
        VIRTQUEUE_DESCRIPTOR_WRITE = 2,
        /// The buffer represents a list of indirect buffer descriptors.
        /// Specification reference: 2.4.5.3
        VIRTQUEUE_DESCRIPTOR_INDIRECT = 4,
};

struct VirtQueueDescriptor
{
        /// Phyical addrsess of the buffer referred to by this descriptor.
        u64 address;
        /// Length of the buffer referred to by this descriptor.
        u32 length;
        /// The flags for this descriptor, must be a combination of the options in `VirtQueue_DescriptorFlag`.
        u16 flags;
        /// The descriptor table index of the next buffer in the chain, if the `VIRTQUEUE_DESCRIPTOR_NEXT` flag is set.
        u16 next;
};

enum VirtQueue_AvailableFlag
{
        /// If the `VIRTIO_GEN_EVENT_IDX` feature is not negotiated, then thiis flag offers a crude mechanism for the
        /// driver to inform the device that it doesn't want interrupts when buffers are used. Otherwise `used_event`
        /// specifies how far the device can progress before interrupting.`
        VIRTQUEUE_AVAIL_NO_INT = 1,
};

/// We use the available ring to offer buffers to the device: each ring entry refers to the head of a descriptor
/// chain. This structure is only written to by the driver and read from by the device.
struct VirtQueueAvailable
{
        ///
        u16 flags;
        /// Indicates where the driver would put the next descriptor entry in the ring (modulo the QueueSize).
        u16 index;
        /// The true size of this buffer is a function of the QueueSize.
        u16 ring[0];
        /// The used_event is only present if the `VIRTIO_GEN_EVENT_IDX` feature has been negotiated.
        // u16 used_event;
};

struct VirtQueueUsed_Entry
{
        /// Index of the start of used descriptor chain.
        u32 index;
        /// Total length of the descriptor chain which was used (written to).
        u32 length;
};

/// The used ring is where the deviice returns buffers once it's done with them: It's only written to by the device, and
/// read from by the driver.
struct VirtQueueUsed
{
        ///
        u16 flags;
        /// Indicates where the driver (??? copy pasted from the v1.0 specification, but I think this should be
        /// device???) would put the next descriptor entry in the ring (modulo the queue size).
        u16 index;
        /// Each entry in the ring is a pair, index indicates the head entry of the descriptor chain describing the
        /// buffer (this must match an entry placed in the available ring by the driver earlier), and length is the
        /// total number of bytes that were wrtten into this buffer.
        ///
        /// The driver must not make assumptions about data in device-writable buffers beyond the first length bytes,
        /// this data should be ignored, in particular it should be masked if the buffer is directly handed off to a
        /// untrusted userspace application.
        struct VirtQueueUsed_Entry ring[0];
        //// The avail_event is only present if the `VIRTIO_GEN_EVENT_IDX` feature has been negotiated.
        // u16 avail_event;
};

struct VirtQueue
{
        /// The queue size corresponbds to the maximum number of buffers in the virt queue (hence also the length of the
        /// descriptor table). This must be a power of 2 and the maximum queue size is 32768. More constraints on this
        /// maximum may be placed by the specific transport.
        u16 queue_size;

        /// We maintain a list of currently unsued free queue descriptors so that we know which descriptor to use next
        /// when we want to make a request.
        /// Note that for descriptors on this list the only valid component of their state is `next`.
        u16 free_descriptors;

        /// What other arguments do we need in here?

        /// The buffer of VirtQueue descriptors for this VirtQueue.
        volatile struct VirtQueueDescriptor* descriptor_table;
        /// The available ring for this VirtQueue.
        volatile struct VirtQueueAvailable* avail_ring;
        /// The used ring for this VirtQueue.
        volatile struct VirtQueueUsed* used_ring;
};

error_t
virtqueue_create(struct VirtQueue* queue, u16 queue_size);

error_t
virtqueue_allocate(struct VirtQueue* queue, u16* descriptor);

void
virtqueue_free(struct VirtQueue* queue, u16 descriptor);

error_t
virtqueue_destroy(struct VirtQueue* queue);
