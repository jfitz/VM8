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

#define MEMORY_SIZE 0x10000

// ========================================
//
// ----------------------------------------
Memory::Memory()
{
  bytes_ = (uint8_t*)malloc(MEMORY_SIZE);

  if (bytes_ == NULL)
  {
    throw std::runtime_error("cannot allocate memory");
  }
  
  memset(bytes_, 0, MEMORY_SIZE);
}

// ========================================
//
// ----------------------------------------
void Memory::mem_write(uint16_t addr, uint8_t val)
{
  bytes_[addr] = val;
}

// ========================================
//
// ----------------------------------------
uint8_t Memory::mem_read(uint16_t addr) const
{
  return bytes_[addr];
}
