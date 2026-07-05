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
void Buss::set_memory(I_Card* card)
{
  memory_card_ = card;
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write(uint16_t address, uint8_t value)
{
  memory_card_->mem_write(address, value);
}

// ========================================
//
// ----------------------------------------
void Buss::mem_write_block(uint16_t dest_address, uint8_t* source_address, uint16_t count)
{
  unsigned int index = 0;

  while (index <= count)
  {
    memory_card_->mem_write(dest_address, source_address[index]);

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
  return memory_card_->mem_read(address);
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
I_Card* Buss::find_io_card(uint8_t port) const
{
  I_Card* active_io_card = NULL;

  for (I_Card* io_card : io_cards_)
  {
    if (io_card->io_responds_to(port))
    {
      active_io_card = io_card;
    }
  }

  return active_io_card;
}
