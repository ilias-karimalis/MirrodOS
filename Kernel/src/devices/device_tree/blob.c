#include "types/bump_alloc.h"
#include "types/number.h"
#include "types/str_view.h"
#include <assert.h>
#include <devices/device_tree/blob.h>
#include <fmt/print.h>
#include <kvspace.h>
#include <pmm.h>
#include <riscv.h>
#include <stddef.h>
#include <stdint.h>
#include <types/error.h>
#include <types/slab.h>

#define ENDIANNESS_FLIP_U16(x) (((x) >> 8) | ((x) << 8))
#define ENDIANNESS_FLIP_U32(x)                                                                                         \
        ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) |                          \
         (((x) << 24) & 0xFF000000))
#define ENDIANNESS_FLIP_U64(x)                                                                                         \
        ((((x) >> 56) & 0x00000000000000FFULL) | (((x) >> 40) & 0x000000000000FF00ULL) |                               \
         (((x) >> 24) & 0x0000000000FF0000ULL) | (((x) >> 8) & 0x00000000FF000000ULL) |                                \
         (((x) << 8) & 0x000000FF00000000ULL) | (((x) << 24) & 0x0000FF0000000000ULL) |                                \
         (((x) << 40) & 0x00FF000000000000ULL) | (((x) << 56) & 0xFF00000000000000ULL))
static inline u128
endianness_flip_u128(u128 x)
{
        union
        {
                u128 val;
                u8 bytes[16];
        } src, dst;

        src.val = x;
        for (int i = 0; i < 16; i++) {
                dst.bytes[i] = src.bytes[15 - i];
        }
        return dst.val;
}

#define READ_BIG_ENDIAN_U32(x) (ENDIANNESS_FLIP_U32(*(u32*)(x)))
#define READ_BIG_ENDIAN_U64(x) (ENDIANNESS_FLIP_U64(*(u64*)(x)))
#define READ_BIG_ENDIAN_U128(x) (endianness_flip_u128(*(u128*)(x)))

#define DEVICE_TREE_BLOB_MAGIC 0x0D00DFEED
#define DEFAULT_ADDRESS_CELLS 2
#define DEFAULT_SIZE_CELLS 1

struct blob_header
{
        u32 magic;
        u32 total_size;
        u32 offset_structs;
        u32 offset_strings;
        u32 offset_rsvmap;
        u32 version;
        u32 compat_version;
        u32 boot_cpuid_phys;
        u32 size_strings;
        u32 size_struct;
};

enum structure_token
{
        STRUCTURE_TOKEN_NODE_START = 0x1,
        STRUCTURE_TOKEN_NODE_END = 0x2,
        STRUCTURE_TOKEN_PROPERTY = 0x3,
        STRUCTURE_TOKEN_NOP = 0x4,
        STRUCTURE_TOKEN_END = 0x9,
};

size_t
device_tree_parse_node(struct device_tree* tree, struct device_tree_node** current, const u8* structs, size_t offset)
{
        struct device_tree_node* new_node = (struct device_tree_node*)slab_allocate(&tree->node_arena);
        new_node->name = sv_from_null_term((const char*)(structs + offset));
        new_node->parent = *current;
        new_node->sibling = NULL;
        new_node->children = NULL;
        new_node->properties = NULL;
        new_node->compatible = NULL;
        new_node->compatible_count = 0;

        if (*current != NULL) {
                new_node->sibling = (*current)->children;
                (*current)->children = new_node;
                *current = new_node;
        }

        *current = new_node;
        return offset + ALIGN_UP(new_node->name.size + 1, sizeof(u32));
}

size_t
device_tree_first_pass_property(struct device_tree* tree,
                                struct device_tree_node* current,
                                const u8* structs,
                                const char* strings,
                                size_t offset)
{
        u32 property_length = READ_BIG_ENDIAN_U32(structs + offset);
        offset += sizeof(u32);
        u32 name_offset = READ_BIG_ENDIAN_U32(structs + offset);
        offset += sizeof(u32);

        struct device_tree_property* new_prop = (struct device_tree_property*)slab_allocate(&tree->property_arena);
        new_prop->name = sv_from_null_term((const char*)strings + name_offset);
        new_prop->type = DT_PROPERTY_RAW;
        new_prop->next_prop = current->properties;
        new_prop->value.raw = (struct view){ structs + offset, property_length };
        current->properties = new_prop;

        return offset + ALIGN_UP(property_length, sizeof(u32));
}

