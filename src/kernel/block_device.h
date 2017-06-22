#ifndef BLOCK_DEVICE_H_
#define BLOCK_DEVICE_H_

#include "stdint.h"

enum class BlockDeviceType { MASS_STORAGE, PARTITION };

class BlockDevice {
 public:
  BlockDevice();
  ~BlockDevice();

  // These must be called from a blocking context
  // TODO virtual functions do not work, they cause page faults
  virtual int ReadBlock(uint64_t block_num, void* dest) = 0;
  virtual int WriteBlock(uint64_t block_num, void* src) = 0;

  uint64_t GetBlockSize();
  uint64_t GetTotalLength();
  BlockDeviceType GetBlockDeviceType();
  char* GetName();
  uint8_t GetFsType();

 protected:
  void SetBlockSize(uint64_t block_size);
  void SetTotalLength(uint64_t total_length);
  void SetBlockDeviceType(BlockDeviceType device_type);
  void SetName(char* name);
  void SetFsType(uint8_t fs_type);

 private:
  uint64_t block_size;
  uint64_t total_length;
  BlockDeviceType device_type;
  char* name;
  uint8_t fs_type;
};

#endif  // BLOCK_DEVICE_H_
