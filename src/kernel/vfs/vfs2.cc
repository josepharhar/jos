#include "vfs2.h"

namespace vfs {

bool IsValidCluster(uint64_t cluster) {
  return cluster >= FAT32_CLUSTER_VALID_START &&
         cluster <= FAT32_CLUSTER_VALID_END;
}

}  // namespace vfs
