#include "i8080.h"

// this array defines the number of cycles one opcode takes.
// note that there are some special cases: conditional RETs and CALLs
// add +6 cycles if the condition is met
// clang-format off
static const uint8_t OPCODES_CYCLES[256] = {
//  0  1   2   3   4   5   6   7   8  9   A   B   C   D   E  F
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 0
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 1
    4, 10, 16, 5,  5,  5,  7,  4,  4, 10, 16, 5,  5,  5,  7, 4,  // 2
    4, 10, 13, 5,  10, 10, 10, 4,  4, 10, 13, 5,  5,  5,  7, 4,  // 3
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 4
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 5
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 6
    7, 7,  7,  7,  7,  7,  7,  7,  5, 5,  5,  5,  5,  5,  7, 5,  // 7
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 8
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 9
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // A
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // B
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // C
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // D
    5, 10, 10, 18, 11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11, // E
    5, 10, 10, 4,  11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11  // F
};
// clang-format on

static const char* DISASSEMBLE_TABLE[] = {
    "nop", "lxi b,#", "stax b", "inx b",
    "inr b", "dcr b", "mvi b,#", "rlc", "ill", "dad b", "ldax b", "dcx b",
    "inr c", "dcr c", "mvi c,#", "rrc", "ill", "lxi d,#", "stax d", "inx d",
    "inr d", "dcr d", "mvi d,#", "ral", "ill", "dad d", "ldax d", "dcx d",
    "inr e", "dcr e", "mvi e,#", "rar", "ill", "lxi h,#", "shld", "inx h",
    "inr h", "dcr h", "mvi h,#", "daa", "ill", "dad h", "lhld", "dcx h",
    "inr l", "dcr l", "mvi l,#", "cma", "ill", "lxi sp,#", "sta $", "inx sp",
    "inr M", "dcr M", "mvi M,#", "stc", "ill", "dad sp", "lda $", "dcx sp",
    "inr a", "dcr a", "mvi a,#", "cmc", "mov b,b", "mov b,c", "mov b,d",
    "mov b,e", "mov b,h", "mov b,l", "mov b,M", "mov b,a", "mov c,b", "mov c,c",
    "mov c,d", "mov c,e", "mov c,h", "mov c,l", "mov c,M", "mov c,a", "mov d,b",
    "mov d,c", "mov d,d", "mov d,e", "mov d,h", "mov d,l", "mov d,M", "mov d,a",
    "mov e,b", "mov e,c", "mov e,d", "mov e,e", "mov e,h", "mov e,l", "mov e,M",
    "mov e,a", "mov h,b", "mov h,c", "mov h,d", "mov h,e", "mov h,h", "mov h,l",
    "mov h,M", "mov h,a", "mov l,b", "mov l,c", "mov l,d", "mov l,e", "mov l,h",
    "mov l,l", "mov l,M", "mov l,a", "mov M,b", "mov M,c", "mov M,d", "mov M,e",
    "mov M,h", "mov M,l", "hlt", "mov M,a", "mov a,b", "mov a,c", "mov a,d",
    "mov a,e", "mov a,h", "mov a,l", "mov a,M", "mov a,a", "add b", "add c",
    "add d", "add e", "add h", "add l", "add M", "add a", "adc b", "adc c",
    "adc d", "adc e", "adc h", "adc l", "adc M", "adc a", "sub b", "sub c",
    "sub d", "sub e", "sub h", "sub l", "sub M", "sub a", "sbb b", "sbb c",
    "sbb d", "sbb e", "sbb h", "sbb l", "sbb M", "sbb a", "ana b", "ana c",
    "ana d", "ana e", "ana h", "ana l", "ana M", "ana a", "xra b", "xra c",
    "xra d", "xra e", "xra h", "xra l", "xra M", "xra a", "ora b", "ora c",
    "ora d", "ora e", "ora h", "ora l", "ora M", "ora a", "cmp b", "cmp c",
    "cmp d", "cmp e", "cmp h", "cmp l", "cmp M", "cmp a", "rnz", "pop b",
    "jnz $", "jmp $", "cnz $", "push b", "adi #", "rst 0", "rz", "ret", "jz $",
    "ill", "cz $", "call $", "aci #", "rst 1", "rnc", "pop d", "jnc $", "out p",
    "cnc $", "push d", "sui #", "rst 2", "rc", "ill", "jc $", "in p", "cc $",
    "ill", "sbi #", "rst 3", "rpo", "pop h", "jpo $", "xthl", "cpo $", "push h",
    "ani #", "rst 4", "rpe", "pchl", "jpe $", "xchg", "cpe $", "ill", "xri #",
    "rst 5", "rp", "pop psw", "jp $", "di", "cp $", "push psw", "ori #",
    "rst 6", "rm", "sphl", "jm $", "ei", "cm $", "ill", "cpi #", "rst 7"
};

// ========================================
//
// ----------------------------------------
void i8080::set_zsp_flags(uint8_t val)
{
  f_z_ = (val) == 0;
  f_s_ = (val) >> 7;
  f_p_ = parity(val);
}

