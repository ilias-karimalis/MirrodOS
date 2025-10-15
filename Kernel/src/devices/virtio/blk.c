#include <assert.h>
#include <devices/virtio/blk.h>
#include <devices/virtio/mmio.h>
#include <fmt/print.h>

void
block_driver_init(struct virtio_driver* driver, void* base)
{
        driver->is_legacy = true;
        driver->regs.legacy = (struct virtio_mmio_legacy_registers*)base;

        driver->regs.legacy->status = VIRTIO_DEVICE_STATUS_RESET;
        driver->regs.legacy->status = VIRTIO_DEVICE_STATUS_ACKNOWLEDGE;
        driver->regs.legacy->status |= VIRTIO_DEVICE_STATUS_DRIVER_OK;

        // Read device feature bits, write subset of feature bits understood by the OS and driver.
        u32 device_features = driver->regs.legacy->device_features;
        kprintln(SV("Device Features: {B}"), device_features);
        // TODO: We should in the future check all the features here and actually check what's supported and not,
        // for now we just disable the read-only feature to allow read-write access, and ignore everything else.
        u32 driver_features = device_features & !(1 << VIRTIO_BLK_F_RO);
        driver->regs.legacy->driver_features = driver_features;
        driver->regs.legacy->status |= VIRTIO_DEVICE_STATUS_FEATURES_OK;

        u32 status = driver->regs.legacy->status;
        if ((status & VIRTIO_DEVICE_STATUS_FEATURES_OK) != 0) {
                kprintln(SV("Device did not accept our features! Status: {X}"), status);
                driver->regs.legacy->status = VIRTIO_DEVICE_STATUS_FAILED;
                return;
        }

        (void)driver_features;

        PANIC();
}
