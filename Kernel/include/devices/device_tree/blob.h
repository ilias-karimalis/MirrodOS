#pragma once

#include <types/error.h>
#include <types/generic_arena.h>
#include <types/number.h>
#include <types/str_view.h>
#include <types/view.h>

struct device_tree
{

        struct generic_arena reserved_region_arena;
        struct reserved_region* reserved_memory;

        struct generic_arena node_arena;
        struct device_tree_node* root_node;

        struct generic_arena property_arena;
};

//	   reserve
struct reserved_region
{
        paddr_t region_base;
        size_t region_size;
        struct reserved_region* next_region;
};

enum device_tree_property_type {
    DT_PROPERTY_RAW,
};

struct device_tree_property
{
        struct str_view name;
        enum device_tree_property_type type;
        union {
            struct view raw;
            u32 phandle;
        } value;
        struct device_tree_property* next_prop;
};

struct device_tree_node
{
        /// Name of the node.
        struct str_view name;
        /// The number of <u32> cells used to encode the address field in this node's reg property.
        u32 address_cells;
        /// The number of <u32> cells used to encode the size field in this node's reg property.
        u32 size_cells;
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
device_tree_parse_blob(const u8* blob, size_t size, struct device_tree* tree);

