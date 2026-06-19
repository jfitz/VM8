#include "Buss.h"

// ========================================
//
// ----------------------------------------
Buss::Buss(I_Memory* memory)
  :
  memory_(memory)
{
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write(uint16_t addr, uint8_t val)
{
  memory_->mem_write(addr, val);
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write_block(uint16_t dest_addr, uint8_t* src_addr, uint16_t count)
{
  unsigned int index = 0;

  while (index <= count)
  {
    memory_->mem_write(dest_addr, src_addr[index]);

    dest_addr += 1;
    index += 1;
  }
}

// ========================================
//
// ----------------------------------------
uint8_t Buss::mem_read(uint16_t addr) const
{
  return memory_->mem_read(addr);
}
