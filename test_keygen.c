#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define K_MAX 4
#define L_MAX 4

#include "KeyGen_internal.h"

int main() {
  printf("Starting KeyGen Test...\n");

  // 1. Test H
  uint8_t seed[32] = {1};
  size_t out_len = 64;
  uint8_t *out = malloc(out_len);
  printf("Testing H...\n");
  H(seed, 32, out, out_len);
  printf("the output is: %u\n", *out);

  // 2. Test ExpandA
  printf("Testing ExpandA...\n");
  int32_t A[4][4][256];
  uint8_t rho[32];
  memset(rho, 0x42, 32);

  ExpandA(rho, A);

  printf("ExpandA returned. Checking first element: %d\n", A[0][0][0]);

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

  // 4. Test NTT and NTT_inv
  int32_t w[256];
  int32_t w2[256];
  printf("before NTT\n");

  for (int i = 0; i < 256; i++) {
    printf("%d, ", s1[0][i]);
  }

  printf("\n");
  NTT(s1[0], w);

  printf("after NTT:\n");

  for (int i = 0; i < 256; i++) {
    printf("%d, ", w[i]);
  }
  printf("\n");
  printf("Using NTT inverse to revert to original vector\n");

  NTT_inv(w, w2);
  for (int i = 0; i < 256; i++) {
    printf("%d, ", w2[i]);
  }
  printf("\n");

  // 5. Test Power2Round
  int32_t w3[2];
  Power2Round(w2[0], w3);
  printf("the decompositioin of %d is: %d 2^d + %d\n", w2[0], w3[1], w3[0]);

  return 0;
}