// ========================================
//
// ----------------------------------------
uint16_t i8080::pc() const {
  return pc_;
}

// ========================================
//
// ----------------------------------------
void i8080::set_pc(uint16_t pc) {
  pc_ = pc;
}

// ========================================
//
// ----------------------------------------
uint16_t i8080::sp() const {
  return sp_;
}

// ========================================
//
// ----------------------------------------
void i8080::set_sp(uint16_t sp) {
  sp_ = sp;
}

// ========================================
//
// ----------------------------------------
uint16_t i8080::bc() const {
  return (r_b_ << 8) | r_c_;
}

// ========================================
//
// ----------------------------------------
void i8080::set_bc(uint16_t val) {
  r_b_ = val >> 8;
  r_c_ = val & 0xFF;
}

// ========================================
//
// ----------------------------------------
uint16_t i8080::de() const {
  return (r_d_ << 8) | r_e_;
}

// ========================================
//
// ----------------------------------------
void i8080::set_de(uint16_t val) {
  r_d_ = val >> 8;
  r_e_ = val & 0xFF;
}

// ========================================
//
// ----------------------------------------
uint16_t i8080::hl() const {
  return (r_h_ << 8) | r_l_;
}

// ========================================
//
// ----------------------------------------
void i8080::set_hl(uint16_t val) {
  r_h_ = val >> 8;
  r_l_ = val & 0xFF;
}

// memory helpers (the only four to use `read_byte` and `write_byte` function
// pointers)

// ========================================
// reads a byte from memory
// ----------------------------------------
uint8_t i8080::rb(uint16_t addr) {
  return read_byte(userdata_, addr);
}

// ========================================
// writes a byte to memory
// ----------------------------------------
void i8080::wb(uint16_t addr, uint8_t val) {
  write_byte(userdata_, addr, val);
}

// ========================================
// reads a word from memory
// ----------------------------------------
uint16_t i8080::rw(uint16_t addr) {
  return read_byte(userdata_, addr + 1) << 8 |
         read_byte(userdata_, addr);
}

// ========================================
// writes a word to memory
// ----------------------------------------
void i8080::ww(uint16_t addr, uint16_t val) {
  write_byte(userdata_, addr, val & 0xFF);
  write_byte(userdata_, addr + 1, val >> 8);
}

// ========================================
// returns the next byte in memory (and updates the program counter)
// ----------------------------------------
uint8_t i8080::pc_next_byte() {
  uint16_t result = rb(pc_);
  pc_ += 1;

  return result;
}

// ========================================
// returns the next word in memory (and updates the program counter)
// ----------------------------------------
uint16_t i8080::pc_next_word() {
  uint16_t result = rw(pc_);
  pc_ += 2;

  return result;
}

// stack helpers

// ========================================
// pushes a value into the stack and updates the stack pointer
// ----------------------------------------
void i8080::push_stack(uint16_t val) {
  sp_ -= 2;
  ww(sp_, val);
}

// ========================================
// pops a value from the stack and updates the stack pointer
// ----------------------------------------
uint16_t i8080::pop_stack() {
  uint16_t val = rw(sp_);
  sp_ += 2;

  return val;
}

// opcodes

// ========================================
// returns the parity of byte: 0 if number of 1 bits in `val` is odd, else 1
// ----------------------------------------
bool i8080::parity(uint8_t val) {
  uint8_t nb_one_bits = 0;

  for (int i = 0; i < 8; i++) {
    nb_one_bits += ((val >> i) & 1);
  }

  return (nb_one_bits & 1) == 0;
}

// ========================================
// returns if there was a carry between bit "bit_no" and "bit_no - 1" when
// executing "a + b + cy"
// ----------------------------------------
bool i8080::carry(int bit_no, uint8_t a, uint8_t b, int16_t result16) {

  int16_t carry_bits = result16 ^ a ^ b;

  return carry_bits & (1 << bit_no);
}

// ========================================
// adds a value (+ an optional carry flag) to a register
// ----------------------------------------
void i8080::add(uint8_t* const reg, uint8_t val, bool cy) {
  uint8_t result8 = *reg + val + cy;
  int16_t result16 = *reg + val + cy;
  f_c_ = i8080::carry(8, *reg, val, result16);
  f_h_ = i8080::carry(4, *reg, val, result16);
  set_zsp_flags(result8);

  *reg = result8;
}

// ========================================
// subtracts a byte (+ an optional carry flag) from a register
// see https://stackoverflow.com/a/8037485
// ----------------------------------------
void i8080::sub(uint8_t* const reg, uint8_t val, bool cy) {
  // call add() which will set flags
  add(reg, ~val, !cy);
  f_c_ = !f_c_;
}

// ========================================
// adds a word to HL
// ----------------------------------------
void i8080::op_dad(uint16_t val) {
  f_c_ = ((hl() + val) >> 16) & 1;
  set_hl(hl() + val);
}

