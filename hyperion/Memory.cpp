// C
#include <cstdlib>
#include <cstddef>
#include <cstring>

// STL
#include <exception>
#include <stdexcept>

// this project

// this file
#include "Memory.h"


// ========================================
//
// ----------------------------------------
Memory::Memory()
  :
  start_address_(0),
  end_address_(0xffff)
{
  size_t size = end_address_ - start_address_ + 1;
  bytes_ = (uint8_t*)malloc(size);

  if (bytes_ == NULL)
  {
    throw std::runtime_error("cannot allocate memory");
  }

  memset(bytes_, 0, size);
}

// ========================================
//
// ----------------------------------------
Memory::~Memory()
{
  free(bytes_);
}

// ========================================
//
// ----------------------------------------
void Memory::mem_write(uint16_t address, uint8_t value)
{
  // check address is in range [start_address, size_)

  bytes_[address] = value;
}

// ========================================
//
// ----------------------------------------
void Memory::io_write(uint8_t port, uint8_t value)
{
  // do something here
}

// ========================================
//
// ----------------------------------------
uint8_t Memory::mem_read(uint16_t address) const
{
  // check address is in range [start_address, size_)

  return bytes_[address];
}

// ========================================
//
// ----------------------------------------
uint8_t Memory::io_read(uint8_t port) const
{
  return 0;
}
