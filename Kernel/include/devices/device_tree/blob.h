#pragma once

#include <types/bump_alloc.h>
#include <types/error.h>
#include <types/number.h>
#include <types/slab.h>
#include <types/str_view.h>
#include <types/view.h>

struct device_tree
{
        struct slab_alloc reserved_arena;
        struct slab_alloc node_arena;
        struct slab_alloc property_arena;
        struct slab_alloc phandlemap_arena;
        struct bump_alloc bump;

        struct device_tree_node* root_node;
        struct device_tree_reserved* reserved_memory;
        struct device_tree_phandle_map* phandle_map;
};

//	   reserve
struct device_tree_reserved
{
        paddr_t region_base;
        size_t region_size;
        struct device_tree_reserved* next_region;
};

enum device_tree_property_type
{
        DT_PROPERTY_RAW,
        DT_PROPERTY_PHANDLE,
        DT_PROPERTY_COMPATIBLE,
        DT_PROPERTY_ADDRESS_CELLS,
        DT_PROPERTY_SIZE_CELLS,
        DT_PROPERTY_REG,
};

struct device_tree_property
{
        struct str_view name;
        enum device_tree_property_type type;
        union property_value
        {
                struct view raw;
                u32 phandle;
                u32 address_cells;
                u32 size_cells;
                struct reg_value
                {
                        size_t n_pairs;
                        void* addresses;
                        void* sizes;
                } reg;
        } value;
        struct device_tree_property* next_prop;
};

struct device_tree_phandle_map
{
        u32 phandle;
        struct device_tree_node* node;
        struct device_tree_phandle_map* next;
};

struct device_tree_node
{
        /// Name of the node.
        struct str_view name;
        /// The number of <u32> cells used to encode the address field in this node's
        /// reg property.
        u32 address_cells;
        /// The number of <u32> cells used to encode the size field in this node's reg
        /// property.
        u32 size_cells;
        /// Compatible device list.
        struct str_view* compatible;
        size_t compatible_count;
        // / phandle for this node, if
        /// List of properties attached to this node.
        struct device_tree_property* properties;
        /// Parent node of this tree node (NULL for root).
        struct device_tree_node* parent;
        /// List of child nodes.
        struct device_tree_node* children;
        /// List of sibling nodes.
        struct device_tree_node* sibling;
};

error_t
device_tree_parse_blob(const u8* blob, struct device_tree* tree);

struct device_tree_property*
device_tree_get_property(struct device_tree_node* node, struct str_view name);

struct device_tree_node*
device_tree_node_from_phandle(struct device_tree* tree, u32 phandle);

struct device_tree_node*
dt_node_from_compatible(struct device_tree* tree, struct str_view compatible);