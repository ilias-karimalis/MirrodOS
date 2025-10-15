#pragma once

#include <types/number.h>

struct virtio_driver;

#define VIRTIO_BLK_F_BARRIER 0
#define VIRTIO_BLK_F_SIZE_MAX 1
#define VIRTIO_BLK_F_SEG_MAX 2
#define VIRTIO_BLK_F_GEOMETRY 4
#define VIRTIO_BLK_F_RO 5
#define VIRTIO_BLK_F_BLK_SIZE 6
#define VIRTIO_BLK_F_SCSI 7
#define VIRTIO_BLK_F_FLUSH 9
#define VIRTIO_BLK_F_WCE 9
#define VIRTIO_BLK_F_TOPOLOGY 10
#define VIRTIO_BLK_F_CONFIG_WCE 11
#define VIRTIO_BLK_F_DISCARD 13
#define VIRTIO_BLK_F_WRITE_ZEROES 14

void
block_driver_init(struct virtio_driver* driver, void* base);

// struct virtio_blk_config
// {
//     u64 capacity;
//     u32 size_max;
//     u32 seg_max;
//     struct virtio_blk_geometry
//     {
//         u16 cylinders;
//         u8 heads;
//         u8 sectors;
//     } geometry;
//     u32 blk_size;
//     struct virtio_blk_topology
//     {
//         u8 physical_block_exp;
//         u8 alignment_offset;
//         u16 min_io_size;
//         u32 opt_io_size;
//     } topology;
//     u8 writeback;
//     u8 unused0[3];
//     TODO();
// };