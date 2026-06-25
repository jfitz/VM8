#pragma once

// C
#include <cstdint>

class I_Card {
 public:
  // mutators
  virtual void mem_write(uint16_t address, uint8_t value) = 0;
  virtual void io_write(uint8_t port, uint8_t value) = 0;

  // derived values
  virtual uint8_t mem_read(uint16_t address) const = 0;
  virtual uint8_t io_read(uint8_t port) const = 0;

  // tests
  virtual bool mem_responds_to(uint16_t address) const = 0;
  virtual bool io_responds_to(uint8_t port) const = 0;
};
