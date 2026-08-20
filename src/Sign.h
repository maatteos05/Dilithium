#ifndef SIGN_H
#define SIGN_H

#include <stddef.h>
#include <stdint.h>

// Generates an ML-DSA signature
int Sign(size_t len_sigma, uint8_t sigma[len_sigma], uint8_t *sk,
         const uint8_t *M, size_t M_len, uint8_t *ctx, int ctx_len);

#endif // SIGN_H