// ========================================
// increments a byte
// ----------------------------------------
uint8_t i8080::inr(uint8_t val) {
  uint8_t result = val + 1;
  f_h_ = (result & 0xF) == 0;
  set_zsp_flags(result);

  return result;
}

// ========================================
// decrements a byte
// ----------------------------------------
uint8_t i8080::dcr(uint8_t val) {
  uint8_t result = val - 1;
  f_h_ = !((result & 0xF) == 0xF);
  set_zsp_flags(result);

  return result;
}

// ========================================
// executes a logic "and" between register A and a byte, then stores the
// result in register A
// ----------------------------------------
void i8080::op_ana(uint8_t val) {
  uint8_t result = r_a_ & val;
  f_c_ = 0;
  f_h_ = ((r_a_ | val) & 0x08) != 0;

  set_zsp_flags(result);
  r_a_ = result;
}

// ========================================
// executes a logic "xor" between register A and a byte, then stores the
// result in register A
// ----------------------------------------
void i8080::op_xra(uint8_t val) {
  r_a_ ^= val;
  f_c_ = 0;
  f_h_ = 0;

  set_zsp_flags(r_a_);
}

// ========================================
// executes a logic "or" between register A and a byte, then stores the
// result in register A
// ----------------------------------------
void i8080::op_ora(uint8_t val) {
  r_a_ |= val;
  f_c_ = 0;
  f_h_ = 0;

  set_zsp_flags(r_a_);
}

// ========================================
// compares the register A to another byte
// ----------------------------------------
void i8080::op_cmp(uint8_t val) {
  int16_t result = r_a_ - val;
  f_c_ = result >> 8;
  f_h_ = ~(r_a_ ^ result ^ val) & 0x10;

  set_zsp_flags(result & 0xFF);
}

// ========================================
// sets the program counter to a given address
// ----------------------------------------
void i8080::jump(uint16_t addr) {
  set_pc(addr);
}

// ========================================
// jumps to next address pointed by the next word in memory if a condition
// is met
// ----------------------------------------
void i8080::cond_jump(bool condition) {
  uint16_t addr = pc_next_word();

  if (condition) {
    set_pc(addr);
  }
}

// ========================================
// pushes the current pc to the stack, then jumps to an address
// ----------------------------------------
void i8080::call(uint16_t addr) {
  push_stack(pc());
  jump(addr);
}

// ========================================
// calls to next word in memory if a condition is met
// ----------------------------------------
void i8080::cond_call(bool condition) {
  uint16_t addr = pc_next_word();

  if (condition) {
    call(addr);
    cyc_ += 6;
  }
}

// ========================================
// returns from subroutine
// ----------------------------------------
void i8080::op_ret() {
  set_pc(pop_stack());
}

// ========================================
// returns from subroutine if a condition is met
// ----------------------------------------
void i8080::cond_ret(bool condition) {
  if (condition) {
    op_ret();
    cyc_ += 6;
  }
}

// ========================================
// pushes register A and the flags into the stack
// ----------------------------------------
void i8080::op_push_psw() {
  // note: bit 3 and 5 are always 0
  uint8_t psw = 0;

  psw |= f_s_ << 7;
  psw |= f_z_ << 6;
  psw |= f_h_ << 4;
  psw |= f_p_ << 2;
  psw |= 1    << 1; // bit 1 is always 1
  psw |= f_c_ << 0;

  push_stack(r_a_ << 8 | psw);
}

// ========================================
// pops register A and the flags from the stack
// ----------------------------------------
void i8080::op_pop_psw() {
  uint16_t psw = pop_stack();
  r_a_ = psw >> 8;
  uint8_t r_f = psw & 0xFF;

  f_s_ = (r_f >> 7) & 1;
  f_z_ = (r_f >> 6) & 1;
  f_h_ = (r_f >> 4) & 1;
  f_p_ = (r_f >> 2) & 1;
  f_c_ = (r_f >> 0) & 1;
}

// ========================================
// rotate register A left
// ----------------------------------------
void i8080::op_rlc() {
  f_c_ = r_a_ >> 7;
  r_a_ = (r_a_ << 1) | f_c_;
}

// ========================================
// rotate register A right
// ----------------------------------------
void i8080::op_rrc() {
  f_c_ = r_a_ & 1;
  r_a_ = (r_a_ >> 1) | (f_c_ << 7);
}

// ========================================
// rotate register A left with the carry flag
// ----------------------------------------
void i8080::op_ral() {
  bool cy = f_c_;
  f_c_ = r_a_ >> 7;
  r_a_ = (r_a_ << 1) | cy;
}

// ========================================
// rotate register A right with the carry flag
// ----------------------------------------
void i8080::op_rar() {
  bool cy = f_c_;
  f_c_ = r_a_ & 1;
  r_a_ = (r_a_ >> 1) | (cy << 7);
}

