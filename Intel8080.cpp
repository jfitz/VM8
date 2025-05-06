#include "Intel8080.h"

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

Intel8080::Intel8080(
  uint8_t (*port_in)(void*, uint8_t),
  void (*port_out)(void*, uint8_t, uint8_t)
)
{
  read_byte = NULL;
  write_byte = NULL;
  supervisor_request_port_in_ = port_in;
  supervisor_request_port_out_ = port_out;
  userdata_ = NULL;

  init();
}

// ========================================
// initializes the emulator with default values
// ----------------------------------------
void Intel8080::init() {
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
//
// ----------------------------------------
void Intel8080::set_zsp_flags(uint8_t val)
{
  f_z_ = (val) == 0;
  f_s_ = (val) >> 7;
  f_p_ = parity(val);
}

// ========================================
//
// ----------------------------------------
uint8_t Intel8080::r_c() const {
  return r_c_;
}

// ========================================
//
// ----------------------------------------
uint8_t Intel8080::r_e() const {
  return r_e_;
}

// ========================================
//
// ----------------------------------------
uint16_t Intel8080::rp_pc() const {
  return pc_;
}

// ========================================
//
// ----------------------------------------
uint16_t Intel8080::rp_sp() const {
  return sp_;
}

// ========================================
//
// ----------------------------------------
uint16_t Intel8080::rp_bc() const {
  return (r_b_ << 8) | r_c_;
}

// ========================================
//
// ----------------------------------------
uint16_t Intel8080::rp_de() const {
  return (r_d_ << 8) | r_e_;
}

// ========================================
//
// ----------------------------------------
uint16_t Intel8080::rp_hl() const {
  return (r_h_ << 8) | r_l_;
}

// ========================================
//
// ----------------------------------------
void Intel8080::set_pc(uint16_t pc) {
  pc_ = pc;
}

// ========================================
//
// ----------------------------------------
void Intel8080::set_sp(uint16_t sp) {
  sp_ = sp;
}

// ========================================
//
// ----------------------------------------
void Intel8080::set_bc(uint16_t val) {
  r_b_ = val >> 8;
  r_c_ = val & 0xFF;
}

// ========================================
//
// ----------------------------------------
void Intel8080::set_de(uint16_t val) {
  r_d_ = val >> 8;
  r_e_ = val & 0xFF;
}

// ========================================
//
// ----------------------------------------
void Intel8080::set_hl(uint16_t val) {
  r_h_ = val >> 8;
  r_l_ = val & 0xFF;
}

// memory helpers (the only four to use `read_byte` and `write_byte` function
// pointers)

// ========================================
// reads a byte from memory
// ----------------------------------------
uint8_t Intel8080::rb(uint16_t addr) {
  return read_byte(userdata_, addr);
}

// ========================================
// writes a byte to memory
// ----------------------------------------
void Intel8080::wb(uint16_t addr, uint8_t val) {
  write_byte(userdata_, addr, val);
}

// ========================================
// reads a word from memory
// ----------------------------------------
uint16_t Intel8080::rw(uint16_t addr) {
  return read_byte(userdata_, addr + 1) << 8 |
         read_byte(userdata_, addr);
}

// ========================================
// writes a word to memory
// ----------------------------------------
void Intel8080::ww(uint16_t addr, uint16_t val) {
  write_byte(userdata_, addr, val & 0xFF);
  write_byte(userdata_, addr + 1, val >> 8);
}

// ========================================
// returns the next byte in memory (and updates the program counter)
// ----------------------------------------
uint8_t Intel8080::pc_next_byte() {
  uint16_t result = rb(pc_);
  pc_ += 1;

  return result;
}

// ========================================
// returns the next word in memory (and updates the program counter)
// ----------------------------------------
uint16_t Intel8080::pc_next_word() {
  uint16_t result = rw(pc_);
  pc_ += 2;

  return result;
}

// stack helpers

// ========================================
// pushes a value into the stack and updates the stack pointer
// ----------------------------------------
void Intel8080::push_stack(uint16_t val) {
  sp_ -= 2;
  ww(sp_, val);
}

// ========================================
// pops a value from the stack and updates the stack pointer
// ----------------------------------------
uint16_t Intel8080::pop_stack() {
  uint16_t val = rw(sp_);
  sp_ += 2;

  return val;
}

// opcodes

// ========================================
// returns the parity of byte: 0 if number of 1 bits in `val` is odd, else 1
// ----------------------------------------
bool Intel8080::parity(uint8_t val) {
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
bool Intel8080::carry(int bit_no, uint8_t a, uint8_t b, int16_t result16) {

  int16_t carry_bits = result16 ^ a ^ b;

  return carry_bits & (1 << bit_no);
}

// ========================================
// adds a value (+ an optional carry flag) to a register
// ----------------------------------------
void Intel8080::add(uint8_t* const reg, uint8_t val, bool cy) {
  uint8_t result8 = *reg + val + cy;
  int16_t result16 = *reg + val + cy;
  f_c_ = Intel8080::carry(8, *reg, val, result16);
  f_h_ = Intel8080::carry(4, *reg, val, result16);
  set_zsp_flags(result8);

  *reg = result8;
}

// ========================================
// subtracts a byte (+ an optional carry flag) from a register
// see https://stackoverflow.com/a/8037485
// ----------------------------------------
void Intel8080::sub(uint8_t* const reg, uint8_t val, bool cy) {
  // call add() which will set flags
  add(reg, ~val, !cy);
  f_c_ = !f_c_;
}

// ========================================
// adds a word to HL
// ----------------------------------------
void Intel8080::op_dad(uint16_t val) {
  f_c_ = ((rp_hl() + val) >> 16) & 1;
  set_hl(rp_hl() + val);
}

// ========================================
// increments a byte
// ----------------------------------------
uint8_t Intel8080::inr(uint8_t val) {
  uint8_t result = val + 1;
  f_h_ = (result & 0xF) == 0;
  set_zsp_flags(result);

  return result;
}

// ========================================
// decrements a byte
// ----------------------------------------
uint8_t Intel8080::dcr(uint8_t val) {
  uint8_t result = val - 1;
  f_h_ = !((result & 0xF) == 0xF);
  set_zsp_flags(result);

  return result;
}

// ========================================
// executes a logic "and" between register A and a byte, then stores the
// result in register A
// ----------------------------------------
void Intel8080::op_ana(uint8_t val) {
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
void Intel8080::op_xra(uint8_t val) {
  r_a_ ^= val;
  f_c_ = 0;
  f_h_ = 0;

  set_zsp_flags(r_a_);
}

// ========================================
// executes a logic "or" between register A and a byte, then stores the
// result in register A
// ----------------------------------------
void Intel8080::op_ora(uint8_t val) {
  r_a_ |= val;
  f_c_ = 0;
  f_h_ = 0;

  set_zsp_flags(r_a_);
}

// ========================================
// compares the register A to another byte
// ----------------------------------------
void Intel8080::op_cmp(uint8_t val) {
  int16_t result = r_a_ - val;
  f_c_ = result >> 8;
  f_h_ = ~(r_a_ ^ result ^ val) & 0x10;

  set_zsp_flags(result & 0xFF);
}

// ========================================
// sets the program counter to a given address
// ----------------------------------------
void Intel8080::jump(uint16_t addr) {
  set_pc(addr);
}

// ========================================
// jumps to next address pointed by the next word in memory if a condition
// is met
// ----------------------------------------
void Intel8080::cond_jump(bool condition) {
  uint16_t addr = pc_next_word();

  if (condition) {
    set_pc(addr);
  }
}

// ========================================
// pushes the current pc to the stack, then jumps to an address
// ----------------------------------------
void Intel8080::call(uint16_t addr) {
  push_stack(rp_pc());
  jump(addr);
}

// ========================================
// calls to next word in memory if a condition is met
// ----------------------------------------
void Intel8080::cond_call(bool condition) {
  uint16_t addr = pc_next_word();

  if (condition) {
    call(addr);
    cyc_ += 6;
  }
}

// ========================================
// returns from subroutine
// ----------------------------------------
void Intel8080::op_ret() {
  set_pc(pop_stack());
}

// ========================================
// returns from subroutine if a condition is met
// ----------------------------------------
void Intel8080::cond_ret(bool condition) {
  if (condition) {
    op_ret();
    cyc_ += 6;
  }
}

// ========================================
// pushes register A and the flags into the stack
// ----------------------------------------
void Intel8080::op_push_psw() {
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
void Intel8080::op_pop_psw() {
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
void Intel8080::op_rlc() {
  f_c_ = r_a_ >> 7;
  r_a_ = (r_a_ << 1) | f_c_;
}

// ========================================
// rotate register A right
// ----------------------------------------
void Intel8080::op_rrc() {
  f_c_ = r_a_ & 1;
  r_a_ = (r_a_ >> 1) | (f_c_ << 7);
}

// ========================================
// rotate register A left with the carry flag
// ----------------------------------------
void Intel8080::op_ral() {
  bool cy = f_c_;
  f_c_ = r_a_ >> 7;
  r_a_ = (r_a_ << 1) | cy;
}

// ========================================
// rotate register A right with the carry flag
// ----------------------------------------
void Intel8080::op_rar() {
  bool cy = f_c_;
  f_c_ = r_a_ & 1;
  r_a_ = (r_a_ >> 1) | (cy << 7);
}

// ========================================
// Decimal Adjust Accumulator: the eight-bit number in register A is adjusted
// to form two four-bit binary-coded-decimal digits.
// For example, if A=$2B and DAA is executed, A becomes $31.
// ----------------------------------------
void Intel8080::op_daa() {
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
void Intel8080::op_xchg() {
  uint16_t val = rp_de();
  set_de(rp_hl());
  set_hl(val);
}

// ========================================
// switches the value of a word at (sp) and HL
// ----------------------------------------
void Intel8080::op_xthl() {
  uint16_t val = rw(rp_sp());
  ww(rp_sp(), rp_hl());
  set_hl(val);
}

// ========================================
// executes one opcode
// ----------------------------------------
void Intel8080::execute(uint8_t opcode) {
  cyc_ += OPCODES_CYCLES[opcode];

  // when DI is executed, interrupts won't be serviced
  // until the end of next instruction:
  if (interrupt_delay_ > 0) {
    interrupt_delay_ -= 1;
  }

  switch (opcode) {
  case 0x7F: r_a_ = r_a_; break; // MOV A,A
  case 0x78: r_a_ = r_b_; break; // MOV A,B
  case 0x79: r_a_ = r_c_; break; // MOV A,C
  case 0x7A: r_a_ = r_d_; break; // MOV A,D
  case 0x7B: r_a_ = r_e_; break; // MOV A,E
  case 0x7C: r_a_ = r_h_; break; // MOV A,H
  case 0x7D: r_a_ = r_l_; break; // MOV A,L
  case 0x7E: r_a_ = rb(rp_hl()); break; // MOV A,M

  case 0x0A: r_a_ = rb(rp_bc()); break; // LDAX B
  case 0x1A: r_a_ = rb(rp_de()); break; // LDAX D
  case 0x3A: r_a_ = rb(pc_next_word()); break; // LDA word

  case 0x47: r_b_ = r_a_; break; // MOV B,A
  case 0x40: r_b_ = r_b_; break; // MOV B,B
  case 0x41: r_b_ = r_c_; break; // MOV B,C
  case 0x42: r_b_ = r_d_; break; // MOV B,D
  case 0x43: r_b_ = r_e_; break; // MOV B,E
  case 0x44: r_b_ = r_h_; break; // MOV B,H
  case 0x45: r_b_ = r_l_; break; // MOV B,L
  case 0x46: r_b_ = rb(rp_hl()); break; // MOV B,M

  case 0x4F: r_c_ = r_a_; break; // MOV C,A
  case 0x48: r_c_ = r_b_; break; // MOV C,B
  case 0x49: r_c_ = r_c_; break; // MOV C,C
  case 0x4A: r_c_ = r_d_; break; // MOV C,D
  case 0x4B: r_c_ = r_e_; break; // MOV C,E
  case 0x4C: r_c_ = r_h_; break; // MOV C,H
  case 0x4D: r_c_ = r_l_; break; // MOV C,L
  case 0x4E: r_c_ = rb(rp_hl()); break; // MOV C,M

  case 0x57: r_d_ = r_a_; break; // MOV D,A
  case 0x50: r_d_ = r_b_; break; // MOV D,B
  case 0x51: r_d_ = r_c_; break; // MOV D,C
  case 0x52: r_d_ = r_d_; break; // MOV D,D
  case 0x53: r_d_ = r_e_; break; // MOV D,E
  case 0x54: r_d_ = r_h_; break; // MOV D,H
  case 0x55: r_d_ = r_l_; break; // MOV D,L
  case 0x56: r_d_ = rb(rp_hl()); break; // MOV D,M

  case 0x5F: r_e_ = r_a_; break; // MOV E,A
  case 0x58: r_e_ = r_b_; break; // MOV E,B
  case 0x59: r_e_ = r_c_; break; // MOV E,C
  case 0x5A: r_e_ = r_d_; break; // MOV E,D
  case 0x5B: r_e_ = r_e_; break; // MOV E,E
  case 0x5C: r_e_ = r_h_; break; // MOV E,H
  case 0x5D: r_e_ = r_l_; break; // MOV E,L
  case 0x5E: r_e_ = rb(rp_hl()); break; // MOV E,M

  case 0x67: r_h_ = r_a_; break; // MOV H,A
  case 0x60: r_h_ = r_b_; break; // MOV H,B
  case 0x61: r_h_ = r_c_; break; // MOV H,C
  case 0x62: r_h_ = r_d_; break; // MOV H,D
  case 0x63: r_h_ = r_e_; break; // MOV H,E
  case 0x64: r_h_ = r_h_; break; // MOV H,H
  case 0x65: r_h_ = r_l_; break; // MOV H,L
  case 0x66: r_h_ = rb(rp_hl()); break; // MOV H,M

  case 0x6F: r_l_ = r_a_; break; // MOV L,A
  case 0x68: r_l_ = r_b_; break; // MOV L,B
  case 0x69: r_l_ = r_c_; break; // MOV L,C
  case 0x6A: r_l_ = r_d_; break; // MOV L,D
  case 0x6B: r_l_ = r_e_; break; // MOV L,E
  case 0x6C: r_l_ = r_h_; break; // MOV L,H
  case 0x6D: r_l_ = r_l_; break; // MOV L,L
  case 0x6E: r_l_ = rb(rp_hl()); break; // MOV L,M

  case 0x77: wb(rp_hl(), r_a_); break; // MOV M,A
  case 0x70: wb(rp_hl(), r_b_); break; // MOV M,B
  case 0x71: wb(rp_hl(), r_c_); break; // MOV M,C
  case 0x72: wb(rp_hl(), r_d_); break; // MOV M,D
  case 0x73: wb(rp_hl(), r_e_); break; // MOV M,E
  case 0x74: wb(rp_hl(), r_h_); break; // MOV M,H
  case 0x75: wb(rp_hl(), r_l_); break; // MOV M,L

  case 0x3E: r_a_ = pc_next_byte(); break; // MVI A,byte
  case 0x06: r_b_ = pc_next_byte(); break; // MVI B,byte
  case 0x0E: r_c_ = pc_next_byte(); break; // MVI C,byte
  case 0x16: r_d_ = pc_next_byte(); break; // MVI D,byte
  case 0x1E: r_e_ = pc_next_byte(); break; // MVI E,byte
  case 0x26: r_h_ = pc_next_byte(); break; // MVI H,byte
  case 0x2E: r_l_ = pc_next_byte(); break; // MVI L,byte
  case 0x36: wb(rp_hl(), pc_next_byte()); break; // MVI M,byte

  case 0x02: wb(rp_bc(), r_a_);        break; // STAX B
  case 0x12: wb(rp_de(), r_a_);        break; // STAX D
  case 0x32: wb(pc_next_word(), r_a_); break; // STA word

  case 0x01: set_bc(pc_next_word()); break; // LXI B,word
  case 0x11: set_de(pc_next_word()); break; // LXI D,word
  case 0x21: set_hl(pc_next_word()); break; // LXI H,word
  case 0x31: set_sp(pc_next_word()); break; // LXI SP,word
  case 0x2A: set_hl(rw(pc_next_word()));  break; // LHLD
  case 0x22: ww(pc_next_word(), rp_hl()); break; // SHLD
  case 0xF9: set_sp(rp_hl());             break; // SPHL

  case 0xEB: op_xchg(); break; // XCHG
  case 0xE3: op_xthl(); break; // XTHL

  case 0x87: add(&r_a_, r_a_, 0); break; // ADD A
  case 0x80: add(&r_a_, r_b_, 0); break; // ADD B
  case 0x81: add(&r_a_, r_c_, 0); break; // ADD C
  case 0x82: add(&r_a_, r_d_, 0); break; // ADD D
  case 0x83: add(&r_a_, r_e_, 0); break; // ADD E
  case 0x84: add(&r_a_, r_h_, 0); break; // ADD H
  case 0x85: add(&r_a_, r_l_, 0); break; // ADD L
  case 0x86: add(&r_a_, rb(rp_hl()), 0); break; // ADD M

  case 0xC6: add(&r_a_, pc_next_byte(), 0); break; // ADI byte

  case 0x8F: add(&r_a_, r_a_, f_c_); break; // ADC A
  case 0x88: add(&r_a_, r_b_, f_c_); break; // ADC B
  case 0x89: add(&r_a_, r_c_, f_c_); break; // ADC C
  case 0x8A: add(&r_a_, r_d_, f_c_); break; // ADC D
  case 0x8B: add(&r_a_, r_e_, f_c_); break; // ADC E
  case 0x8C: add(&r_a_, r_h_, f_c_); break; // ADC H
  case 0x8D: add(&r_a_, r_l_, f_c_); break; // ADC L
  case 0x8E: add(&r_a_, rb(rp_hl()), f_c_); break; // ADC M

  case 0xCE: add(&r_a_, pc_next_byte(), f_c_); break; // ACI byte

  case 0x97: sub(&r_a_, r_a_, 0); break; // SUB A
  case 0x90: sub(&r_a_, r_b_, 0); break; // SUB B
  case 0x91: sub(&r_a_, r_c_, 0); break; // SUB C
  case 0x92: sub(&r_a_, r_d_, 0); break; // SUB D
  case 0x93: sub(&r_a_, r_e_, 0); break; // SUB E
  case 0x94: sub(&r_a_, r_h_, 0); break; // SUB H
  case 0x95: sub(&r_a_, r_l_, 0); break; // SUB L
  case 0x96: sub(&r_a_, rb(rp_hl()), 0); break; // SUB M

  case 0xD6: sub(&r_a_, pc_next_byte(), 0); break; // SUI byte

  case 0x9F: sub(&r_a_, r_a_, f_c_); break; // SBB A
  case 0x98: sub(&r_a_, r_b_, f_c_); break; // SBB B
  case 0x99: sub(&r_a_, r_c_, f_c_); break; // SBB C
  case 0x9A: sub(&r_a_, r_d_, f_c_); break; // SBB D
  case 0x9B: sub(&r_a_, r_e_, f_c_); break; // SBB E
  case 0x9C: sub(&r_a_, r_h_, f_c_); break; // SBB H
  case 0x9D: sub(&r_a_, r_l_, f_c_); break; // SBB L
  case 0x9E: sub(&r_a_, rb(rp_hl()), f_c_); break; // SBB M

  case 0xDE: sub(&r_a_, pc_next_byte(), f_c_); break; // SBI byte

  case 0x09: op_dad(rp_bc()); break; // DAD B
  case 0x19: op_dad(rp_de()); break; // DAD D
  case 0x29: op_dad(rp_hl()); break; // DAD H
  case 0x39: op_dad(rp_sp()); break; // DAD SP

  case 0xF3: f_i_ = 0; break; // DI
  case 0xFB:
    f_i_ = 1;
    interrupt_delay_ = 1;
    break; // EI

  case 0x00: break; // NOP
  case 0x76: halted_ = 1; break; // HLT

  case 0x3C: r_a_ = inr(r_a_); break; // INR A
  case 0x04: r_b_ = inr(r_b_); break; // INR B
  case 0x0C: r_c_ = inr(r_c_); break; // INR C
  case 0x14: r_d_ = inr(r_d_); break; // INR D
  case 0x1C: r_e_ = inr(r_e_); break; // INR E
  case 0x24: r_h_ = inr(r_h_); break; // INR H
  case 0x2C: r_l_ = inr(r_l_); break; // INR L
  case 0x34: wb(rp_hl(), inr(rb(rp_hl()))); break; // INR M

  case 0x3D: r_a_ = dcr(r_a_); break; // DCR A
  case 0x05: r_b_ = dcr(r_b_); break; // DCR B
  case 0x0D: r_c_ = dcr(r_c_); break; // DCR C
  case 0x15: r_d_ = dcr(r_d_); break; // DCR D
  case 0x1D: r_e_ = dcr(r_e_); break; // DCR E
  case 0x25: r_h_ = dcr(r_h_); break; // DCR H
  case 0x2D: r_l_ = dcr(r_l_); break; // DCR L
  case 0x35: wb(rp_hl(), dcr(rb(rp_hl()))); break; // DCR M

  case 0x03: set_bc(rp_bc() + 1); break; // INX B
  case 0x13: set_de(rp_de() + 1); break; // INX D
  case 0x23: set_hl(rp_hl() + 1); break; // INX H
  case 0x33: set_sp(rp_sp() + 1); break; // INX SP

  case 0x0B: set_bc(rp_bc() - 1); break; // DCX B
  case 0x1B: set_de(rp_de() - 1); break; // DCX D
  case 0x2B: set_hl(rp_hl() - 1); break; // DCX H
  case 0x3B: set_sp(rp_sp() - 1); break; // DCX SP

  case 0x27: op_daa();       break; // DAA
  case 0x2F: r_a_ = ~r_a_;   break; // CMA
  case 0x37: f_c_ = 1;       break; // STC
  case 0x3F: f_c_ = !f_c_;   break; // CMC

  case 0x07: op_rlc(); break; // RLC (rotate left)
  case 0x0F: op_rrc(); break; // RRC (rotate right)
  case 0x17: op_ral(); break; // RAL
  case 0x1F: op_rar(); break; // RAR

  case 0xA7: op_ana(r_a_); break; // ANA A
  case 0xA0: op_ana(r_b_); break; // ANA B
  case 0xA1: op_ana(r_c_); break; // ANA C
  case 0xA2: op_ana(r_d_); break; // ANA D
  case 0xA3: op_ana(r_e_); break; // ANA E
  case 0xA4: op_ana(r_h_); break; // ANA H
  case 0xA5: op_ana(r_l_); break; // ANA L
  case 0xA6: op_ana(rb(rp_hl())); break; // ANA M

  case 0xE6: op_ana(pc_next_byte()); break; // ANI byte

  case 0xAF: op_xra(r_a_); break; // XRA A
  case 0xA8: op_xra(r_b_); break; // XRA B
  case 0xA9: op_xra(r_c_); break; // XRA C
  case 0xAA: op_xra(r_d_); break; // XRA D
  case 0xAB: op_xra(r_e_); break; // XRA E
  case 0xAC: op_xra(r_h_); break; // XRA H
  case 0xAD: op_xra(r_l_); break; // XRA L
  case 0xAE: op_xra(rb(rp_hl())); break; // XRA M

  case 0xEE: op_xra(pc_next_byte()); break; // XRI byte

  case 0xB7: op_ora(r_a_); break; // ORA A
  case 0xB0: op_ora(r_b_); break; // ORA B
  case 0xB1: op_ora(r_c_); break; // ORA C
  case 0xB2: op_ora(r_d_); break; // ORA D
  case 0xB3: op_ora(r_e_); break; // ORA E
  case 0xB4: op_ora(r_h_); break; // ORA H
  case 0xB5: op_ora(r_l_); break; // ORA L
  case 0xB6: op_ora(rb(rp_hl())); break; // ORA M

  case 0xF6: op_ora(pc_next_byte()); break; // ORI byte

  case 0xBF: op_cmp(r_a_); break; // CMP A
  case 0xB8: op_cmp(r_b_); break; // CMP B
  case 0xB9: op_cmp(r_c_); break; // CMP C
  case 0xBA: op_cmp(r_d_); break; // CMP D
  case 0xBB: op_cmp(r_e_); break; // CMP E
  case 0xBC: op_cmp(r_h_); break; // CMP H
  case 0xBD: op_cmp(r_l_); break; // CMP L
  case 0xBE: op_cmp(rb(rp_hl())); break; // CMP M
  case 0xFE: op_cmp(pc_next_byte()); break; // CPI byte

  case 0xC3: jump(pc_next_word());  break; // JMP
  case 0xC2: cond_jump(f_z_ == 0);    break; // JNZ
  case 0xCA: cond_jump(f_z_ == 1);    break; // JZ
  case 0xD2: cond_jump(f_c_ == 0);    break; // JNC
  case 0xDA: cond_jump(f_c_ == 1);    break; // JC
  case 0xE2: cond_jump(f_p_ == 0);    break; // JPO
  case 0xEA: cond_jump(f_p_ == 1);    break; // JPE
  case 0xF2: cond_jump(f_s_ == 0);    break; // JP
  case 0xFA: cond_jump(f_s_ == 1);    break; // JM

  case 0xE9: set_pc(rp_hl());         break; // PCHL
  case 0xCD: call(pc_next_word()); break; // CALL

  case 0xC4: cond_call(f_z_ == 0); break; // CNZ
  case 0xCC: cond_call(f_z_ == 1); break; // CZ
  case 0xD4: cond_call(f_c_ == 0); break; // CNC
  case 0xDC: cond_call(f_c_ == 1); break; // CC
  case 0xE4: cond_call(f_p_ == 0); break; // CPO
  case 0xEC: cond_call(f_p_ == 1); break; // CPE
  case 0xF4: cond_call(f_s_ == 0); break; // CP
  case 0xFC: cond_call(f_s_ == 1); break; // CM

  case 0xC9: op_ret();            break; // RET
  case 0xC0: cond_ret(f_z_ == 0); break; // RNZ
  case 0xC8: cond_ret(f_z_ == 1); break; // RZ
  case 0xD0: cond_ret(f_c_ == 0); break; // RNC
  case 0xD8: cond_ret(f_c_ == 1); break; // RC
  case 0xE0: cond_ret(f_p_ == 0); break; // RPO
  case 0xE8: cond_ret(f_p_ == 1); break; // RPE
  case 0xF0: cond_ret(f_s_ == 0); break; // RP
  case 0xF8: cond_ret(f_s_ == 1); break; // RM

  case 0xC7: call(0x00); break; // RST 0
  case 0xCF: call(0x08); break; // RST 1
  case 0xD7: call(0x10); break; // RST 2
  case 0xDF: call(0x18); break; // RST 3
  case 0xE7: call(0x20); break; // RST 4
  case 0xEF: call(0x28); break; // RST 5
  case 0xF7: call(0x30); break; // RST 6
  case 0xFF: call(0x38); break; // RST 7

  case 0xC5: push_stack(rp_bc()); break; // PUSH B
  case 0xD5: push_stack(rp_de()); break; // PUSH D
  case 0xE5: push_stack(rp_hl()); break; // PUSH H
  case 0xF5: op_push_psw();    break; // PUSH PSW
  case 0xC1: set_bc(pop_stack());  break; // POP B
  case 0xD1: set_de(pop_stack());  break; // POP D
  case 0xE1: set_hl(pop_stack());  break; // POP H
  case 0xF1: op_pop_psw();          break; // POP PSW

  case 0xDB: // IN
    r_a_ = supervisor_request_port_in_(userdata_, pc_next_byte());
    break;
  case 0xD3: // OUT
    supervisor_request_port_out_(userdata_, pc_next_byte(), r_a_);
    break;

  // undocumented NOPs
  case 0x08:
  case 0x10:
  case 0x18:
  case 0x20:
  case 0x28:
  case 0x30:
  case 0x38:
    break;

  // undocumented RET
  case 0xD9:
    op_ret();
    break;

  // undocumented CALLs
  case 0xDD:
  case 0xED:
  case 0xFD:
    call(pc_next_word());
    break;

  // undocumented JMP
  case 0xCB:
    jump(pc_next_word());
    break;
  }
}

// ========================================
// executes one instruction
// ----------------------------------------
void Intel8080::exec_step() {
  // interrupt processing: if an interrupt is pending and IFF is set,
  // we execute the interrupt vector passed by the user.
  if (interrupt_pending_ && f_i_ && interrupt_delay_ == 0) {
    interrupt_pending_ = 0;
    f_i_ = 0;
    halted_ = 0;

    execute(interrupt_vector_);
  } else if (!halted_) {
    execute(pc_next_byte());
  }
}

// ========================================
// asks for an interrupt to be serviced
// ----------------------------------------
void Intel8080::exec_interrupt(uint8_t opcode) {
  interrupt_pending_ = 1;
  interrupt_vector_ = opcode;
}

// ========================================
// outputs a debug trace of the emulator state to the standard output,
// including registers and flags
// ----------------------------------------
void Intel8080::debug_output(bool print_disassembly) {
  uint8_t f = 0;
  f |= f_s_ << 7;
  f |= f_z_ << 6;
  f |= f_h_ << 4;
  f |= f_p_ << 2;
  f |= 1    << 1; // bit 1 is always 1
  f |= f_c_ << 0;

  printf("PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, CYC: %lu",
	 rp_pc(), r_a_ << 8 | f, rp_bc(), rp_de(), rp_hl(), rp_sp(), cyc_);

  uint16_t my_pc = rp_pc();

  printf("\t(%02X %02X %02X %02X)", rb(my_pc), rb(my_pc + 1), rb(my_pc + 2), rb(my_pc + 3));

  if (print_disassembly) {
    printf(" - %s", DISASSEMBLE_TABLE[rb(my_pc)]);
  }

  printf("\n");
}
