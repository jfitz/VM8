// C
#include <cstdlib>
#include <cstddef>
#include <cstring>

// STL
#include <exception>
#include <stdexcept>

// this project

// this file
#include "Memory64k.h"


// ========================================
//
// ----------------------------------------
Memory64k::Memory64k()
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
Memory64k::~Memory64k()
{
  free(bytes_);
}

// ========================================
//
// ----------------------------------------
void Memory64k::mem_write(uint16_t address, uint8_t value)
{
  bytes_[address] = value;
}

// ========================================
//
// ----------------------------------------
void Memory64k::io_write(uint8_t port, uint8_t value)
{
  if (io_responds_to(port))
  {
    // do something here
  }
}

// ========================================
//
// ----------------------------------------
uint8_t Memory64k::mem_read(uint16_t address) const
{
  return bytes_[address];
}

// ========================================
//
// ----------------------------------------
uint8_t Memory64k::io_read(uint8_t port) const
{
  if (io_responds_to(port))
  {
    // do something here
    return 0;
  }

  return 0;
}

// ========================================
//
// ----------------------------------------
bool Memory64k::mem_responds_to(uint16_t address) const
{
    return true;
}

// ========================================
//
// ----------------------------------------
bool Memory64k::io_responds_to(uint8_t port) const
{
  return false;
}
