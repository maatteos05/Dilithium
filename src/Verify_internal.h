#ifndef VERIFY_INTERNAL_H
#define VERIFY_INTERNAL_H

#include "params.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void pkDecode(uint8_t *pk, uint8_t rho[32], int32_t t1[k][256]);

void SigDecode(size_t sigma_len, uint8_t sigma[sigma_len],
               uint8_t c_tilde[lambda / 4], int32_t z[l][256], bool h[k][256]);

int UseHint(bool h, int32_t r);

bool Verify_internal(size_t sigma_len, uint8_t sigma[sigma_len], uint8_t *pk,
                     uint8_t *Mp, size_t len_Mp);

#endif // VERIFY_INTERNAL_H
