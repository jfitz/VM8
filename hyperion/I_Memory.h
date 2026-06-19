#pragma once

// C
#include <cstdint>

class I_Memory {
 public:
  // mutators
  virtual void mem_write(uint16_t, uint8_t) = 0;

  // derived values
  virtual uint8_t mem_read(uint16_t) const = 0;
};
