#ifndef VERIFY_H
#define VERIFY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool Verify(uint8_t *pk, uint8_t *M, size_t M_len, size_t len_sigma,
            uint8_t sigma[len_sigma], uint8_t *ctx, int ctx_len);

#endif // VERIFY_H
