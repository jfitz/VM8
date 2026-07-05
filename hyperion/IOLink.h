#pragma once

#include "I_Card.h"

class IOLink : public I_Card
{
private:
  uint8_t start_address_;
  uint8_t end_address_;

 public:
  // constructors
  IOLink();

  // mutators
  void mem_write(uint16_t address, uint8_t value);
  void io_write(uint8_t port, uint8_t value);

  // derived values
  uint8_t mem_read(uint16_t address) const;
  uint8_t io_read(uint8_t port) const;

  // tests
  bool mem_responds_to(uint16_t address) const;
  bool io_responds_to(uint8_t port) const;
};
