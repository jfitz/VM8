#pragma once

// C
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// project
#include "I_Buss.h"

class Intel8080 {
 public:
  // memory + io interface
  uint8_t (*read_byte)(uint16_t); // user function to read from memory
  void (*write_byte)(uint16_t, uint8_t); // same for writing to memory

 private:
  // cycle count
  unsigned long num_cycles_;
  unsigned long t_count_;

  I_Buss* buss_;
  // callback to supervisor for writing to port
  void (*supervisor_request_port_out_)(uint8_t, uint8_t, const Intel8080* cpu, const I_Buss* buss);
  // callback to supervisor to halt
  void (*supervisor_request_halt_)();

  // program counter, stack pointer
  uint16_t pc_;
  uint16_t sp_;

  // registers
  uint8_t r_a_, r_b_, r_c_, r_d_, r_e_, r_h_, r_l_;

  // flags: sign, zero, half-carry, parity, carry, interrupt flip-flop
  bool f_s_ : 1, f_z_ : 1, f_h_ : 1, f_p_ : 1, f_c_ : 1, f_i_ : 1;

 public:
  bool interrupt_pending_ : 1;
  uint8_t interrupt_vector_;
  uint8_t interrupt_delay_;

 public:
  // constructors
  Intel8080(
    I_Buss* buss,
    void (*port_out)(uint8_t, uint8_t, const Intel8080* cpu, const I_Buss* buss),
    void (*set_halted)()
  );

  // mutators
  void init();
  
  void set_zsp_flags(uint8_t val);
  
  // properties
  unsigned long num_cycles() const;

  // change state of 8080
  void set_pc(uint16_t pc);
  void set_sp(uint16_t sp);
  void set_bc(uint16_t bc);
  void set_de(uint16_t de);
  void set_hl(uint16_t de);

  // report state of 8080
  uint8_t r_c() const;
  uint8_t r_e() const;
  
  uint16_t rp_pc() const;
  uint16_t rp_sp() const;
  uint16_t rp_bc() const;
  uint16_t rp_de() const;
  uint16_t rp_hl() const;

  uint8_t rb(uint16_t addr);
  void wb(uint16_t addr, uint8_t val);
  uint16_t rw(uint16_t addr);
  void ww(uint16_t addr, uint16_t val);

  uint8_t pc_next_byte();
  uint16_t pc_next_word();

  void push_stack(uint16_t val);
  uint16_t pop_stack();

  static bool parity(uint8_t val);
  static bool carry(int bit_no, uint8_t a, uint8_t b, int16_t result16);

  // operations
  void add(uint8_t* const reg, uint8_t val, bool cy);
  void sub(uint8_t* const reg, uint8_t val, bool cy);

  void op_dad(uint16_t val);
  uint8_t op_inr(uint8_t val);
  void op_inm();
  uint8_t op_dcr(uint8_t val);
  void op_dcm();

  void op_ana(uint8_t val);
  void op_xra(uint8_t val);
  void op_ora(uint8_t val);
  void op_cmp(uint8_t val);

  void jump(uint16_t addr);
  void cond_jump(bool condition);
  void call(uint16_t addr);
  void cond_call(bool condition);
  void op_ret();
  void cond_ret(bool condition);

  void op_push_psw();
  void op_pop_psw();

  void op_rlc();
  void op_rrc();
  void op_ral();
  void op_rar();

  void op_daa();

  void op_xchg();
  void op_xthl();

  void exec_step();
  void exec_interrupt(uint8_t opcode);
  void debug_output(bool print_disassembly);
  void execute(uint8_t opcode);
};

