#pragma once

// project
#include "I_Card.h"

// this file
#include "I_Buss.h"

class Buss : public I_Buss
{
private:
  I_Card* memory_;

public:
  // constructors
  Buss(I_Card* memory);

  // mutators
  void mem_write(uint16_t address, uint8_t value);
  void mem_write_block(uint16_t dest_address, uint8_t* source_address, uint16_t count);

    void io_write(uint8_t port, uint8_t value);

  // derived values
  uint8_t mem_read(uint16_t address) const;
  uint8_t io_read(uint8_t port) const;
};
