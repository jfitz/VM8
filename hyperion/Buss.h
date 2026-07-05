#pragma once

// STL
#include <vector>

// project
#include "I_Card.h"

// this file
#include "I_Buss.h"

class Buss : public I_Buss
{
private:
  I_Card* memory_card_;
  std::vector<I_Card*> io_cards_;

public:
  // constructors
  Buss();

  // mutators
  void set_memory(I_Card* card);

  void mem_write(uint16_t address, uint8_t value);

  void io_write(uint8_t port, uint8_t value);

  // derived values
  uint8_t mem_read(uint16_t address) const;
  uint8_t io_read(uint8_t port) const;

private:
  I_Card* find_io_card(uint8_t port) const;
};