void
rewrite_reg_property(struct device_tree* tree, struct device_tree_node* node, struct device_tree_property* prop)
{
        struct view raw = prop->value.raw;
        size_t address_size = sizeof(u32) * node->address_cells;
        size_t size_size = sizeof(u32) * node->size_cells;
        size_t n_pairs = raw.size / (address_size + size_size);
        ASSERT(n_pairs > 0);
        ASSERT(raw.size % (address_size + size_size) == 0);
        ASSERT(node->address_cells <= 3);
        ASSERT(node->size_cells <= 2);

        void* addresses = bump_allocate_aligned(&tree->bump, address_size * n_pairs, address_size);
        void* sizes = bump_allocate_aligned(&tree->bump, size_size * n_pairs, size_size);
        ASSERT((address_size == 0) ? addresses == NULL : addresses != NULL);
        ASSERT((size_size == 0) ? sizes == NULL : sizes != NULL);

        for (size_t i = 0, j = 0; i < n_pairs; i++) {
                switch (node->address_cells) {
                        case 0:
                                break;
                        case 1:
                                ((u32*)addresses)[i] = READ_BIG_ENDIAN_U32(raw.data + j);
                                break;
                        case 2:
                                ((u64*)addresses)[i] = READ_BIG_ENDIAN_U64(raw.data + j);
                                break;
                        case 3:
                                ((u128*)addresses)[i] = READ_BIG_ENDIAN_U128(raw.data + j);
                                break;
                        default:
                                __builtin_unreachable();
                }
                j += address_size;

                switch (node->size_cells) {
                        case 0:
                                break;
                        case 1:
                                ((u32*)sizes)[i] = READ_BIG_ENDIAN_U32(raw.data + j);
                                break;
                        case 2:
                                ((u64*)sizes)[i] = READ_BIG_ENDIAN_U64(raw.data + j);
                                break;
                        default:
                                __builtin_unreachable();
                }
                j += size_size;
        }

        prop->type = DT_PROPERTY_REG;
        prop->value.reg.n_pairs = n_pairs;
        prop->value.reg.addresses = addresses;
        prop->value.reg.sizes = sizes;
}

error_t
device_tree_second_pass_properties(struct device_tree* tree, struct device_tree_node* current)
{
        error_t err = EC_SUCCESS;
        u32 next_address_cells = current->address_cells;
        u32 next_size_cells = current->size_cells;
        for (struct device_tree_property* prop = current->properties; prop != NULL; prop = prop->next_prop) {
                if (sv_compare(SV("compatible"), prop->name) == 0) {
                        struct view raw = prop->value.raw;
                        size_t num_strings = 0;
                        for (size_t i = 0; i < raw.size; i++) {
                                if (raw.data[i] == '\0') {
                                        num_strings++;
                                }
                        }
                        current->compatible_count = num_strings;
                        size_t array_len = sizeof(struct str_view) * num_strings;
                        current->compatible = bump_allocate_aligned(&tree->bump, array_len, sizeof(struct str_view));
                        ASSERT(current->compatible != NULL);

                        for (size_t i = 0, j = 0; i < num_strings && j < raw.size; i++) {
                                current->compatible[i] = sv_from_null_term(&raw.data[j]);
                                j += current->compatible[i].size + 1;
                        }
                        prop->type = DT_PROPERTY_COMPATIBLE;
                } else if (sv_compare(SV("#address-cells"), prop->name) == 0) {
                        prop->value.address_cells = READ_BIG_ENDIAN_U32(prop->value.raw.data);
                        if (prop->value.address_cells > 3) {
                                return EC_DT_ADDRESS_CELLS_TOO_LARGE;
                        }
                        next_address_cells = prop->value.address_cells;
                        prop->type = DT_PROPERTY_ADDRESS_CELLS;
                } else if (sv_compare(SV("#size-cells"), prop->name) == 0) {
                        prop->value.size_cells = READ_BIG_ENDIAN_U32(prop->value.raw.data);
                        if (prop->value.size_cells > 2) {
                                return EC_DT_SIZE_CELLS_TOO_LARGE;
                        }
                        next_size_cells = prop->value.size_cells;
                        prop->type = DT_PROPERTY_SIZE_CELLS;
                } else if (sv_compare(SV("phandle"), prop->name) == 0) {
                        prop->value.phandle = READ_BIG_ENDIAN_U32(prop->value.raw.data);
                        prop->type = DT_PROPERTY_PHANDLE;
                        struct device_tree_phandle_map* map_entry =
                          (struct device_tree_phandle_map*)slab_allocate(&tree->phandlemap_arena);
                        map_entry->phandle = prop->value.phandle;
                        map_entry->node = current;
                        map_entry->next = tree->phandle_map;
                        tree->phandle_map = map_entry;
                } else if (sv_compare(SV("reg"), prop->name) == 0) {
                } else {
                        kprintln(SV("Unhandled Device Tree Property: {V}"), SVP(prop->name));
                }
        }

        // Rewrite pass for properties that depend on #address-cells and #size-cells
        for (struct device_tree_property* prop = current->properties; prop != NULL; prop = prop->next_prop) {
                if (sv_compare(SV("reg"), prop->name) == 0) {
                        rewrite_reg_property(tree, current, prop);
                }
        }

        // Pass one more time through the properties to worn on `reg`, `ranges` and `bus-ranges`.

        for (struct device_tree_node* child = current->children; child != NULL; child = child->sibling) {
                child->address_cells = next_address_cells;
                child->size_cells = next_size_cells;
                err = device_tree_second_pass_properties(tree, child);
                if (error_is_err(err)) {
                        return err;
                }
        }
        return err;
}

