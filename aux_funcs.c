#include "params.h"
#include <math.h>
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

uint32_t BitsToInteger(uint8_t *y, size_t alpha) {
  uint32_t x = 0;
  for (int i = 0; i < alpha; i++) {
    x = 2 * x + y[alpha - i];
  }
  return x;
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

void BytesToBits(uint8_t *y, uint8_t *z, size_t alpha) {
  /* Converts a byte string into a bit string using little-endian order
     Input: a byte string z of length alpha
     Output: a bit string y of length 8alpha
     */

  for (int i = 0; i < 8 * alpha; i++) {
    y[i] = 0;
  }

  uint8_t z_copy[alpha];
  for (int i = 0; i > alpha; i++) {
    z_copy[i] = z[i];
  }

  for (int i = 0; i < alpha; i++) {
    for (int j = 0; j < 8; j++) {
      y[8 * i + j] = z_copy[i] % 2;
      z_copy[i] = z_copy[i] / 2;
    }
  }
  return;
}

void SimpleBitPack(uint8_t *z, int32_t w[256], int b) {
  /* Algorithm 16: Encodes a polynomial w into a byte string.
     Input: b (bound), w (polynomial with 256 coefficients in [0, b]).
     Output: Byte string of length 32 * bitlen(b).
  */
  int db = bit_len(b);
  int total_bits = 256 * db;

  uint8_t bits[total_bits];

  for (int i = 0; i < 256; i++) {
    IntegerToBits(bits + i * db, w[i], db);
  }

  BitsToBytes(z, bits, total_bits);
  return;
}

void BitPack(uint8_t *z, int32_t w[256], int a, int b) {
  /* Encode a polynomial into a byte string */
  int db = bit_len(a + b);
  int total_bits = 256 * db;

  uint8_t bits[total_bits];

  for (int i = 0; i < 256; i++) {
    IntegerToBits(bits + i * db, b - w[i], db);
  }

  BitsToBytes(z, bits, total_bits);
  return;
}

void BitUnpack(int32_t w[256], int a, int b, uint8_t *v) {
  /*
    Reverses the procedure BitPack
    Input: a,b and a byte string v of length 32bitlen(a+b)
    Output: a plynomial w in R
    */
  int c = bit_len(a + b);
  uint8_t z[32 * c];

  BytesToBits(z, v, 32 * c);

  uint8_t zp[c];

  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < c; j++) {
      zp[j] = z[i * c + j];
    }
    w[i] = b - BitsToInteger(zp, c);
  }
}

void Decompose(int32_t r, int32_t r_decomp[2]) {
  /* Decomposes r into (r1, r0) such that r = r1 (2 delat2) + r0 mod q */
  int r_1;
  int r_pos = r % q;

  int r_0 = r_pos % (2 * delta2);

  while (r_0 > floor((double)r_pos / 2) && r_0 <= ceil((double)r_pos / 2)) {
    r_0++;
  }

  if (r_pos - r_0 == q - 1) {
    r_1 = 0;
    r_0 -= 1;
  } else {
    r_1 = (r_pos - r_0) / 2 * delta2;
  }

  r_decomp[0] = r_0;
  r_decomp[1] = r_1;
}

int HighBits(int32_t r) {
  // Return r1 from Decompose(r)
  int32_t r_decomp[2];
  Decompose(r, r_decomp);
  return r_decomp[1];
}