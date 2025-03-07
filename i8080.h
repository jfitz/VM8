#ifndef I8080_I8080_H_
#define I8080_I8080_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef class i8080 {
 public:
  // memory + io interface
  uint8_t (*read_byte)(void*, uint16_t); // user function to read from memory
  void (*write_byte)(void*, uint16_t, uint8_t); // same for writing to memory
  uint8_t (*port_in)(void*, uint8_t); // user function to read from port
  void (*port_out)(void*, uint8_t, uint8_t); // same for writing to port
  void* userdata_; // user custom pointer

  unsigned long cyc; // cycle count

  // program counter, stack pointer
 private:
  uint16_t pc_;
  uint16_t sp_;

 public:
  // registers
  uint8_t a_, b_;
 private:
  uint8_t c_;
 public:
  uint8_t d_, e_, h_, l_;

  // flags: sign, zero, half-carry, parity, carry, interrupt flip-flop
  bool sf : 1, zf : 1, hf : 1, pf : 1, cf : 1, iff : 1;
  bool halted : 1;

  bool interrupt_pending : 1;
  uint8_t interrupt_vector;
  uint8_t interrupt_delay;

 public:
  uint8_t c() const;
  void set_c(uint8_t c);

  uint16_t pc() const;
  void set_pc(uint16_t pc);

  uint16_t sp() const;
  void set_sp(uint16_t sp);

  uint16_t bc() const;
  void set_bc(uint16_t bc);

  uint16_t de() const;
  void set_de(uint16_t de);

  uint16_t hl() const;
  void set_hl(uint16_t de);

  uint8_t rb(uint16_t addr);
  void wb(uint16_t addr, uint8_t val);
} i8080;

extern "C" void i8080_init(i8080* const c);
extern "C" void i8080_step(i8080* const c);
void i8080_interrupt(i8080* const c, uint8_t opcode);
void i8080_debug_output(i8080* const c, bool print_disassembly);

#endif // I8080_I8080_H_
