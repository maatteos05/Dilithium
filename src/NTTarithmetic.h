#ifndef NTTARITHMETIC_H
#define NTTARITHMETIC_H

#include <stddef.h>
#include <stdint.h>

void AddNTT(int len, int32_t u[len], const int32_t v[len],
            const int32_t w[len]);

void AddVectorNTT(int len, int32_t u[len][256], int32_t w[len][256],
                  int32_t v[len][256]);

void MultiplyNTT(int len, int32_t c[len], const int32_t a[len],
                 const int32_t b[len]);

void ScalarVectorNTT(size_t len, int32_t w[len][256], int32_t c[256],
                     int32_t v[len][256]);

void MatrixVectorNTT(uint8_t kk, uint8_t ll, int32_t w[kk][256],
                     const int32_t M[kk][ll][256], const int32_t v[ll][256]);

#endif // NTTARITHMETIC_H
