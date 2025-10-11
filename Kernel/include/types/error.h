#pragma once

#include <stdbool.h>
#include <types/number.h>
#include <types/str_view.h>

enum ErrorCode
{
        EC_SUCCESS,
        EC_NOT_IMPLEMENTED,
        EC_NULL_ARGUMENT,

        /// PMM Errors
        EC_PMM_REGION_LIST_FULL,
        EC_PMM_REGION_TOO_SMALL,
        EC_PMM_REGION_ALREADY_MANAGED,
        EC_PMM_BAD_ALIGNMENT,
        EC_PMM_OUT_OF_MEMORY,

        // Riscv Paging Errors
        EC_RISCV_SV39_UNALIGNED_ADDR,
        EC_RISCV_SV39_ALLOC_FAILED,
        EC_RISCV_SV39_MAPPING_EXISTS,
        EC_RISCV_SV39_NO_MAPPING,

        // Virtio Errors
        EC_VIRTIO_INVALID_MAGIC,
        EC_VIRTIO_UNSUPPORTED_VERSION,
        EC_VIRTIO_INVALID_DEVICE,
        EC_VIRTIO_UNSUPPORTED_DEVICE,

        // Device Tree Errors
        EC_DT_BLOB_INVALID_MAGIC,
        EC_DT_BLOB_EMPTY_TREE,
        EC_DT_BLOB_REWRITE_FAILED,
        EC_DT_ADDRESS_CELLS_TOO_LARGE,
        EC_DT_SIZE_CELLS_TOO_LARGE,

        // Driver Errors
        EC_DEVICE_NO_DEVICES,

        // Plic Errors
        EC_PLIC_ALREADY_INITIALIZED,
        EC_PLIC_NO_INTERRUPT,
        EC_PLIC_UNREGISTERED_DRIVER,
        EC_PLIC_UNREGISTERED_INTERRUPT,

        /// Uart Errors
        EC_UART_DRIVER_NO_DATA,

        // This must be the last entry - used for counting
        EC_COUNT
};

typedef u64 error_t;

/// Pushes an error code onto the stack. Erases the bottom-most element if the stack is full.
error_t
error_push(error_t err, enum ErrorCode code);

/// Pops the top error code from the stack.
error_t
error_pop(error_t err);

/// Returns the top error value from the stack without removing it.
enum ErrorCode
error_top(error_t err);

/// Returns true if the top error code is not an error.
bool
error_is_ok(error_t err);

/// Returns false if the top error code is an error.
bool
error_is_err(error_t err);

struct str_view
error_string(error_t err);