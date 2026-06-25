// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// project
#include "Buss.h"
#include "Intel8080.h"
#include "Memory.h"

static bool test_finished__ = 0;
static bool running__ = false;

// ========================================
//
// ----------------------------------------
static void set_halted() {
  running__ = false;
  test_finished__ = 1;
}

// ========================================
//
// ----------------------------------------
static uint8_t port_in(uint8_t port) {
  return 0x00;
}

// ========================================
//
// ----------------------------------------
static void port_out(uint8_t port, uint8_t value, const Intel8080* cpu, const I_Buss* buss) {
  if (port == 1)
  {
    uint8_t operation = cpu->r_c();

    if (operation == 2)
    {
      // print a character stored in E
      printf("%c", cpu->r_e());
    }
    else if (operation == 9)
    {
      // print from memory at (DE) until '$' char
      uint16_t addr = cpu->rp_de();

      do
      {
        printf("%c", buss->mem_read(addr++));
      } while (buss->mem_read(addr) != '$');
    }
  }
}

// ========================================
//
// ----------------------------------------
static inline int load_file(const char* filename, uint16_t addr, I_Buss* buss)
{
  puts("Loading file...");

  FILE* f = fopen(filename, "rb");

  if (f == NULL) {
    fprintf(stderr, "error: can't open file '%s'.\n", filename);

    return 1;
  }

  // file size
  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  uint8_t* buffer = (uint8_t*)malloc(file_size);

  if (buffer == NULL)
  {
    puts("Cannot allocate buffer");

    fclose(f);

    return 1;
  }

  size_t num_bytes_read = fread(buffer, sizeof(uint8_t), file_size, f);

  fclose(f);

  if (num_bytes_read != file_size)
  {
    fprintf(stderr, "error: while reading file '%s'\n", filename);

    return 1;
  }

  buss->mem_write_block(addr, buffer, file_size);

  free(buffer);

  return 0;
}

// ========================================
//
// ----------------------------------------
static inline void run_test(
  Intel8080* const cpu, I_Buss* buss, const char* filename, unsigned long cyc_expected)
{
  puts("Running test...");

  cpu->init();

  if (load_file(filename, 0x100, buss) != 0)
  {
    return;
  }

  printf("*** TEST: %s\n", filename);

  cpu->set_pc(0x100);

  // inject "out 0,a" at 0x0000 (signal to stop the test)
  buss->mem_write(0x0000, 0x76);  // HLT
  buss->mem_write(0x0001, 0x00);

  // inject "out 1,a" at 0x0005 (signal to output some characters)
  buss->mem_write(0x0005, 0xD3);  // OUT
  buss->mem_write(0x0006, 0x01);
  buss->mem_write(0x0007, 0xC9);  // RET

  long num_instructions = 0;

  test_finished__ = 0;

  while (!test_finished__) {
    num_instructions += 1;

    // uncomment following line to have a debug output of machine state
    // warning: will output multiple GB of data for the whole test suite
    // cpu->debug_output(false);

    cpu->exec_step();
  }

  unsigned long cyc_actual = cpu->num_cycles();
  long long diff = cyc_expected - cyc_actual;
  printf("\n*** %lu instructions executed on %lu cycles"
         " (expected=%lu, diff=%lld)\n\n",
      num_instructions, cyc_actual, cyc_expected, diff);
}

// ========================================
//
// ----------------------------------------
int main(void) {
  puts("i8080_tests");

  puts("Creating Memory...");
  Memory memory;

  puts("Creating Buss...");
  Buss buss;
  buss.add(&memory);

  puts("Creating Intel8080...");
  Intel8080 cpu(&buss, port_in, port_out, set_halted);

  puts("Running tests...");
  run_test(&cpu, &buss, "cpu_tests/TST8080.COM", 4921LU);
  run_test(&cpu, &buss, "cpu_tests/CPUTEST.COM", 255653380LU);
  run_test(&cpu, &buss, "cpu_tests/8080PRE.COM", 7814LU);
  run_test(&cpu, &buss, "cpu_tests/8080EXM.COM", 23803381168LU);

  return 0;
}
