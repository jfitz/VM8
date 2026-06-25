#include "Buss.h"

// ========================================
//
// ----------------------------------------
Buss::Buss()
{
}

// ========================================
//
// ----------------------------------------
void Buss::add(I_Card* memory)
{
  cards_.push_back(memory);
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write(uint16_t address, uint8_t value)
{
  I_Card* active_card = find_memory_card(address);
  
  active_card->mem_write(address, value);
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write_block(uint16_t dest_address, uint8_t* source_address, uint16_t count)
{
  I_Card* active_card = find_memory_card(dest_address);
  
  unsigned int index = 0;

  while (index <= count)
  {
    active_card->mem_write(dest_address, source_address[index]);

    dest_address += 1;
    index += 1;
  }
}

// ========================================
//
// ----------------------------------------
void Buss::io_write(uint8_t port, uint8_t value)
{
  I_Card* active_card = find_io_card(port);
  
  active_card->io_write(port, value);
}

// ========================================
//
// ----------------------------------------
uint8_t Buss::mem_read(uint16_t address) const
{
  I_Card* active_card = find_memory_card(address);
  
  return active_card->mem_read(address);
}

// ========================================
//
// ----------------------------------------
uint8_t Buss::io_read(uint8_t port) const
{
  I_Card* active_card = find_io_card(port);
  
  return active_card->io_read(port);
}

// ========================================
//
// ----------------------------------------
I_Card* Buss::find_memory_card(uint16_t address) const
{
  I_Card* active_card = NULL;

  for (I_Card* card : cards_)
  {
    if (card->mem_responds_to(address))
    {
      active_card = card;
    }
  }

  return active_card;
}

// ========================================
//
// ----------------------------------------
I_Card* Buss::find_io_card(uint8_t port) const
{
  I_Card* active_card = NULL;

  for (I_Card* card : cards_)
  {
    if (card->io_responds_to(port))
    {
      active_card = card;
    }
  }

  return active_card;
}
