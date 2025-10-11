#pragma once

#include <devices/device_tree/blob.h>
#include <devices/plic.h>
#include <devices/uart.h>
#include <devices/virtio/virtio_mmio.h>

enum device_type
{
        DEVICE_TYPE_VIRTIO_MMIO,
        DEVICE_TYPE_PLIC,
        DEVICE_TYPE_UART,
};

struct driver
{
        enum device_type type;
        union
        {
                struct virtio_driver virtio;
                struct plic_driver plic;
                struct uart_driver uart;
        } d;
};

void
devices_init(struct device_tree* tree, u32 bsp_hartid);

struct plic_driver*
devices_get_plic_driver(u32 hartid);