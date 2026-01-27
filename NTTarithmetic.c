#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void AddNTT(int len, int32_t u[len], int32_t v[len], int32_t w[len]) {
  for (int i = 0; i < len; i++) {
    u[i] = v[i] + w[i];
  }
}

void MultiplyNTT(int len, int32_t c[len], int32_t a[len], int32_t b[len]) {
  for (int i = 0; i < len; i++) {
    c[i] = a[i] * b[i];
  }
}

void ScalarVectorNTT(size_t l, int32_t w[l][256], int32_t c[256],
                     int32_t v[l][256]) {
  w = malloc(l * sizeof(int32_t));
  for (int i = 0; i < l; i++) {
    MultiplyNTT(256, w[i], c, v[i]);
  }
}

void MatrixVectorNTT(uint8_t k, uint8_t l, int32_t w[k][256],
                     int32_t M[k][l][256], int32_t v[l][256]) {
  /* Matrix to vector multiplication with elements in T_q
     Input: M (k x l), v (l x 1)
     Output: w = M v (k x 1) */
  int32_t u[k][256];
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      w[i][j] = 0;
    }
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < l; j++) {

      MultiplyNTT(256, u[i], M[i][j],
                  v[j]); // the size of the vectors in the matrix are 256 long
      AddNTT(l, w[i], w[i], u[i]);
    }
  }
}