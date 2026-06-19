#pragma once

// C
#include <cstdint>

class I_Buss {
 public:
  uint8_t mem_read(uint16_t);
  void mem_write(uint16_t, uint8_t);
};
