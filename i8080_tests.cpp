// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Intel8080.h"

// memory callbacks
#define MEMORY_SIZE 0x10000
static uint8_t* memory__ = NULL;
static bool test_finished__ = 0;

static uint8_t rb(void* userdata, uint16_t addr) {
  return memory__[addr];
}

static void wb(void* userdata, uint16_t addr, uint8_t val) {
  memory__[addr] = val;
}

static uint8_t port_in(void* userdata, uint8_t port) {
  return 0x00;
}

static void port_out(void* userdata, uint8_t port, uint8_t value) {
  Intel8080* const c = (Intel8080*) userdata;

  if (port == 0) {
    test_finished__ = 1;
  } else if (port == 1) {
    uint8_t operation = c->r_c();

    if (operation == 2) { // print a character stored in E
      printf("%c", c->r_e());
    } else if (operation == 9) { // print from memory at (DE) until '$' char
      uint16_t addr = c->rp_de();

      do {
        printf("%c", rb(c, addr++));
      } while (rb(c, addr) != '$');
    }
  }
}

static inline int load_file(const char* filename, uint16_t addr) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "error: can't open file '%s'.\n", filename);

    return 1;
  }

  // file size check:
  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  if (file_size + addr >= MEMORY_SIZE) {
    fprintf(stderr, "error: file %s can't fit in memory.\n", filename);

    return 1;
  }

  // copying the bytes in memory:
  size_t result = fread(&memory__[addr], sizeof(uint8_t), file_size, f);
  if (result != file_size) {
    fprintf(stderr, "error: while reading file '%s'\n", filename);

    return 1;
  }

  fclose(f);

  return 0;
}

static inline void run_test(
    Intel8080* const c, const char* filename, unsigned long cyc_expected) {
  c->init();
  c->userdata_ = c;
  c->read_byte = rb;
  c->write_byte = wb;
  c->supervisor_request_port_in = port_in;
  c->supervisor_request_port_out = port_out;
  memset(memory__, 0, MEMORY_SIZE);

  if (load_file(filename, 0x100) != 0) {
    return;
  }
  printf("*** TEST: %s\n", filename);

  c->set_pc(0x100);

  // inject "out 0,a" at 0x0000 (signal to stop the test)
  memory__[0x0000] = 0xD3;  // OUT
  memory__[0x0001] = 0x00;

  // inject "out 1,a" at 0x0005 (signal to output some characters)
  memory__[0x0005] = 0xD3;  // OUT
  memory__[0x0006] = 0x01;
  memory__[0x0007] = 0xC9;  // RET

  long nb_instructions = 0;

  test_finished__ = 0;

  while (!test_finished__) {
    nb_instructions += 1;

    // uncomment following line to have a debug output of machine state
    // warning: will output multiple GB of data for the whole test suite
    // c->debug_output(false);

    c->exec_step();
  }

  long long diff = cyc_expected - c->cyc_;
  printf("\n*** %lu instructions executed on %lu cycles"
         " (expected=%lu, diff=%lld)\n\n",
      nb_instructions, c->cyc_, cyc_expected, diff);
}

int main(void) {
  memory__ = (uint8_t*)malloc(MEMORY_SIZE);
  if (memory__ == NULL) {
    puts("cannot allocate memory");

    return 1;
  }

  Intel8080 cpu;
  run_test(&cpu, "cpu_tests/TST8080.COM", 4924LU);
  run_test(&cpu, "cpu_tests/CPUTEST.COM", 255653383LU);
  run_test(&cpu, "cpu_tests/8080PRE.COM", 7817LU);
  run_test(&cpu, "cpu_tests/8080EXM.COM", 23803381171LU);

  free(memory__);

  return 0;
}
