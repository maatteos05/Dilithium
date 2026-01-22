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

  // printf("The seed is: ");
  // for (int i = 0; i < 32; i++) { // Print first 64 bytes
  //   printf("%02x", seed[i]);
  // }
  // printf("\n");

  for (int i = 0; i < 32; i++) {
    if (seed[i] != 0) {
      check = false;
    }
  }
  if (check) {
    return -1;
  }
  return KeyGen_internal(seed, sk, pk);
}

int main() {
  // sk_len = 128 + 32 * ((k+l)*3 + d*k) = 128 + 32*76 = 2560 bytes
  // pk_size = 32 * k * 10 = 1280 bytes
  uint8_t sk[2560], pk[1280];
  KeyGen(sk, pk);
  // Print first N bytes of secret key
  printf("The secret key is: \n");
  for (int i = 0; i < 2560; i++) { // Print first 64 bytes
    printf("%02x", sk[i]);
  }
  printf("\n");

  // Print first N bytes of public key
  printf("The public key is: \n");
  for (int i = 0; i < 1280; i++) {
    printf("%02x", pk[i]);
  }
  printf("\n");

  return 0;
}