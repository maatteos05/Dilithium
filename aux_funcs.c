#include <stdint.h>
#include <stddef.h>

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