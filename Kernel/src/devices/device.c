#include <assert.h>
#include <devices/device.h>
#include <devices/device_tree/blob.h>
#include <kvspace.h>
#include <limine/platform_info.h>
#include <memory.h>
#include <riscv.h>

struct driver_node
{
        struct driver_node* next;
        struct driver driver;
};

/// Map of Hart ID to its PLIC driver, if it has initialized one.
struct plic_driver** hart_plic_map = NULL;
size_t map_capacity = 0;
size_t map_alloc_size = 0;
/// List of all device drivers that have been initialized.
struct driver_node* drivers = NULL;
/// Number of initialized drivers.
size_t driver_count = 0;
/// Slab allocator for driver nodes.
struct slab_alloc driver_node_arena = { 0 };

void
devices_recursive_initialize(struct device_tree* tree, struct device_tree_node* node, struct plic_driver* plic)
{
        kprintln(node->name);
        for (size_t i = 0; i < node->compatible_count; i++) {
                struct str_view compat = node->compatible[i];
                if (sv_compare(compat, SV("ns16550a")) == 0) {
                        struct device_tree_property* reg = device_tree_get_property(node, SV("reg"));
                        if (reg == NULL || reg->type != DT_PROPERTY_REG || reg->value.reg.n_pairs != 1 ||
                            node->address_cells != 2 || node->size_cells != 2) {
                                kprintln(SV("Failed to initialize the {V} device"), SVP(node->name));
                                continue;
                        }
                        u64 phys_addr = ((u64*)reg->value.reg.addresses)[0];
                        void* virt_addr = kernel_hhdm_phys_to_virt(phys_addr);
                        struct driver_node* uart_node = slab_allocate(&driver_node_arena);
                        uart_driver_init(&uart_node->driver.d.uart, virt_addr);
                        plic_driver_enable_int(plic, 10, 1, &uart_node->driver);
                        uart_node->driver.type = DEVICE_TYPE_UART;
                        uart_node->next = drivers;
                        drivers = uart_node;
                        driver_count++;
                        kprintln(SV("Initialized a UART device at {X}"), phys_addr);
                }

                else if (sv_compare(compat, SV("virtio,mmio")) == 0) {
                        struct device_tree_property* reg = device_tree_get_property(node, SV("reg"));
                        if (reg == NULL || reg->type != DT_PROPERTY_REG || reg->value.reg.n_pairs != 1 ||
                            node->address_cells != 2 || node->size_cells != 2) {
                                kprintln(SV("The {V} device's device tree node is malformed."), node->name);
                                continue;
                        }
                        u64 phys_addr = ((u64*)reg->value.reg.addresses)[0];
                        u64 phys_size = ((u64*)reg->value.reg.sizes)[0];
                        void* virt_addr = kernel_hhdm_phys_to_virt(phys_addr);
                        struct driver_node* virtio_node = slab_allocate(&driver_node_arena);
                        virtio_mmio_driver_init(&virtio_node->driver.d.virtio, virt_addr, phys_size);
                        virtio_node->driver.type = DEVICE_TYPE_VIRTIO_MMIO;
                        virtio_node->next = drivers;
                        drivers = virtio_node;
                        driver_count++;
                        kprintln(SV("Initialized a VirtIO MMIO device at {X}"), phys_addr);
                }
        }

        for (struct device_tree_node* child = node->children; child != NULL; child = child->sibling) {
                devices_recursive_initialize(tree, child, plic);
        }
}

void
devices_init(struct device_tree* tree, u32 bsp_hartid)
{
        drivers = NULL;
        driver_count = 0;
        slab_autorefill_init(&driver_node_arena, sizeof(struct driver_node));

        struct allocation alloc = kalloc(RISCV_SV39_PAGE_SIZE, RISCV_SV39_PAGE_SIZE);
        map_alloc_size = RISCV_SV39_PAGE_SIZE;
        map_capacity = map_alloc_size / sizeof(struct plic_driver*);
        hart_plic_map = alloc.buffer;
        memzero(hart_plic_map, map_alloc_size);

        /// To initialize the device drivers, we must have a working plic driver for this hart.
        struct device_tree_node* plic_node = dt_node_from_compatible(tree, SV("riscv,plic0"));
        if (plic_node == NULL) {
                PANIC(SV("No PLIC found in device tree."));
        }
        struct device_tree_property* reg = device_tree_get_property(plic_node, SV("reg"));
        if (reg == NULL || reg->type != DT_PROPERTY_REG || reg->value.reg.n_pairs != 1 ||
            plic_node->address_cells != 2 || plic_node->size_cells != 2) {
                PANIC(SV("Problem with `riscv,plic0` device's `reg` property."));
        }
        u64 phys_addr = ((u64*)reg->value.reg.addresses)[0];
        kprintln(SV("Found PLIC at physical address {X}"), phys_addr);
        void* virt_addr = kernel_hhdm_phys_to_virt(phys_addr);
        struct driver_node* plic_driver_node = slab_allocate(&driver_node_arena);
        plic_driver_node->driver.type = DEVICE_TYPE_PLIC;
        plic_driver_init(&plic_driver_node->driver.d.plic, virt_addr, bsp_hartid);
        plic_driver_node->next = drivers;
        drivers = plic_driver_node;
        driver_count++;
        hart_plic_map[bsp_hartid] = &plic_driver_node->driver.d.plic;

        /// We can now safely initialize all other devices by walking the device tree.
        devices_recursive_initialize(tree, tree->root_node, hart_plic_map[bsp_hartid]);
}

struct plic_driver*
devices_get_plic_driver(u32 hartid)
{
        if (hartid >= map_capacity) {
                return NULL;
        }
        return hart_plic_map[hartid];
}