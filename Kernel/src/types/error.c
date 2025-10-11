#include <stdbool.h>
#include <types/error.h>

static struct str_view error_strings[] = {
        [EC_SUCCESS] = SV("EC_SUCCESS: No error."),
        [EC_NOT_IMPLEMENTED] = SV("EC_NOT_IMPLEMENTED: Feature not implemented."),
        [EC_NULL_ARGUMENT] = SV("EC_NULL_ARGUMENT: Null argument provided."),

        // PMM Errors
        [EC_PMM_REGION_LIST_FULL] = SV("EC_PMM_REGION_LIST_FULL: Physical memory manager region list is full."),
        [EC_PMM_REGION_TOO_SMALL] = SV("EC_PMM_REGION_TOO_SMALL: Physical memory region is too small."),
        [EC_PMM_REGION_ALREADY_MANAGED] =
          SV("EC_PMM_REGION_ALREADY_MANAGED: Physical memory region is already managed."),
        [EC_PMM_BAD_ALIGNMENT] = SV("EC_PMM_BAD_ALIGNMENT: Bad alignment for physical memory allocation."),
        [EC_PMM_OUT_OF_MEMORY] = SV("EC_PMM_OUT_OF_MEMORY: Physical memory manager is out of memory."),

        // RISC-V Paging Errors
        [EC_RISCV_SV39_UNALIGNED_ADDR] = SV("EC_RISCV_SV39_UNALIGNED_ADDR: Unaligned address for SV39 paging."),
        [EC_RISCV_SV39_ALLOC_FAILED] = SV("EC_RISCV_SV39_ALLOC_FAILED: SV39 page allocation failed."),
        [EC_RISCV_SV39_MAPPING_EXISTS] = SV("EC_RISCV_SV39_MAPPING_EXISTS: SV39 mapping already exists."),
        [EC_RISCV_SV39_NO_MAPPING] = SV("EC_RISCV_SV39_NO_MAPPING: No SV39 mapping found."),

        // VirtIO Errors
        [EC_VIRTIO_INVALID_MAGIC] = SV("EC_VIRTIO_INVALID_MAGIC: Invalid VirtIO magic number."),
        [EC_VIRTIO_UNSUPPORTED_VERSION] = SV("EC_VIRTIO_UNSUPPORTED_VERSION: Unsupported VirtIO version."),
        [EC_VIRTIO_INVALID_DEVICE] = SV("EC_VIRTIO_INVALID_DEVICE: Invalid VirtIO device."),
        [EC_VIRTIO_UNSUPPORTED_DEVICE] = SV("EC_VIRTIO_UNSUPPORTED_DEVICE: Unsupported VirtIO device."),

        // Device Tree Errors
        [EC_DT_BLOB_INVALID_MAGIC] = SV("EC_DT_BLOB_INVALID_MAGIC: Invalid device tree blob magic."),
        [EC_DT_BLOB_EMPTY_TREE] = SV("EC_DT_BLOB_EMPTY_TREE: Device tree blob is empty."),
        [EC_DT_BLOB_REWRITE_FAILED] = SV("EC_DT_BLOB_REWRITE_FAILED: Device tree blob rewrite failed."),
        [EC_DT_ADDRESS_CELLS_TOO_LARGE] = SV("EC_DT_ADDRESS_CELLS_TOO_LARGE: Device tree address cells too large."),
        [EC_DT_SIZE_CELLS_TOO_LARGE] = SV("EC_DT_SIZE_CELLS_TOO_LARGE: Device tree size cells too large."),

        // Driver Errors
        [EC_DEVICE_NO_DEVICES] = SV("EC_DEVICE_NO_DEVICES: No devices found."),

        // PLIC Errors
        [EC_PLIC_ALREADY_INITIALIZED] = SV("EC_PLIC_ALREADY_INITIALIZED: PLIC already initialized on this hart."),
        [EC_PLIC_NO_INTERRUPT] = SV("EC_PLIC_NO_INTERRUPT: No interrupt claimed by PLIC on this hart."),
        [EC_PLIC_UNREGISTERED_DRIVER] =
          SV("EC_PLIC_UNREGISTERED_DRIVER: Claimed interrupt driver registered, but not implemented."),
        [EC_PLIC_UNREGISTERED_INTERRUPT] =
          SV("EC_PLIC_UNREGISTERED_INTERRUPT: Claimed interrupt has no registered handler."),

        // Uart Errors
        [EC_UART_DRIVER_NO_DATA] = SV("EC_UART_DRIVER_NO_DATA: No data available to read from UART."),
};

// Static assertion to ensure error_strings array size matches ErrorCode enum count
_Static_assert(sizeof(error_strings) / sizeof(error_strings[0]) == EC_COUNT,
               "error_strings array size must match ErrorCode enum count");

error_t
error_push(error_t err, enum ErrorCode code)
{
        err <<= sizeof(enum ErrorCode);
        err |= (u64)code;
        return err;
}

error_t
error_pop(error_t err)
{
        return err >> sizeof(enum ErrorCode);
}

enum ErrorCode
error_top(error_t err)
{
        return (enum ErrorCode)(err & 0xFF);
}

bool
error_is_ok(error_t err)
{
        return error_top(err) == EC_SUCCESS;
}

bool
error_is_err(error_t err)
{
        return !error_is_ok(err);
}

struct str_view
error_string(error_t err)
{
        return error_strings[error_top(err)];
}