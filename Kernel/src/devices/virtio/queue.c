#include <devices/virtio/queue.h>
#include <kvspace.h>
#include <pmm.h>
#include <riscv.h>
#include <types/error.h>

error_t
virtqueue_create(struct VirtQueue* queue, u16 queue_size)
{

        // Check that the queue_size conforms to the specification.
        if (queue_size < VIRTQUEUE_QUEUE_SIZE_MIN || queue_size > VIRTQUEUE_QUEUE_SIZE_MAX ||
            !IS_ALIGNED(queue_size, 2)) {
                return error_push(EC_VIRTQUEUE_SIZE, EC_VIRTQUEUE_CREATE);
        }

        // Compute the size of buffer which must be allocated for the VirtQueue.
        // For now we default to placing the Table and Rings on separate pages, note also that we don't check if the
        // device supports the extra options as we don't support them right now. If we wanted to, this function would
        // have to take those flags as an argument. We are however already allocating enough memory for them in the size
        // calculations.
        size_t descriptor_table_sz = ALIGN_UP(16 * queue_size, RISCV_SV39_PAGE_SIZE);
        size_t available_ring_sz = ALIGN_UP(6 + 2 * queue_size, RISCV_SV39_PAGE_SIZE);
        size_t used_ring_sz = ALIGN_UP(6 + 8 * queue_size, RISCV_SV39_PAGE_SIZE);
        // ^ Note that by putting them on separate pages we guarantee that they're aligned properly.
        size_t total_sz = descriptor_table_sz + available_ring_sz + used_ring_sz;

        paddr_t base = 0;
        error_t err = pmm_alloc(total_sz, &base);
        if (error_is_err(err)) {
                return error_push(err, EC_VIRTQUEUE_CREATE);
        }
        void* va_base = kernel_hhdm_phys_to_virt(base);

        queue->queue_size = queue_size;
        queue->free_descriptors = 0;
        queue->descriptor_table = (struct VirtQueueDescriptor*)va_base;
        queue->avail_ring = (struct VirtQueueAvailable*)(va_base + descriptor_table_sz);
        queue->used_ring = (struct VirtQueueUsed*)(va_base + descriptor_table_sz + available_ring_sz);

        // Loop through the descriptors and set them to point to the next one in the queue so that the free list
        // invariant is maintained. Note that the last descriptor is being set to point to `queue_size + 1`, so that's
        // our indicator for when we've reached the end of the list.
        for (size_t i = 0; i < queue_size; i++) {
                queue->descriptor_table[i].next = i + 1;
        }
        return EC_SUCCESS;
}

error_t
virtqueue_allocate(struct VirtQueue* queue, u16* descriptor)
{
        if (queue->free_descriptors == queue->queue_size + 1) {
                return EC_VIRTQUEUE_FULL;
        }
        *descriptor = queue->free_descriptors;
        queue->free_descriptors = queue->descriptor_table[*descriptor].next;
        // queue->descriptor_table[*descriptor].next = queue->queue_size + 1;
        return EC_NOT_IMPLEMENTED;
}

void
virtqueue_free(struct VirtQueue* queue, u16 descriptor)
{
        queue->descriptor_table[descriptor].next = queue->free_descriptors;
        queue->free_descriptors = descriptor;
}

error_t
virtqueue_destroy(struct VirtQueue* queue)
{
        return EC_NOT_IMPLEMENTED;
}