error_t
device_tree_parse_blob(const u8* blob, struct device_tree* tree)
{
        struct blob_header* hdr = (struct blob_header*)blob;
        if (DEVICE_TREE_BLOB_MAGIC != ENDIANNESS_FLIP_U32(hdr->magic)) {
                return EC_DT_BLOB_INVALID_MAGIC;
        }

        slab_autorefill_init(&tree->node_arena, sizeof(struct device_tree_node));
        slab_autorefill_init(&tree->property_arena, sizeof(struct device_tree_property));
        slab_autorefill_init(&tree->reserved_arena, sizeof(struct device_tree_reserved));
        slab_autorefill_init(&tree->phandlemap_arena, sizeof(struct device_tree_phandle_map));
        bump_initialize(&tree->bump);
        struct allocation bump_mem = kalloc(2 * RISCV_SV39_PAGE_SIZE, RISCV_SV39_PAGE_SIZE);
        bump_grow(&tree->bump, bump_mem.buffer, bump_mem.size);

        u64* buffer_rsvmap = (u64*)(blob + ENDIANNESS_FLIP_U32(hdr->offset_rsvmap));
        tree->reserved_memory = NULL;
        while (buffer_rsvmap[0] != 0 && buffer_rsvmap[1] != 0) {
                struct device_tree_reserved* rr = (struct device_tree_reserved*)slab_allocate(&tree->reserved_arena);
                rr->region_base = ENDIANNESS_FLIP_U64(buffer_rsvmap[0]);
                rr->region_size = ENDIANNESS_FLIP_U64(buffer_rsvmap[1]);
                rr->next_region = tree->reserved_memory;
                tree->reserved_memory = rr;
        }

        const u8* structs = blob + ENDIANNESS_FLIP_U32(hdr->offset_structs);
        const char* strings = (const char*)blob + ENDIANNESS_FLIP_U32(hdr->offset_strings);

        size_t offset = 0;
        size_t depth = 0;
        struct device_tree_node* current = NULL;
        bool PARSING_STRUCTS = true;
        tree->root_node = NULL;
        while (PARSING_STRUCTS) {
                ASSERT(offset % 4 == 0, SV("Accesses must be 4 byte aligned."));
                u32 token = READ_BIG_ENDIAN_U32(structs + offset);
                offset += sizeof(u32);

                switch (token) {
                        case STRUCTURE_TOKEN_NODE_START:
                                offset = device_tree_parse_node(tree, &current, structs, offset);
                                if (depth == 0 && current->name.size == 0) {
                                        tree->root_node = current;
                                        current->name = SV("/");
                                }
                                depth++;
                                break;
                        case STRUCTURE_TOKEN_NODE_END:
                                current = current->parent;
                                depth--;
                                break;
                        case STRUCTURE_TOKEN_PROPERTY:
                                offset = device_tree_first_pass_property(tree, current, structs, strings, offset);
                                break;
                        case STRUCTURE_TOKEN_NOP:
                                break;
                        case STRUCTURE_TOKEN_END:
                                if (current != NULL) {
                                        PANIC(SV("STRUCTURE_TOKEN_END token found while parsing device tree blob, "
                                                 "while not in the root node. Current depth: {X}"),
                                              depth);
                                }
                                PARSING_STRUCTS = false;
                                break;
                        default:
                                PANIC(SV("Unknown structure type ({X}) encountered while parsing device tree blob."),
                                      token);
                                break;
                }
        }

        if (tree->root_node == NULL) {
                return EC_DT_BLOB_EMPTY_TREE;
        }

        tree->root_node->address_cells = DEFAULT_ADDRESS_CELLS;
        tree->root_node->size_cells = DEFAULT_SIZE_CELLS;
        error_t err = device_tree_second_pass_properties(tree, tree->root_node);
        if (error_is_err(err)) {
                return error_push(err, EC_DT_BLOB_REWRITE_FAILED);
        }
        return EC_SUCCESS;
}

struct device_tree_property*
device_tree_get_property(struct device_tree_node* node, struct str_view name)
{
        for (struct device_tree_property* prop = node->properties; prop != NULL; prop = prop->next_prop) {
                if (sv_compare(prop->name, name) == 0) {
                        return prop;
                }
        }
        return NULL;
}

struct device_tree_node*
dt_node_from_compatible_recursive(struct device_tree_node* node, struct str_view compatible)
{
        if (node == NULL) {
                return NULL;
        }

        for (size_t i = 0; i < node->compatible_count; i++) {
                if (sv_compare(node->compatible[i], compatible) == 0) {
                        return node;
                }
        }

        struct device_tree_node* found = dt_node_from_compatible_recursive(node->children, compatible);
        if (found != NULL) {
                return found;
        }

        return dt_node_from_compatible_recursive(node->sibling, compatible);
}

struct device_tree_node*
dt_node_from_compatible(struct device_tree* tree, struct str_view compatible)
{
        if (tree == NULL || tree->root_node == NULL) {
                return NULL;
        }

        return dt_node_from_compatible_recursive(tree->root_node, compatible);
}