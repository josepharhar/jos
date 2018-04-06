#include "vfs2.h"

bool IsValidCluster(uint64_t cluster) {
  return cluster >= FAT32_CLUSTER_VALID_START &&
         cluster <= FAT32_CLUSTER_VALID_END;
}
