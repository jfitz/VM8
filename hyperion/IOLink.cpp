// C
#include <cstdlib>
#include <cstddef>
#include <cstring>

// STL
#include <exception>
#include <stdexcept>

// this project

// this file
#include "IOLink.h"


// ========================================
//
// ----------------------------------------
IOLink::IOLink()
  :
  start_address_(0),
  end_address_(1)
{
}

// ========================================
//
// ----------------------------------------
void IOLink::mem_write(uint16_t address, uint8_t value)
{
}

// ========================================
//
// ----------------------------------------
void IOLink::io_write(uint8_t port, uint8_t value)
{
  if (io_responds_to(port))
  {
    // do something
  }
}

// ========================================
//
// ----------------------------------------
uint8_t IOLink::mem_read(uint16_t address) const
{
  return 0;
}

// ========================================
//
// ----------------------------------------
uint8_t IOLink::io_read(uint8_t port) const
{
  if (io_responds_to(port))
  {
    // do something here
    return 0;
  }

  return 0;
}

// ========================================
//
// ----------------------------------------
bool IOLink::mem_responds_to(uint16_t address) const
{
    return false;
}

// ========================================
//
// ----------------------------------------
bool IOLink::io_responds_to(uint8_t port) const
{
  return (port >= start_address_) && (port <= end_address_);
}
