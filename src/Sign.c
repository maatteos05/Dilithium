#include "aux_funcs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "KeyGen_internal.h"
#include "Sign_internal.h"
#include "randombytes.h"

int Sign(size_t len_sigma, uint8_t sigma[len_sigma], uint8_t *sk,
         const uint8_t *M, size_t M_len, uint8_t *ctx, int ctx_len) {
  /* Generates an ML-DSA signature
     Input: Private key sk, message M in {0,1}*, context string ctx
     Output: Signature sigma
  */

  if (ctx_len > 255) {
    printf("exit\n");
    return -1; // fail
  }

  uint8_t rnd[32]; // randomly generate this array
  randombytes(rnd, 32);

  bool check = true;

  for (int i = 0; i < 32; i++) {
    if (rnd[i] != 0)
      check = false;
  }

  if (check) {
    printf("Random bit generation failed in signature generation\n");
    return -1;
  }

  uint8_t in[ctx_len + 2];
  uint8_t out[8 * (ctx_len + 2)];
  uint8_t empty[1] = {0};
  uint8_t res[1];
  IntegerToBytes(res, ctx_len, 1);
  memcpy(in, empty, 1);
  memcpy(in + 1, res, 1);
  memcpy(in + 2, ctx, ctx_len);

  BytesToBits(out, in, ctx_len + 2);

  // M' = BytesToBits(0 || ctx_len || ctx) || M
  // BytesToBits expands each byte to 8 bits, so output is 8*(ctx_len+2) bytes
  size_t Mp_size = 8 * (ctx_len + 2) + M_len;
  uint8_t Mp[Mp_size];

  memcpy(Mp, out, 8 * (ctx_len + 2));
  memcpy(Mp + 8 * (ctx_len + 2), M, M_len);

  Sign_internal(sigma, sk, Mp, Mp_size, rnd);
  return 0;
}

#ifdef SIGN_MAIN
int main() {
  // sk_len = 128 + 32 * ((k+l)*3 + d*k) = 128 + 32*76 = 2560 bytes
  // pk_size = 32 * k * 10 = 1280 bytes
  uint8_t sk[MLDSA44_SK_LEN], pk[MLDSA44_PK_LEN];

  // Use a fixed seed for testing (in production, use randombytes)
  uint8_t seed[32] = {0};
  seed[0] = 0x01;
  seed[1] = 0xFF;
  if (KeyGen_internal(seed, pk, sk) != 0) {
    printf("KeyGen failed \n");
    return 1;
  }

  const char *message = "Hello, this is a test message lol!";
  printf("The message is: %s\n", message);
  size_t M_len = strlen(message);

  uint8_t ctx[] = "test_context";
  size_t ctx_len = sizeof(ctx) - 1;

  size_t len_sigma =
      lambda / 4 + l * 32 * (1 + bit_len(delta1 - 1)) + omega + k;
  uint8_t sigma[len_sigma];

  int result =
      Sign(len_sigma, sigma, sk, (const uint8_t *)message, M_len, ctx, ctx_len);

  if (result != 0) {
    printf("Sign failed with code %d\n", result);
    return 1;
  }

  printf("The signed message is:\n");
  for (size_t i = 0; i < len_sigma; i++) {
    printf("%02x", sigma[i]); // Hex, 2 digits per byte
    if ((i + 1) % 32 == 0)
      printf("\n"); // Newline every 32
  }
  printf("\n");

  return 0;
}
#endif // SIGN_MAIN
