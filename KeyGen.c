#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "KeyGen_internal.h"
#include "randombytes.h"

/* FIPS 204 ML-DSA-44 sizes (k=l=4, eta=2, d=13):
 *   pk_len = 32 + 32*k*(bitlen(q-1)-d) = 1312 bytes
 *   sk_len = 128 + 32*((k+l)*bitlen(2*eta) + d*k) = 2560 bytes
 */
#define MLDSA44_PK_LEN 1312
#define MLDSA44_SK_LEN 2560

int KeyGen(uint8_t *sk, uint8_t *pk) {
  uint8_t seed[32] = {0};
  // randombytes(seed, sizeof(seed));
  seed[0] = 0x1;
  seed[1] = 0xFF;
  /* IMPORTANT: KeyGen_internal signature is (seed, pk, sk). */
  return KeyGen_internal(seed, pk, sk);
}

int main(void) {
  uint8_t sk[MLDSA44_SK_LEN];
  uint8_t pk[MLDSA44_PK_LEN];

  if (KeyGen(sk, pk) != 0) {
    fprintf(stderr, "KeyGen failed\n");
    return 1;
  }

  printf("The secret key is:\n");
  for (size_t i = 0; i < MLDSA44_SK_LEN; i++) {
    printf("%02x", sk[i]);
  }
  printf("\n");

  printf("The public key is:\n");
  for (size_t i = 0; i < MLDSA44_PK_LEN; i++) {
    printf("%02x", pk[i]);
  }
  printf("\n");

  return 0;
}
