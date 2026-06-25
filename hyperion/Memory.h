#pragma once

#include "I_Card.h"

class Memory : public I_Card
{
private:
  uint16_t start_address_;
  uint16_t end_address_;
  uint8_t* bytes_;

public:
  // constructors
  Memory();
  ~Memory();

  // mutators
  void mem_write(uint16_t address, uint8_t value);
  void io_write(uint8_t port, uint8_t value);

  // derived values
  uint8_t mem_read(uint16_t address) const;
  uint8_t io_read(uint8_t port) const;
};