// ========================================
// Decimal Adjust Accumulator: the eight-bit number in register A is adjusted
// to form two four-bit binary-coded-decimal digits.
// For example, if A=$2B and DAA is executed, A becomes $31.
// ----------------------------------------
void i8080::op_daa() {
  bool cy = f_c_;
  uint8_t correction = 0;

  uint8_t lsb = r_a_ & 0x0F;
  uint8_t msb = r_a_ >> 4;

  if (f_h_ || lsb > 9) {
    correction += 0x06;
  }

  if (f_c_ || msb > 9 || (msb >= 9 && lsb > 9)) {
    correction += 0x60;
    cy = 1;
  }

  add(&r_a_, correction, 0);
  f_c_ = cy;
}

// ========================================
// switches the value of registers DE and HL
// ----------------------------------------
void i8080::op_xchg() {
  uint16_t val = de();
  set_de(hl());
  set_hl(val);
}

// ========================================
// switches the value of a word at (sp) and HL
// ----------------------------------------
void i8080::op_xthl() {
  uint16_t val = rw(sp());
  ww(sp(), hl());
  set_hl(val);
}

// ========================================
// executes one opcode
// ----------------------------------------
static inline void i8080_execute(i8080* const c, uint8_t opcode) {
  c->cyc_ += OPCODES_CYCLES[opcode];

  // when DI is executed, interrupts won't be serviced
  // until the end of next instruction:
  if (c->interrupt_delay_ > 0) {
    c->interrupt_delay_ -= 1;
  }

  switch (opcode) {
  case 0x7F: c->r_a_ = c->r_a_; break; // MOV A,A
  case 0x78: c->r_a_ = c->r_b_; break; // MOV A,B
  case 0x79: c->r_a_ = c->r_c_; break; // MOV A,C
  case 0x7A: c->r_a_ = c->r_d_; break; // MOV A,D
  case 0x7B: c->r_a_ = c->r_e_; break; // MOV A,E
  case 0x7C: c->r_a_ = c->r_h_; break; // MOV A,H
  case 0x7D: c->r_a_ = c->r_l_; break; // MOV A,L
  case 0x7E: c->r_a_ = c->rb(c->hl()); break; // MOV A,M

  case 0x0A: c->r_a_ = c->rb(c->bc()); break; // LDAX B
  case 0x1A: c->r_a_ = c->rb(c->de()); break; // LDAX D
  case 0x3A: c->r_a_ = c->rb(c->pc_next_word()); break; // LDA word

  case 0x47: c->r_b_ = c->r_a_; break; // MOV B,A
  case 0x40: c->r_b_ = c->r_b_; break; // MOV B,B
  case 0x41: c->r_b_ = c->r_c_; break; // MOV B,C
  case 0x42: c->r_b_ = c->r_d_; break; // MOV B,D
  case 0x43: c->r_b_ = c->r_e_; break; // MOV B,E
  case 0x44: c->r_b_ = c->r_h_; break; // MOV B,H
  case 0x45: c->r_b_ = c->r_l_; break; // MOV B,L
  case 0x46: c->r_b_ = c->rb(c->hl()); break; // MOV B,M

  case 0x4F: c->r_c_ = c->r_a_; break; // MOV C,A
  case 0x48: c->r_c_ = c->r_b_; break; // MOV C,B
  case 0x49: c->r_c_ = c->r_c_; break; // MOV C,C
  case 0x4A: c->r_c_ = c->r_d_; break; // MOV C,D
  case 0x4B: c->r_c_ = c->r_e_; break; // MOV C,E
  case 0x4C: c->r_c_ = c->r_h_; break; // MOV C,H
  case 0x4D: c->r_c_ = c->r_l_; break; // MOV C,L
  case 0x4E: c->r_c_ = c->rb(c->hl()); break; // MOV C,M

  case 0x57: c->r_d_ = c->r_a_; break; // MOV D,A
  case 0x50: c->r_d_ = c->r_b_; break; // MOV D,B
  case 0x51: c->r_d_ = c->r_c_; break; // MOV D,C
  case 0x52: c->r_d_ = c->r_d_; break; // MOV D,D
  case 0x53: c->r_d_ = c->r_e_; break; // MOV D,E
  case 0x54: c->r_d_ = c->r_h_; break; // MOV D,H
  case 0x55: c->r_d_ = c->r_l_; break; // MOV D,L
  case 0x56: c->r_d_ = c->rb(c->hl()); break; // MOV D,M

  case 0x5F: c->r_e_ = c->r_a_; break; // MOV E,A
  case 0x58: c->r_e_ = c->r_b_; break; // MOV E,B
  case 0x59: c->r_e_ = c->r_c_; break; // MOV E,C
  case 0x5A: c->r_e_ = c->r_d_; break; // MOV E,D
  case 0x5B: c->r_e_ = c->r_e_; break; // MOV E,E
  case 0x5C: c->r_e_ = c->r_h_; break; // MOV E,H
  case 0x5D: c->r_e_ = c->r_l_; break; // MOV E,L
  case 0x5E: c->r_e_ = c->rb(c->hl()); break; // MOV E,M

  case 0x67: c->r_h_ = c->r_a_; break; // MOV H,A
  case 0x60: c->r_h_ = c->r_b_; break; // MOV H,B
  case 0x61: c->r_h_ = c->r_c_; break; // MOV H,C
  case 0x62: c->r_h_ = c->r_d_; break; // MOV H,D
  case 0x63: c->r_h_ = c->r_e_; break; // MOV H,E
  case 0x64: c->r_h_ = c->r_h_; break; // MOV H,H
  case 0x65: c->r_h_ = c->r_l_; break; // MOV H,L
  case 0x66: c->r_h_ = c->rb(c->hl()); break; // MOV H,M

  case 0x6F: c->r_l_ = c->r_a_; break; // MOV L,A
  case 0x68: c->r_l_ = c->r_b_; break; // MOV L,B
  case 0x69: c->r_l_ = c->r_c_; break; // MOV L,C
  case 0x6A: c->r_l_ = c->r_d_; break; // MOV L,D
  case 0x6B: c->r_l_ = c->r_e_; break; // MOV L,E
  case 0x6C: c->r_l_ = c->r_h_; break; // MOV L,H
  case 0x6D: c->r_l_ = c->r_l_; break; // MOV L,L
  case 0x6E: c->r_l_ = c->rb(c->hl()); break; // MOV L,M

  case 0x77: c->wb(c->hl(), c->r_a_); break; // MOV M,A
  case 0x70: c->wb(c->hl(), c->r_b_); break; // MOV M,B
  case 0x71: c->wb(c->hl(), c->r_c_); break; // MOV M,C
  case 0x72: c->wb(c->hl(), c->r_d_); break; // MOV M,D
  case 0x73: c->wb(c->hl(), c->r_e_); break; // MOV M,E
  case 0x74: c->wb(c->hl(), c->r_h_); break; // MOV M,H
  case 0x75: c->wb(c->hl(), c->r_l_); break; // MOV M,L

  case 0x3E: c->r_a_ = c->pc_next_byte(); break; // MVI A,byte
  case 0x06: c->r_b_ = c->pc_next_byte(); break; // MVI B,byte
  case 0x0E: c->r_c_ = c->pc_next_byte(); break; // MVI C,byte
  case 0x16: c->r_d_ = c->pc_next_byte(); break; // MVI D,byte
  case 0x1E: c->r_e_ = c->pc_next_byte(); break; // MVI E,byte
  case 0x26: c->r_h_ = c->pc_next_byte(); break; // MVI H,byte
  case 0x2E: c->r_l_ = c->pc_next_byte(); break; // MVI L,byte
  case 0x36:
    c->wb(c->hl(), c->pc_next_byte());
    break; // MVI M,byte

  case 0x02: c->wb(c->bc(), c->r_a_);    break; // STAX B
  case 0x12: c->wb(c->de(), c->r_a_);    break; // STAX D
  case 0x32: c->wb(c->pc_next_word(), c->r_a_); break; // STA word

  case 0x01: c->set_bc(c->pc_next_word()); break; // LXI B,word
  case 0x11: c->set_de(c->pc_next_word()); break; // LXI D,word
  case 0x21: c->set_hl(c->pc_next_word()); break; // LXI H,word
  case 0x31: c->set_sp(c->pc_next_word()); break; // LXI SP,word
  case 0x2A: c->set_hl(c->rw(c->pc_next_word())); break; // LHLD
  case 0x22: c->ww(c->pc_next_word(), c->hl()); break; // SHLD
  case 0xF9: c->set_sp(c->hl());                break; // SPHL

  case 0xEB: c->op_xchg(); break; // XCHG
  case 0xE3: c->op_xthl(); break; // XTHL

  case 0x87: c->add(&c->r_a_, c->r_a_, 0); break; // ADD A
  case 0x80: c->add(&c->r_a_, c->r_b_, 0); break; // ADD B
  case 0x81: c->add(&c->r_a_, c->r_c_, 0); break; // ADD C
  case 0x82: c->add(&c->r_a_, c->r_d_, 0); break; // ADD D
  case 0x83: c->add(&c->r_a_, c->r_e_, 0); break; // ADD E
  case 0x84: c->add(&c->r_a_, c->r_h_, 0); break; // ADD H
  case 0x85: c->add(&c->r_a_, c->r_l_, 0); break; // ADD L
  case 0x86:
    c->add(&c->r_a_, c->rb(c->hl()), 0);
    break; // ADD M

  case 0xC6: c->add(&c->r_a_, c->pc_next_byte(), 0); break; // ADI byte

  case 0x8F: c->add(&c->r_a_, c->r_a_, c->f_c_); break; // ADC A
  case 0x88: c->add(&c->r_a_, c->r_b_, c->f_c_); break; // ADC B
  case 0x89: c->add(&c->r_a_, c->r_c_, c->f_c_); break; // ADC C
  case 0x8A: c->add(&c->r_a_, c->r_d_, c->f_c_); break; // ADC D
  case 0x8B: c->add(&c->r_a_, c->r_e_, c->f_c_); break; // ADC E
  case 0x8C: c->add(&c->r_a_, c->r_h_, c->f_c_); break; // ADC H
  case 0x8D: c->add(&c->r_a_, c->r_l_, c->f_c_); break; // ADC L
  case 0x8E:
    c->add(&c->r_a_, c->rb(c->hl()), c->f_c_);
    break; // ADC M

  case 0xCE: c->add(&c->r_a_, c->pc_next_byte(), c->f_c_); break; // ACI byte

  case 0x97: c->sub(&c->r_a_, c->r_a_, 0); break; // SUB A
  case 0x90: c->sub(&c->r_a_, c->r_b_, 0); break; // SUB B
  case 0x91: c->sub(&c->r_a_, c->r_c_, 0); break; // SUB C
  case 0x92: c->sub(&c->r_a_, c->r_d_, 0); break; // SUB D
  case 0x93: c->sub(&c->r_a_, c->r_e_, 0); break; // SUB E
  case 0x94: c->sub(&c->r_a_, c->r_h_, 0); break; // SUB H
  case 0x95: c->sub(&c->r_a_, c->r_l_, 0); break; // SUB L
  case 0x96:
    c->sub(&c->r_a_, c->rb(c->hl()), 0);
    break; // SUB M

  case 0xD6: c->sub(&c->r_a_, c->pc_next_byte(), 0); break; // SUI byte

  case 0x9F: c->sub(&c->r_a_, c->r_a_, c->f_c_); break; // SBB A
  case 0x98: c->sub(&c->r_a_, c->r_b_, c->f_c_); break; // SBB B
  case 0x99: c->sub(&c->r_a_, c->r_c_, c->f_c_); break; // SBB C
  case 0x9A: c->sub(&c->r_a_, c->r_d_, c->f_c_); break; // SBB D
  case 0x9B: c->sub(&c->r_a_, c->r_e_, c->f_c_); break; // SBB E
  case 0x9C: c->sub(&c->r_a_, c->r_h_, c->f_c_); break; // SBB H
  case 0x9D: c->sub(&c->r_a_, c->r_l_, c->f_c_); break; // SBB L
  case 0x9E:
    c->sub(&c->r_a_, c->rb(c->hl()), c->f_c_);
    break; // SBB M

  case 0xDE: c->sub(&c->r_a_, c->pc_next_byte(), c->f_c_); break; // SBI byte

  case 0x09: c->op_dad(c->bc()); break; // DAD B
  case 0x19: c->op_dad(c->de()); break; // DAD D
  case 0x29: c->op_dad(c->hl()); break; // DAD H
  case 0x39: c->op_dad(c->sp()); break; // DAD SP

  case 0xF3: c->f_i_ = 0; break; // DI
  case 0xFB:
    c->f_i_ = 1;
    c->interrupt_delay_ = 1;
    break; // EI

  case 0x00: break; // NOP
  case 0x76: c->halted_ = 1; break; // HLT

  case 0x3C: c->r_a_ = c->inr(c->r_a_); break; // INR A
  case 0x04: c->r_b_ = c->inr(c->r_b_); break; // INR B
  case 0x0C: c->r_c_ = c->inr(c->r_c_); break; // INR C
  case 0x14: c->r_d_ = c->inr(c->r_d_); break; // INR D
  case 0x1C: c->r_e_ = c->inr(c->r_e_); break; // INR E
  case 0x24: c->r_h_ = c->inr(c->r_h_); break; // INR H
  case 0x2C: c->r_l_ = c->inr(c->r_l_); break; // INR L
  case 0x34:
    c->wb(c->hl(), c->inr(c->rb(c->hl())));
    break; // INR M

  case 0x3D: c->r_a_ = c->dcr(c->r_a_); break; // DCR A
  case 0x05: c->r_b_ = c->dcr(c->r_b_); break; // DCR B
  case 0x0D: c->r_c_ = c->dcr(c->r_c_); break; // DCR C
  case 0x15: c->r_d_ = c->dcr(c->r_d_); break; // DCR D
  case 0x1D: c->r_e_ = c->dcr(c->r_e_); break; // DCR E
  case 0x25: c->r_h_ = c->dcr(c->r_h_); break; // DCR H
  case 0x2D: c->r_l_ = c->dcr(c->r_l_); break; // DCR L
  case 0x35:
    c->wb(c->hl(), c->dcr(c->rb(c->hl())));
    break; // DCR M

  case 0x03: c->set_bc(c->bc() + 1); break; // INX B
  case 0x13: c->set_de(c->de() + 1); break; // INX D
  case 0x23: c->set_hl(c->hl() + 1); break; // INX H
  case 0x33: c->set_sp(c->sp() + 1); break; // INX SP

  case 0x0B: c->set_bc(c->bc() - 1); break; // DCX B
  case 0x1B: c->set_de(c->de() - 1); break; // DCX D
  case 0x2B: c->set_hl(c->hl() - 1); break; // DCX H
  case 0x3B: c->set_sp(c->sp() - 1); break; // DCX SP

  case 0x27: c->op_daa();          break; // DAA
  case 0x2F: c->r_a_ = ~c->r_a_;   break; // CMA
  case 0x37: c->f_c_ = 1;          break; // STC
  case 0x3F: c->f_c_ = !c->f_c_;   break; // CMC

  case 0x07: c->op_rlc(); break; // RLC (rotate left)
  case 0x0F: c->op_rrc(); break; // RRC (rotate right)
  case 0x17: c->op_ral(); break; // RAL
  case 0x1F: c->op_rar(); break; // RAR

  case 0xA7: c->op_ana(c->r_a_); break; // ANA A
  case 0xA0: c->op_ana(c->r_b_); break; // ANA B
  case 0xA1: c->op_ana(c->r_c_); break; // ANA C
  case 0xA2: c->op_ana(c->r_d_); break; // ANA D
  case 0xA3: c->op_ana(c->r_e_); break; // ANA E
  case 0xA4: c->op_ana(c->r_h_); break; // ANA H
  case 0xA5: c->op_ana(c->r_l_); break; // ANA L
  case 0xA6: c->op_ana(c->rb(c->hl())); break; // ANA M

  case 0xE6: c->op_ana(c->pc_next_byte()); break; // ANI byte

  case 0xAF: c->op_xra(c->r_a_); break; // XRA A
  case 0xA8: c->op_xra(c->r_b_); break; // XRA B
  case 0xA9: c->op_xra(c->r_c_); break; // XRA C
  case 0xAA: c->op_xra(c->r_d_); break; // XRA D
  case 0xAB: c->op_xra(c->r_e_); break; // XRA E
  case 0xAC: c->op_xra(c->r_h_); break; // XRA H
  case 0xAD: c->op_xra(c->r_l_); break; // XRA L
  case 0xAE: c->op_xra(c->rb(c->hl())); break; // XRA M

  case 0xEE: c->op_xra(c->pc_next_byte()); break; // XRI byte

  case 0xB7: c->op_ora(c->r_a_); break; // ORA A
  case 0xB0: c->op_ora(c->r_b_); break; // ORA B
  case 0xB1: c->op_ora(c->r_c_); break; // ORA C
  case 0xB2: c->op_ora(c->r_d_); break; // ORA D
  case 0xB3: c->op_ora(c->r_e_); break; // ORA E
  case 0xB4: c->op_ora(c->r_h_); break; // ORA H
  case 0xB5: c->op_ora(c->r_l_); break; // ORA L
  case 0xB6: c->op_ora(c->rb(c->hl())); break; // ORA M

  case 0xF6: c->op_ora(c->pc_next_byte()); break; // ORI byte

  case 0xBF: c->op_cmp(c->r_a_); break; // CMP A
  case 0xB8: c->op_cmp(c->r_b_); break; // CMP B
  case 0xB9: c->op_cmp(c->r_c_); break; // CMP C
  case 0xBA: c->op_cmp(c->r_d_); break; // CMP D
  case 0xBB: c->op_cmp(c->r_e_); break; // CMP E
  case 0xBC: c->op_cmp(c->r_h_); break; // CMP H
  case 0xBD: c->op_cmp(c->r_l_); break; // CMP L
  case 0xBE: c->op_cmp(c->rb(c->hl())); break; // CMP M
  case 0xFE: c->op_cmp(c->pc_next_byte());           break; // CPI byte

  case 0xC3: c->jump(c->pc_next_word());  break; // JMP
  case 0xC2: c->cond_jump(c->f_z_ == 0);    break; // JNZ
  case 0xCA: c->cond_jump(c->f_z_ == 1);    break; // JZ
  case 0xD2: c->cond_jump(c->f_c_ == 0);    break; // JNC
  case 0xDA: c->cond_jump(c->f_c_ == 1);    break; // JC
  case 0xE2: c->cond_jump(c->f_p_ == 0);    break; // JPO
  case 0xEA: c->cond_jump(c->f_p_ == 1);    break; // JPE
  case 0xF2: c->cond_jump(c->f_s_ == 0);    break; // JP
  case 0xFA: c->cond_jump(c->f_s_ == 1);    break; // JM

  case 0xE9: c->set_pc(c->hl());         break; // PCHL
  case 0xCD: c->call(c->pc_next_word()); break; // CALL

  case 0xC4: c->cond_call(c->f_z_ == 0); break; // CNZ
  case 0xCC: c->cond_call(c->f_z_ == 1); break; // CZ
  case 0xD4: c->cond_call(c->f_c_ == 0); break; // CNC
  case 0xDC: c->cond_call(c->f_c_ == 1); break; // CC
  case 0xE4: c->cond_call(c->f_p_ == 0); break; // CPO
  case 0xEC: c->cond_call(c->f_p_ == 1); break; // CPE
  case 0xF4: c->cond_call(c->f_s_ == 0); break; // CP
  case 0xFC: c->cond_call(c->f_s_ == 1); break; // CM

  case 0xC9: c->op_ret();               break; // RET
  case 0xC0: c->cond_ret(c->f_z_ == 0); break; // RNZ
  case 0xC8: c->cond_ret(c->f_z_ == 1); break; // RZ
  case 0xD0: c->cond_ret(c->f_c_ == 0); break; // RNC
  case 0xD8: c->cond_ret(c->f_c_ == 1); break; // RC
  case 0xE0: c->cond_ret(c->f_p_ == 0); break; // RPO
  case 0xE8: c->cond_ret(c->f_p_ == 1); break; // RPE
  case 0xF0: c->cond_ret(c->f_s_ == 0); break; // RP
  case 0xF8: c->cond_ret(c->f_s_ == 1); break; // RM

  case 0xC7: c->call(0x00); break; // RST 0
  case 0xCF: c->call(0x08); break; // RST 1
  case 0xD7: c->call(0x10); break; // RST 2
  case 0xDF: c->call(0x18); break; // RST 3
  case 0xE7: c->call(0x20); break; // RST 4
  case 0xEF: c->call(0x28); break; // RST 5
  case 0xF7: c->call(0x30); break; // RST 6
  case 0xFF: c->call(0x38); break; // RST 7

  case 0xC5: c->push_stack(c->bc()); break; // PUSH B
  case 0xD5: c->push_stack(c->de()); break; // PUSH D
  case 0xE5: c->push_stack(c->hl()); break; // PUSH H
  case 0xF5: c->op_push_psw();       break; // PUSH PSW
  case 0xC1: c->set_bc(c->pop_stack());  break; // POP B
  case 0xD1: c->set_de(c->pop_stack());  break; // POP D
  case 0xE1: c->set_hl(c->pop_stack());  break; // POP H
  case 0xF1: c->op_pop_psw();            break; // POP PSW

  case 0xDB: c->r_a_ = c->port_in(c->userdata_, c->pc_next_byte()); break; // IN
  case 0xD3: c->port_out(c->userdata_, c->pc_next_byte(), c->r_a_); break; // OUT

  case 0x08:
  case 0x10:
  case 0x18:
  case 0x20:
  case 0x28:
  case 0x30:
  case 0x38: break; // undocumented NOPs

  case 0xD9: c->op_ret();                   break; // undocumented RET

  case 0xDD:
  case 0xED:
  case 0xFD: c->call(c->pc_next_word()); break; // undocumented CALLs

  case 0xCB: c->jump(c->pc_next_word());  break; // undocumented JMP
  }
}

