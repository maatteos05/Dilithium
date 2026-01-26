#include "aux_funcs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int Sign(uint8_t *sk, const uint8_t *M, size_t M_len, uint8_t *ctx,
         size_t ctx_len) {
  /* Generates an ML-DSA signature
     Input: Private key sk, message M in {0,1}*, context string ctx
     Output: Signature sigma
  */
  if (ctx_len < 256) {
    return -1; // fail
  }

  uint8_t rnd[32]; // randomly generate this array

  bool check = true;

  for (int i = 0; i < 32; i++) {
    if (rnd[i] != 0)
      check = false;
  }

  if (check) {
    return -1;
  }

  uint8_t in[ctx_len + 2];
  uint8_t out[ctx_len + 2];
  uint8_t empty[1] = {0};
  uint8_t res[1];
  IntegerToBytes(res, ctx_len, 1);
  memcpy(in, empty, 1);
  memcpy(in + 1, res, 1);
  memcpy(in + 2, ctx, ctx_len);

  BytesToBits(out, in, ctx_len + 2);

  uint8_t Mp[ctx_len + 2 + M_len];
  memcpy(Mp, out, ctx_len + 2);
  memcpy(Mp + ctx_len + 2, M, M_len);

  return 0;
}

int main() {
  // sk_len = 128 + 32 * ((k+l)*3 + d*k) = 128 + 32*76 = 2560 bytes
  // pk_size = 32 * k * 10 = 1280 bytes
  uint8_t sk[2560], pk[1280];

  return 0;
}