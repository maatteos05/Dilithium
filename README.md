# Dilithium

## Name
Matteo Sainton

## Description
Implementing the post-quantum Dilithium digital signature scheme in C.

This is implemented following the work done in the publication:

Module-Lattice-Based Digital Signature Standard
https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.204.pdf#page=47.09

## Roadmap
Implement the 3 main algorithms of the digital signature:
    - Key Generation
    - Signing
    - Verifying

## Structure

KeyGen - generate a pair of public and private key.
    KeyGen_internal - does the actual work
    aux_funcs       - auxiliary functions used to perform some computations 
    NTT_zetas       - zetas informations used for NTT()
    NTT-1_zetas     - zetas informations used for NTT_inv()
    test_keygen     - test the current functions

Hash functions
    fips202         - Hash function named H (refer to publication used)
    keccakf1600     - hash function named G (refer to publication used)

## Authors and acknowledgment
Under the supervision of Prof. Banegas at Inria Saclay.

## Project status
on going.
current goal: Test the key generation algorithm.
