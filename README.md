# Crystals-Dilithium C Implementation (ML-DSA-44)

An implementation of the post-quantum **Crystals-Dilithium** digital signature scheme in standard C, conformed to the [FIPS 204: Module-Lattice-Based Digital Signature Standard](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.204.pdf).

This project was developed by Matteo Sainton under the supervision of **Prof. Banegas at Inria Saclay** as part of a Bachelor's Thesis.

---

## 1. What the Project is About

This repository provides a reference C implementation of the **ML-DSA-44** parameter set (security level 2). The primary objective is to implement the core algorithms cleanly and modularly, preparing the codebase for subsequent porting and optimization on resource-constrained embedded systems.

### Cryptographic Configuration (ML-DSA-44)
The parameters defined in [params.h](file:///Users/matteo/Desktop/BX/BachelorThesis/Dilithium_Sign/src/params.h) are conformed to FIPS 204 specifications:

| Parameter | Value | Description |
| :--- | :--- | :--- |
| $q$ | $8380417$ | Prime modulus |
| $d$ | $13$ | Power-of-2 rounding bit-depth |
| $\eta$ (ETA) | $2$ | Secret noise bound |
| $(k, l)$ | $(4, 4)$ | Module dimensions (vectors of size 4) |
| $\gamma_1$ (delta1)| $2^{17}$ | Mask coefficient bound |
| $\gamma_2$ (delta2)| $(q - 1) / 88$ | Low-order rounding divisor |
| $\tau$ | $39$ | Hamming weight of challenge polynomial |
| $\beta$ | $78$ | Rejection bound ($\tau \cdot \eta$) |
| $\omega$ | $80$ | Maximum hint bits threshold |

---

## 2. Directory Structure

The repository is organized to separate core cryptographic routines, auxiliary algorithms, external dependency libraries, and test suites:

```
.
├── Makefile                     # Compiler options & compilation targets
├── README.md                    # Project documentation (this file)
├── src/                         # Library sources and headers
│   ├── KeyGen.c / .h            # Top-level keypair generation
│   ├── KeyGen_internal.c / .h   # Internal keygen algorithms (NTT expansion)
│   ├── Sign.c / .h              # Top-level signature generation
│   ├── Sign_internal.c / .h     # Rejection sampling, hints, mask expansion
│   ├── Verify.c / .h            # Top-level signature verification
│   ├── Verify_internal.c / .h   # Public-key verification checking
│   ├── NTTarithmetic.c / .h     # Polynomial operations in NTT domain
│   ├── zetas_array.c            # Precomputed roots of unity for NTT
│   ├── aux_funcs.c / .h         # Bitpacking, decomposes, modular reduction
│   ├── params.h                 # ML-DSA-44 parameters and bounds
│   ├── fips202.c / .h           # SHAKE128/256 & SHA-3 implementations
│   ├── keccakf1600.c / .h       # Core Keccak-f[1600] permutation
│   ├── randombytes.c / .h       # Cryptographically secure random number generator
└── tests/                       # Unit tests & verification diagnostics
    ├── test_keygen.c            # Tests for H, Expand, NTT, and Power2Round
    └── test_aux_funcs.c         # Unit tests for bitpacking and decomposes
```

---

## 3. How to Compile and Build

Compilation is managed via the [Makefile](file:///Users/matteo/Desktop/BX/BachelorThesis/Dilithium_Sign/Makefile). The compiler options target C11 with `-O2` optimizations and strict warnings (`-Wall -Wextra`).

### Build Targets

- **Build everything:**
  ```bash
  make all
  ```
  This creates five executables in the root directory: `keygen`, `sign`, `verify`, `test_keygen`, and `test_aux_funcs`.

- **Build individual programs:**
  ```bash
  make keygen          # Build key generation binary
  make sign            # Build signature generation binary
  make verify          # Build verification test binary
  make test_keygen     # Build keygen unit test
  make test_aux_funcs  # Build auxiliary functions unit test
  ```

- **Clean build artifacts:**
  ```bash
  make clean
  ```

---

## 4. How to Run and Test

### A. Executables
1. **Key Generation (`./keygen`):**
   Generates a fresh public/private keypair using entropy sourced from the platform's secure RNG (e.g., `arc4random` on macOS or `/dev/urandom` on Linux) and prints their hexadecimal representation.
   ```bash
   ./keygen
   ```
2. **Signature Generation (`./sign`):**
   Signs a mock message under a generated private key and prints the resulting signature.
   ```bash
   ./sign
   ```
3. **Verification Test (`./verify`):**
   Generates a keypair, signs a test message, verifies the valid signature (should output `VALID ✓`), and subsequently tests a tampered signature (should reject and output `INVALID ✓ (expected)`).
   ```bash
   ./verify
   ```

### B. Diagnostic Tests
1. **NTT and Arithmetic Diagnostic (`./test_keygen`):**
   Verifies that the Number Theoretic Transform (NTT) and inverse NTT functions are correctly computed (verifying that $x = \text{NTT}^{-1}(\text{NTT}(x))$), and tests the hash and expansion bounds.
   ```bash
   ./test_keygen
   ```
2. **Auxiliary Unit Tests (`./test_aux_funcs`):**
   Runs a checklist of $19$ tests verifying modular decomposes, bitpacking, unpacking, and coefficient rejection sampling.
   ```bash
   ./test_aux_funcs
   ```

---

## 5. Current Roadmap

- **Desktop/C Reference:** Finished and verified.
- **Embedded Target (Next Phase):** Optimize stack sizes, polynomial representation, and memory allocations for deployment on microcontrollers (e.g., ARM Cortex-M4).
