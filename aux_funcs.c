#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ETA 2

/**
 * Computes a base-256 representation of x mod 256^alpha
 * using little-endian order.
 *
 */
void IntegerToBytes(uint8_t *y, uint32_t x, size_t alpha) {
  uint32_t x_p = x;
  for (size_t i = 0; i < alpha; i++) {
    y[i] = (uint8_t)(x_p % 256);
    x_p = x_p / 256;
  }
}

int32_t CoeffFromThreeBytes(uint8_t b0, uint8_t b1, uint8_t b2) {
  /* Generates an element in {0,1,..., q = 8380417} U {-1} */
  uint8_t b2_prime = b2;

  // Set the top bit to zero
  if (b2_prime > 127) {
    b2_prime -= 128;
  }

  int32_t z = ((int32_t)b2_prime << 16) | ((int32_t)b1 << 8) | (int32_t)b0;

  if (z < 8380417) { // 8380417 is prime
    return z;
  } else {
    return -1;
  }
}

int32_t CoeffFromHalfByte(uint8_t b) {
  // Generates an element of {-eta, ..., eta} U {BOT}

  if (ETA == 2) {
    if (b < 15) {
      return 2 - (b % 5);
    } else {
      return -1;
    }
  } else if (ETA == 4) {
    if (b < 9) {
      return 4 - b;
    } else {
      return -1;
    }
  }
  return -1;
}

void IntegerToBits(uint8_t *y, int32_t x, int alpha) {
  int32_t x_prime = x;
  for (int i = 0; i < alpha; i++) {
    y[i] = x_prime % 2;
    x_prime = x_prime / 2;
  }
}

int bit_len(uint32_t x) {
  /* Computes the bit length of an integer */
  if (x == 0)
    return 0;
  int length = 0;
  while (x > 0) {
    x >>= 1;
    length++;
  }
  return length;
}

void BitsToBytes(uint8_t *z, uint8_t *y, int alpha) {
  /* Algorithm 12: Converts a bit string into a byte string.
     Input: A bit string y of length alpha.
     Output: A byte string z of length ceil(alpha/8).
  */
  int out_len = (alpha + 7) / 8; // ceil(alpha / 8)

  for (int i = 0; i < out_len; i++) {
    z[i] = 0;
  }

  for (int i = 0; i < alpha; i++) {
    z[i / 8] += y[i] * (1 << (i % 8));
  }
}

void SimpleBitPack(uint8_t *z, int32_t w[256], int b) {
  /* Algorithm 16: Encodes a polynomial w into a byte string.
     Input: b (bound), w (polynomial with 256 coefficients in [0, b]).
     Output: Byte string of length 32 * bitlen(b).
  */
  int d = bit_len(b);
  int total_bits = 256 * d;

  uint8_t bits[total_bits];

  for (int i = 0; i < 256; i++) {
    IntegerToBits(bits + i * d, w[i], d);
  }

  BitsToBytes(z, bits, total_bits);
  return;
}

void BitPack(uint8_t *z, int32_t w[256], int a, int b) {
  /* Encode a polynomial into a byte string */
  int d = bit_len(a + b);
  int total_bits = 256 * d;

  uint8_t bits[total_bits];

  for (int i = 0; i < 256; i++) {
    IntegerToBits(bits + i * d, b - w[i], d);
  }

  BitsToBytes(z, bits, total_bits);
  return;
}