#include "Buss.h"

// ========================================
//
// ----------------------------------------
Buss::Buss()
{
}

// ========================================
//
// ----------------------------------------
void Buss::add(I_Card* memory)
{
  memory_ = memory;
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write(uint16_t address, uint8_t value)
{
  memory_->mem_write(address, value);
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write_block(uint16_t dest_address, uint8_t* source_address, uint16_t count)
{
  unsigned int index = 0;

  while (index <= count)
  {
    memory_->mem_write(dest_address, source_address[index]);

    dest_address += 1;
    index += 1;
  }
}

// ========================================
//
// ----------------------------------------
void Buss::io_write(uint8_t port, uint8_t value)
{
  memory_->io_write(port, value);
}

// ========================================
//
// ----------------------------------------
uint8_t Buss::mem_read(uint16_t address) const
{
  return memory_->mem_read(address);
}

// ========================================
//
// ----------------------------------------
uint8_t Buss::io_read(uint8_t port) const
{
  return memory_->io_read(port);
}
