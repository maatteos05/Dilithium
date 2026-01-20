#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "KeyGen_internal.c"

int KeyGen(uint8_t sk, uint8_t pk) {
  /* Generate a pair of public and private keys */
  uint8_t seed[32] = {
      0}; // need to find an algorithm to generate a valid random seed
  bool check = true;

  for (int i = 0; i < 32; i++) {
    if (seed[i] != 0) {
      check = false;
    }
  }
  if (!check) {
    return -1;
  }
  return KeyGen_internal(seed, &sk, &pk);
}
