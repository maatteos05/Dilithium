#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define K_MAX 4
#define L_MAX 4

// Function Prototypes (from KeyGen_internal.c)
void H(uint8_t seed[32]);

void ExpandA(uint8_t seed[32], int32_t A[K_MAX][L_MAX][256]);
void ExpandS(uint8_t seed[64], int32_t s1[L_MAX][256], int32_t s2[K_MAX][256]);

int main() {
  printf("Starting KeyGen Test...\n");

  // 1. Test H
  uint8_t seed[32] = {0};
  printf("Testing H...\n");
  H(seed);

  // 2. Test ExpandA
  printf("Testing ExpandA...\n");
  int32_t A[K_MAX][L_MAX][256];
  uint8_t rho[32];
  memset(rho, 0x42, 32);

  int32_t A_actual[4][4][256];
  ExpandA(rho, A_actual);

  printf("ExpandA returned. Checking first element: %d\n", A_actual[0][0][0]);

  // 3. Test ExpandS
  printf("Testing ExpandS...\n");
  int32_t s1[4][256];
  int32_t s2[4][256];
  uint8_t rho_p[64]; // ExpandS takes 64 bytes
  memset(rho_p, 0x11, 64);

  ExpandS(rho_p, s1, s2);

  printf("ExpandS returned.\n");
  printf("s1[0][0]: %d\n", s1[0][0]);
  printf("s2[0][0]: %d\n", s2[0][0]);

  printf("Test Complete.\n");
  return 0;
}
