#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "KeyGen_internal.c"
#include "randombytes.h"

int KeyGen(uint8_t *sk, uint8_t *pk) {
  /* Generate a pair of public and private keys */
  uint8_t seed[32]; // need to find an algorithm to generate a valid random seed
  randombytes(seed, 32);

  bool check = true;

  for (int i = 0; i < 32; i++) {
    if (seed[i] != 0) {
      check = false;
    }
  }
  if (!check) {
    return -1;
  }
  return KeyGen_internal(seed, sk, pk);
}

int main() {
  uint8_t sk[128], pk[256]; // Allocate appropriate sizes
  KeyGen(sk, pk);
  // Print first N bytes of secret key
  printf("The secret key is: ");
  for (int i = 0; i < 64; i++) { // Print first 64 bytes
    printf("%02x", sk[i]);
  }
  printf("\n");

  // Print first N bytes of public key
  printf("The public key is: ");
  for (int i = 0; i < 64; i++) {
    printf("%02x", pk[i]);
  }
  printf("\n");
  return 0;
}