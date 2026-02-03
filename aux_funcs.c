#include "params.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ETA 2

/**
 * Computes a base-256 representation of x mod 256^alpha
 * using little-endian order.
 *
 */
void IntegerToBytes(uint8_t *y, uint32_t x, size_t alpha) {
  /* Computes a base-256 representation of x mod 256^alpha using little-endian
  order
  Input: x and alpha
  Output; y of length alpha */
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
  /* Computes the integer value expresed by a bit string
  using little-endian order
  Input: a postivie integer alpha and a bit string y of length alpha
  Output: a nonnegative integer x */
  uint32_t x = 0;
  for (size_t i = 0; i < alpha; i++) {
    x = 2 * x + y[alpha - 1 - i];
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

  for (size_t i = 0; i < 8 * alpha; i++) {
    y[i] = 0;
  }

  uint8_t z_copy[alpha];
  for (size_t i = 0; i < alpha; i++) {
    z_copy[i] = z[i];
  }

  for (size_t i = 0; i < alpha; i++) {
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

void SimpleBitUnpack(int32_t w[256], int b, uint8_t v[32 * bit_len(b)]) {
  /* Reverses the procedure SimpleBitPack
     Input: b in N, byte string v
     Output: polynomial w in R
  */
  int c = bit_len(b);
  size_t z_len = 8 * 32 * bit_len(b); // bits = 8 * bytes
  uint8_t z[z_len];
  BytesToBits(z, v, 32 * bit_len(b));

  uint8_t bstr[c];

  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < c; j++) {
      bstr[j] = z[i * c + j]; // generate z[ic], z[ic+1], ...z[ic + c - 1]
    }
    w[i] = BitsToInteger(bstr, c);
  }
}

void BitPack(uint8_t *z, int32_t w[256], int a, int b) {
  /* Encode a polynomial w into a byte string */
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
  int len = 32 * c;
  uint8_t z[8 * len];

  BytesToBits(z, v, len);

  uint8_t zp[c];

  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < c; j++) {
      zp[j] = z[i * c + j];
    }
    w[i] = b - BitsToInteger(zp, c);
  }
}

void Decompose(int32_t r, int32_t r_decomp[2]) {
  /* FIPS 204 Algorithm 37: Decompose
   * Decomposes r into (r1, r0) such that r = r1*(2*delta2) + r0 mod q
   * with r0 in [-(delta2-1), delta2]
   */
  int32_t r_plus = r % q;
  if (r_plus < 0)
    r_plus += q; // Ensure positive representative

  // r0 = r_plus mod± (2*delta2), centered to [-(delta2-1), delta2]
  int32_t r0 = r_plus % (2 * delta2);
  if (r0 > delta2) {
    r0 -= 2 * delta2; // Center to negative
  }

  int32_t r1;
  if (r_plus - r0 == q - 1) {
    r1 = 0;
    r0 -= 1;
  } else {
    r1 = (r_plus - r0) / (2 * delta2); // Fixed: parentheses!
  }

  r_decomp[0] = r0;
  r_decomp[1] = r1;
}

int HighBits(int32_t r) {
  // Return r1 from Decompose(r)
  int32_t r_decomp[2];
  Decompose(r, r_decomp);
  return r_decomp[1];
}

int LowBits(int32_t r) {
  // Returns r0 from the output of Decompose(r)
  int32_t r_decomp[2];
  Decompose(r, r_decomp);
  return r_decomp[0];
}

int32_t fqred(int64_t x) {
  x %= (int64_t)q;
  if (x < 0)
    x += (int64_t)q;
  return (int32_t)x;
}

int32_t infNorm(int32_t w[256]) {
  /* Compute the max |w[i] mod^+- q| of w */
  int32_t w_inf = 0;
  int32_t temp, centered;
  for (int i = 0; i < 256; i++) {
    centered = fqred(w[i]); // [0, q-1]
    if (centered > q / 2) {
      centered = centered - q; // Now in [-(q-1)/2, (q-1)/2]
    }
    temp = (centered < 0) ? -centered : centered; // abs()
    if (temp > w_inf) {
      w_inf = temp;
    }
  }
  return w_inf;
}

void HintBitPack(uint8_t y[omega + k], bool h[k][256]) {
  /* Encodes a polynomial vector h with binary coefficients into
  a byte string.
  Input: a polynomial vector h
  Output: a byte string y of length omega + k that encodes h */
  for (int i = 0; i < omega + k; i++) {
    y[i] = 0;
  }
  int index = 0;
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      if (h[i][j] != 0) {
        y[index] = j;
        index++;
      }
    }
    y[omega + i] = index;
  }
}

int HintBitUnpack(bool h[k][256], uint8_t y[omega + k]) {
  /* Reverses the procedure HintBitPack
  Input: a byte string y of length w+k that encodes h as described above
  Output: a polynomial vector h in R^k_2
  */

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      h[i][j] = 0;
    }
  }
  int index = 0;
  int first;

  for (int i = 0; i < k; i++) {
    if (y[omega + i] < index || y[omega + i] > omega) {
      return -1;
    }
    first = index;

    while (index < y[omega + i]) {
      if (index > first) {
        if (y[index - 1] >= y[index]) {
          return -1;
        }
      }
      h[i][y[index]] = 1;
      index++;
    }
  }

  for (int i = index; i < omega; i++) {
    if (y[i] != 0) {
      return -1;
    }
  }

  return 0;
}