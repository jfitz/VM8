#pragma once

#include "I_Memory.h"

class Memory : public I_Memory
{
private:
  uint8_t* bytes_;
public:
  // constructors
  Memory();

  // mutators
  void mem_write(uint16_t, uint8_t);

  // derived values
  uint8_t mem_read(uint16_t) const;
};
