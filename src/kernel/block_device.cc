#include "block_device.h"

BlockDevice::BlockDevice() {}

BlockDevice::~BlockDevice() {}

uint64_t BlockDevice::GetBlockSize() {
  return block_size;
}

uint64_t BlockDevice::GetTotalLength() {
  return total_length;
}

BlockDeviceType BlockDevice::GetBlockDeviceType() {
  return device_type;
}

char* BlockDevice::GetName() {
  return name;
}

uint8_t BlockDevice::GetFsType() {
  return fs_type;
}

void BlockDevice::SetBlockSize(uint64_t block_size) {
  this->block_size = block_size;
}

void BlockDevice::SetTotalLength(uint64_t total_length) {
  this->total_length = total_length;
}

void BlockDevice::SetBlockDeviceType(BlockDeviceType device_type) {
  this->device_type = device_type;
}

void BlockDevice::SetName(char* name) {
  this->name = name;
}

void BlockDevice::SetFsType(uint8_t fs_type) {
  this->fs_type = fs_type;
}
