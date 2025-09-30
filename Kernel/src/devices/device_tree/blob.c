#include "fmt/print.h"
#include <assert.h>
#include <devices/device_tree/blob.h>
#include <kvspace.h>
#include <pmm.h>
#include <riscv.h>
#include <stddef.h>
#include <stdint.h>
#include <types/error.h>
#include <types/generic_arena.h>

#define ENDIANNESS_FLIP_U16(x) (((x) >> 8) | ((x) << 8))
#define ENDIANNESS_FLIP_U32(x)                                                                                         \
        ((((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) |                          \
         (((x) << 24) & 0xFF000000))
#define ENDIANNESS_FLIP_U64(x)                                                                                         \
        ((((x) >> 56) & 0x00000000000000FFULL) | (((x) >> 40) & 0x000000000000FF00ULL) |                               \
         (((x) >> 24) & 0x0000000000FF0000ULL) | (((x) >> 8) & 0x00000000FF000000ULL) |                                \
         (((x) << 8) & 0x000000FF00000000ULL) | (((x) << 24) & 0x0000FF0000000000ULL) |                                \
         (((x) << 40) & 0x00FF000000000000ULL) | (((x) << 56) & 0xFF00000000000000ULL))

#define READ_BIG_ENDIAN_U32(x) (ENDIANNESS_FLIP_U32(*(u32*)(x)))

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
        if (tree->node_arena.free_blocks <= 0) {
                paddr_t pa = 0;
                error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &pa);
                if (error_is_err(err)) {
                        PANIC(SV("Failed to allocate memory for device tree node arena."));
                }
                void* buffer = kernel_hhdm_phys_to_virt(pa);
                generic_arena_grow(&tree->node_arena, buffer, RISCV_SV39_PAGE_SIZE);
        }
        struct device_tree_node* new_node = (struct device_tree_node*)generic_arena_alloc(&tree->node_arena);
        kprintln(SV("{X}"), new_node);
        new_node->name = sv_from_null_term((const char*)(structs + offset));

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

        if (tree->property_arena.free_blocks <= 0) {
                paddr_t pa = 0;
                error_t err = pmm_alloc_aligned(RISCV_SV39_PAGE_SIZE, RISCV_SV39_PAGE_SIZE, &pa);
                kprintln(SV("PA: {X}"), pa);
                kprintln(SV("err: {X}"), err);
                if (error_is_err(err)) {
                        PANIC(SV("Failed to allocate memory for device tree property arena."));
                }
                kprintln(SV("PA: {X}"), pa);
                void* buffer = kernel_hhdm_phys_to_virt(pa);
                generic_arena_grow(&tree->property_arena, buffer, RISCV_SV39_PAGE_SIZE);
        }
        struct device_tree_property* new_prop =
          (struct device_tree_property*)generic_arena_alloc(&tree->property_arena);
        new_prop->name = sv_from_null_term((const char*)strings + name_offset);
        new_prop->type = DT_PROPERTY_RAW;
        new_prop->next_prop = current->properties;
        new_prop->value.raw = (struct view){ structs + offset, property_length };

        return offset + ALIGN_UP(property_length, sizeof(u32));
}

error_t
device_tree_second_pass_properties(struct device_tree_node* current)
{
        error_t err = EC_SUCCESS;

        for (struct device_tree_property* prop = current->properties; prop != NULL; prop = prop->next_prop) {
                kprintln(SV("Unhandled Device Tree Property: {V}"), SVP(prop->name));
        }

        // Pass one more time through the properties to worn on `reg`, `ranges` and `bus-ranges`.

        for (struct device_tree_node* child = current->children; child != NULL; child = child->sibling) {
                err = device_tree_second_pass_properties(child);
                if (error_is_err(err)) {
                        return err;
                }
        }
        return err;
}

error_t
device_tree_parse_blob(const u8* blob, size_t size, struct device_tree* tree)
{
        struct blob_header* hdr = (struct blob_header*)blob;
        if (DEVICE_TREE_BLOB_MAGIC != ENDIANNESS_FLIP_U32(hdr->magic)) {
                return EC_DT_BLOB_INVALID_MAGIC;
        }

        u64* buffer_rsvmap = (u64*)(blob + ENDIANNESS_FLIP_U32(hdr->offset_rsvmap));
        tree->reserved_memory = NULL;
        while (buffer_rsvmap[0] != 0 && buffer_rsvmap[1] != 0) {
                if (tree->reserved_region_arena.free_blocks <= 0) {
                        paddr_t pa = 0;
                        error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &pa);
                        if (error_is_err(err)) {
                                return err;
                        }
                        void* buffer = kernel_hhdm_phys_to_virt(pa);
                        generic_arena_grow(&tree->reserved_region_arena, buffer, RISCV_SV39_PAGE_SIZE);
                }
                struct reserved_region* rr = (struct reserved_region*)generic_arena_alloc(&tree->reserved_region_arena);
                rr->region_base = ENDIANNESS_FLIP_U64(buffer_rsvmap[0]);
                rr->region_size = ENDIANNESS_FLIP_U64(buffer_rsvmap[1]);
                rr->next_region = tree->reserved_memory;
                tree->reserved_memory = rr;
        }
        kprintln(SV("Parsed reserved_memory."));

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
                                kprintln(SV("NODE_START"));
                                offset = device_tree_parse_node(tree, &current, structs, offset);
                                if (depth == 0 && current->name.size == 0) {
                                        tree->root_node = current;
                                        current->name = SV("/");
                                }
                                depth++;
                                break;
                        case STRUCTURE_TOKEN_NODE_END:
                                kprintln(SV("NODE_END"));
                                current = current->parent;
                                depth--;
                                break;
                        case STRUCTURE_TOKEN_PROPERTY:
                                kprintln(SV("PROP"));

                                offset = device_tree_first_pass_property(tree, current, structs, strings, offset);
                                break;
                        case STRUCTURE_TOKEN_NOP:
                                kprintln(SV("NOP"));

                                break;
                        case STRUCTURE_TOKEN_END:
                                kprintln(SV("END"));
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

        kprintln(SV("Finished first pass on structs"));
        if (tree->root_node == NULL) {
                return EC_DT_BLOB_EMPTY_TREE;
        }

        tree->root_node->address_cells = DEFAULT_ADDRESS_CELLS;
        tree->root_node->size_cells = DEFAULT_SIZE_CELLS;
        error_t err = device_tree_second_pass_properties(tree->root_node);
        if (error_is_err(err)) {
                return error_push(err, EC_DT_BLOB_REWRITE_FAILED);
        }
        return EC_SUCCESS;
}
