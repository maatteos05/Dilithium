#include "aux_funcs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "KeyGen_internal.h"
#include "Sign.h"
#include "Verify.h"
#include "Verify_internal.h"

bool Verify(uint8_t *pk, uint8_t *M, size_t M_len, size_t len_sigma,
            uint8_t sigma[len_sigma], uint8_t *ctx, int ctx_len) {
  // Verifies a signature sigma for a message M.

  uint8_t pad;
  uint8_t padding;
  uint8_t input[ctx_len + 2];
  uint8_t output[8 * (ctx_len + 2)];

  size_t Mp_len = 8 * (ctx_len + 2) + M_len;
  uint8_t Mp[Mp_len];

  IntegerToBytes(&pad, 0, 1);
  IntegerToBytes(&padding, ctx_len, 1);
  input[0] = pad;
  input[1] = padding;
  memcpy(input + 2, ctx, ctx_len);
  BytesToBits(output, input, ctx_len + 2);

  memcpy(Mp, output, 8 * (ctx_len + 2));
  memcpy(Mp + 8 * (ctx_len + 2), M, M_len);

  return Verify_internal(len_sigma, sigma, pk, Mp, Mp_len);
}

int main() {
  printf("=== Dilithium Verify Test ===\n\n");

  // 1. Generate keypair
  uint8_t sk[MLDSA44_SK_LEN], pk[MLDSA44_PK_LEN];
  uint8_t seed[32] = {0};
  seed[0] = 0x01;
  seed[1] = 0xFF;

  printf("Generating keypair...\n");
  if (KeyGen_internal(seed, pk, sk) != 0) {
    printf("KeyGen failed!\n");
    return 1;
  }
  printf("Keypair generated successfully.\n\n");

  // 2. Sign a test message
  const char *message = "Hello, Dilithium verification test!";
  printf("Message: \"%s\"\n", message);
  size_t M_len = strlen(message);

  uint8_t ctx[] = "test_context";
  size_t ctx_len = sizeof(ctx) - 1;

  size_t len_sigma =
      lambda / 4 + l * 32 * (1 + bit_len(delta1 - 1)) + omega + k;
  uint8_t sigma[len_sigma];

  printf("Signing message...\n");
  if (Sign(len_sigma, sigma, sk, (const uint8_t *)message, M_len, ctx,
           ctx_len) != 0) {
    printf("signed failed\n");
    return 1;
  }
  printf("Message signed successfully.\n\n");

  // 3. Verify the valid signature
  printf("Verifying valid signature...\n");
  bool valid =
      Verify(pk, (uint8_t *)message, M_len, len_sigma, sigma, ctx, ctx_len);
  printf("Verification result: %s\n\n", valid ? "VALID ✓" : "INVALID ✗");

  // 4. Test with tampered signature (flip a bit)
  printf("Testing with tampered signature...\n");
  sigma[100] ^= 0x01; // Flip one bit
  bool tampered =
      Verify(pk, (uint8_t *)message, M_len, len_sigma, sigma, ctx, ctx_len);
  printf("Tampered signature result: %s\n\n",
         tampered ? "VALID (unexpected!)" : "INVALID ✓ (expected)");

  // 5. Summary
  printf("=== Test Summary ===\n");
  printf("Valid signature:   %s\n", valid ? "PASS" : "FAIL");
  printf("Tampered rejected: %s\n", !tampered ? "PASS" : "FAIL");

  return (valid && !tampered) ? 0 : 1;
}