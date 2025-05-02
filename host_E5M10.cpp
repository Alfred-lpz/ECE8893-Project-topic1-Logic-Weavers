// host.cpp
#include <ap_int.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <cstring>

#include "dcl.h"

// -----------------------------------------------------------------------------
// Copy of the half-to-float converter from your kernel file
static float half_to_float(data_16 hbits) {
    uint16_t h = (uint16_t)hbits;
    uint32_t sign = (h >> 15) & 0x1;
    uint32_t exp  = (h >> 10) & 0x1F;
    uint32_t frac =  h        & 0x3FF;

    uint32_t f_sign = sign << 31;
    uint32_t f_exp;
    uint32_t f_frac;

    if (exp == 0) {
        if (frac == 0) {
            // zero
            f_exp = 0;
            f_frac = 0;
        } else {
            // subnormal → normalize
            int shift = 0;
            uint32_t mant = frac;
            while ((mant & 0x400) == 0) {
                mant <<= 1;
                shift++;
            }
            mant &= 0x3FF;
            int32_t new_exp = int32_t(127 - 15 - shift + 1);
            f_exp  = (new_exp > 0 ? new_exp : 0) << 23;
            f_frac = mant << 13;
        }
    }
    else if (exp == 0x1F) {
        // Inf or NaN
        f_exp  = 0xFF << 23;
        f_frac = (frac ? (frac << 13) : 0);
    }
    else {
        // normal
        int32_t new_exp = int32_t(exp) - 15 + 127;
        f_exp  = (uint32_t)new_exp << 23;
        f_frac = frac << 13;
    }

    uint32_t fbits = f_sign | f_exp | f_frac;
    union { uint32_t u; float f; } conv;
    conv.u = fbits;
    return conv.f;
}

// Forward declaration of your kernel:
void MatMul_E5M10(
    data_16 A[DIM][DIM],
    data_16 B[DIM][DIM],
    data_16 C[DIM][DIM]
);

//---------------------------------------------------------------------------
// Utility: load DIM×DIM half-precision values from a binary file.
//--------------------------------------------------------------------------- 
bool load_matrix(const char* filename, data_16 M[DIM][DIM]) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "ERROR: cannot open " << filename << "\n";
        return false;
    }
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            uint16_t raw;
            in.read(reinterpret_cast<char*>(&raw), sizeof(raw));
            if (!in) {
                std::cerr << "ERROR: unexpected EOF in " << filename
                          << " at element [" << i << "][" << j << "]\n";
                return false;
            }
            M[i][j] = data_16(raw);
        }
    }
    return true;
}

//---------------------------------------------------------------------------
// Entry point
//--------------------------------------------------------------------------- 
int main(int argc, char** argv) {
    const char* fileA    = "matrix_a_E5M10.bin";
    const char* fileB    = "matrix_b_E5M10.bin";
    const char* fileCref = "matrix_c_E5M10.bin";

    static data_16 A[DIM][DIM], B[DIM][DIM];
    static data_16 C_hw[DIM][DIM], C_ref[DIM][DIM];

    // Load inputs and reference
    if (!load_matrix(fileA,    A))      return 1;
    if (!load_matrix(fileB,    B))      return 1;
    if (!load_matrix(fileCref, C_ref))  return 1;

    // Run the kernel
    MatMul_E5M10(A, B, C_hw);

    // Compute MSE
    double mse = 0.0;
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            float hw  = half_to_float(C_hw[i][j]);
            float ref = half_to_float(C_ref[i][j]);
            double diff = double(hw) - double(ref);
            mse += diff * diff;
        }
    }
    mse /= double(DIM) * double(DIM);

    std::cout << "=== MSE over " << DIM*DIM << " elements: " << mse << " ===\n";

    return 0;
}
