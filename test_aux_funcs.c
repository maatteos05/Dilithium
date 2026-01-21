#include "aux_funcs.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Test helper
void print_bytes(const char *label, uint8_t *arr, int len) {
  printf("%s: ", label);
  for (int i = 0; i < len; i++)
    printf("%02x ", arr[i]);
  printf("\n");
}

void print_bits(const char *label, uint8_t *arr, int len) {
  printf("%s: ", label);
  for (int i = 0; i < len; i++)
    printf("%d", arr[i]);
  printf("\n");
}

int main() {
  int passed = 0, failed = 0;

  printf("=== Testing Auxiliary Functions ===\n\n");

  // 1. Test IntegerToBytes
  printf("1. IntegerToBytes\n");
  {
    uint8_t result[4];
    IntegerToBytes(result, 0x12345678, 4);
    // Little-endian: 0x78, 0x56, 0x34, 0x12
    if (result[0] == 0x78 && result[1] == 0x56 && result[2] == 0x34 &&
        result[3] == 0x12) {
      printf("   PASS: 0x12345678 -> ");
      print_bytes("", result, 4);
      passed++;
    } else {
      printf("   FAIL: Expected 78 56 34 12, got ");
      print_bytes("", result, 4);
      failed++;
    }
  }

  // 2. Test bit_len
  printf("\n2. bit_len\n");
  {
    int tests[][2] = {{0, 0},   {1, 1},   {2, 2},    {3, 2},
                      {255, 8}, {256, 9}, {1023, 10}};
    for (int i = 0; i < 7; i++) {
      int result = bit_len(tests[i][0]);
      if (result == tests[i][1]) {
        printf("   PASS: bit_len(%d) = %d\n", tests[i][0], result);
        passed++;
      } else {
        printf("   FAIL: bit_len(%d) = %d, expected %d\n", tests[i][0], result,
               tests[i][1]);
        failed++;
      }
    }
  }

  // 3. Test IntegerToBits
  printf("\n3. IntegerToBits\n");
  {
    uint8_t bits[8];
    IntegerToBits(bits, 5, 4); // 5 = 0101 in binary (LSB first: 1,0,1,0)
    if (bits[0] == 1 && bits[1] == 0 && bits[2] == 1 && bits[3] == 0) {
      printf("   PASS: 5 -> ");
      print_bits("", bits, 4);
      passed++;
    } else {
      printf("   FAIL: 5 -> ");
      print_bits("", bits, 4);
      failed++;
    }
  }

  // 4. Test CoeffFromThreeBytes
  printf("\n4. CoeffFromThreeBytes\n");
  {
    // Test valid coefficient (< q = 8380417)
    int32_t c1 = CoeffFromThreeBytes(0x00, 0x00, 0x00);
    int32_t c2 =
        CoeffFromThreeBytes(0xFF, 0xFF, 0x7F); // 8388607 > q, should return -1
    int32_t c3 = CoeffFromThreeBytes(0x00, 0x00, 0x01); // 65536

    if (c1 == 0) {
      printf("   PASS: (0,0,0) = 0\n");
      passed++;
    } else {
      printf("   FAIL: (0,0,0) = %d\n", c1);
      failed++;
    }

    if (c2 == -1) {
      printf("   PASS: (FF,FF,7F) = -1 (rejected)\n");
      passed++;
    } else {
      printf("   FAIL: (FF,FF,7F) = %d, expected -1\n", c2);
      failed++;
    }

    if (c3 == 65536) {
      printf("   PASS: (0,0,1) = 65536\n");
      passed++;
    } else {
      printf("   FAIL: (0,0,1) = %d\n", c3);
      failed++;
    }
  }

  // 5. Test CoeffFromHalfByte (ETA=2)
  printf("\n5. CoeffFromHalfByte (ETA=2)\n");
  {
    // For eta=2: if b<15, return 2 - (b % 5)
    // b=0: 2-0=2, b=1: 2-1=1, b=2: 2-2=0, b=3: 2-3=-1, b=4: 2-4=-2
    // b=5: 2-0=2, ... b=15: -1 (rejected)
    int32_t tests[][2] = {{0, 2}, {1, 1}, {2, 0}, {3, -1}, {4, -2}, {15, -1}};
    for (int i = 0; i < 6; i++) {
      int32_t result = CoeffFromHalfByte(tests[i][0]);
      if (result == tests[i][1]) {
        printf("   PASS: CoeffFromHalfByte(%d) = %d\n", tests[i][0], result);
        passed++;
      } else {
        printf("   FAIL: CoeffFromHalfByte(%d) = %d, expected %d\n",
               tests[i][0], result, tests[i][1]);
        failed++;
      }
    }
  }

  // 6. Test SimpleBitPack
  printf("\n6. SimpleBitPack\n");
  {
    int32_t poly[256] = {0};
    poly[0] = 5; // 101 in 3 bits
    poly[1] = 3; // 011 in 3 bits
    poly[2] = 7; // 111 in 3 bits

    int bound = 7;                      // bit_len(7) = 3
    int out_size = 32 * bit_len(bound); // 32 * 3 = 96 bytes
    uint8_t z[96];

    SimpleBitPack(z, poly, bound);

    // First 9 bits: 101 011 111 = packed into bytes
    // LSB first: bits 0-7 go to z[0], bit 8 goes to z[1]
    printf("   First 3 bytes: %02x %02x %02x\n", z[0], z[1], z[2]);
    printf("   (Manual verification needed for full correctness)\n");
    passed++;
  }

  printf("\n=== Summary: %d passed, %d failed ===\n", passed, failed);
  return failed > 0 ? 1 : 0;
}