// ========================================
// initializes the emulator with default values
// ----------------------------------------
void i8080::init() {
  read_byte = NULL;
  write_byte = NULL;
  port_in = NULL;
  port_out = NULL;
  userdata_ = NULL;

  cyc_ = 0;

  pc_ = 0;
  sp_ = 0;

  r_a_ = 0;
  r_b_ = 0;
  r_c_ = 0;
  r_d_ = 0;
  r_e_ = 0;
  r_h_ = 0;
  r_l_ = 0;

  f_s_ = 0;
  f_z_ = 0;
  f_h_ = 0;
  f_p_ = 0;
  f_c_ = 0;
  f_i_ = 0;

  halted_ = 0;
  interrupt_pending_ = 0;
  interrupt_vector_ = 0;
  interrupt_delay_ = 0;
}

// ========================================
// executes one instruction
// ----------------------------------------
void i8080::exec_step() {
  // interrupt processing: if an interrupt is pending and IFF is set,
  // we execute the interrupt vector passed by the user.
  if (interrupt_pending_ && f_i_ && interrupt_delay_ == 0) {
    interrupt_pending_ = 0;
    f_i_ = 0;
    halted_ = 0;

    i8080_execute(this, interrupt_vector_);
  } else if (!halted_) {
    i8080_execute(this, pc_next_byte());
  }
}

// ========================================
// asks for an interrupt to be serviced
// ----------------------------------------
void i8080::exec_interrupt(uint8_t opcode) {
  interrupt_pending_ = 1;
  interrupt_vector_ = opcode;
}

// ========================================
// outputs a debug trace of the emulator state to the standard output,
// including registers and flags
// ----------------------------------------
void i8080::debug_output(bool print_disassembly) {
  uint8_t f = 0;
  f |= f_s_ << 7;
  f |= f_z_ << 6;
  f |= f_h_ << 4;
  f |= f_p_ << 2;
  f |= 1    << 1; // bit 1 is always 1
  f |= f_c_ << 0;

  printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
	 pc(), r_a_ << 8 | f, bc(), de(), hl(), sp(), cyc_);

  uint16_t my_pc = pc();

  printf("\t(%02X %02X %02X %02X)", rb(my_pc), rb(my_pc + 1),
	 rb(my_pc + 2), rb(my_pc + 3));

  if (print_disassembly) {
    printf(" - %s", DISASSEMBLE_TABLE[rb(my_pc)]);
  }

  printf("\n");
}
