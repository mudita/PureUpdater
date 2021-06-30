#pragma once

struct blk_partition;

/** Scan Hardware for a partitions
 * @param disk Disk handler
 * @param part Partition for resize
 * @return number of partitions or error
 */
int blk_priv_scan_partitions(int disk, struct blk_partition **part);