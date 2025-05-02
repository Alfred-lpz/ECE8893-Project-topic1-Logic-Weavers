#include "dcl.h"
#include <ap_int.h>
#include <stdint.h>


// -----------------------------------------------------------------------------
// Convert IEEE-754 binary16 (half) to IEEE-754 binary32 (float)
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
            // subnormal half → normalized float
            int    shift = 0;
            uint32_t mant = frac;
            // normalize mantissa
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
        // normal case
        int32_t new_exp = int32_t(exp) - 15 + 127;
        f_exp  = (uint32_t)new_exp << 23;
        f_frac = frac << 13;
    }

    uint32_t fbits = f_sign | f_exp | f_frac;
    union { uint32_t u; float f; } conv;
    conv.u = fbits;
    return conv.f;
}

// -----------------------------------------------------------------------------
// Convert IEEE-754 binary32 (float) to IEEE-754 binary16 (half), round-to-nearest
static data_16 float_to_half(float vf) {
    union { uint32_t u; float f; } conv;
    conv.f = vf;
    uint32_t fbits = conv.u;

    uint32_t sign = (fbits >> 31) & 0x1;
    int32_t  exp  = int32_t((fbits >> 23) & 0xFF) - 127;
    uint32_t frac =  fbits & 0x7FFFFF;

    uint16_t h_sign = sign << 15;
    uint16_t h_exp;
    uint16_t h_frac;

    if (exp < -14) {
        // too small => subnormal or zero
        if (exp < -24) {
            // underflow to zero
            h_exp  = 0;
            h_frac = 0;
        } else {
            // subnormal half
            int shift = -14 - exp;
            uint32_t mant = (1u << 23) | frac;  // implicit 1
            // round to nearest
            uint32_t rnd = mant + (1u << (shift - 1));
            h_frac = uint16_t(rnd >> (shift + 13)); 
            h_exp  = 0;
        }
    }
    else if (exp > 15) {
        // overflow => inf
        h_exp  = 0x1F << 10;
        h_frac = 0;
    }
    else {
        // normal half
        h_exp = uint16_t(exp + 15) << 10;
        // take top 10 bits of frac with rounding
        uint32_t mant = frac;
        uint32_t rnd  = mant + 0x1000;            // 1 << 12
        h_frac = uint16_t(rnd >> 13);
        if (h_frac & 0x400) {                     // overflow in mantissa?
            h_frac = 0;
            h_exp  += (1 << 10);
        }
        if ((h_exp >> 10) >= 0x1F) {              // exponent overflow?
            h_exp  = 0x1F << 10;
            h_frac = 0;
        }
    }

    return data_16(h_sign | h_exp | h_frac);
}

// -----------------------------------------------------------------------------
// Top‐level matrix‐multiply for half‐precision (E5M10)
void MatMul_E5M10(
    data_16 A[DIM][DIM],
    data_16 B[DIM][DIM],
    data_16 C[DIM][DIM]
) {
    // HLS interface pragmas (adjust bundles/ports to your design)
#pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port=A bundle=control
#pragma HLS INTERFACE s_axilite port=B bundle=control
#pragma HLS INTERFACE s_axilite port=C bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Triple‐nested loop
    //   – pipeline inner two loops for II=1
    //   – can further unroll or tile as needed
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            float sum = 0.0f;
            for (int k = 0; k < DIM; k++) {
                float fa = half_to_float(A[i][k]);
                float fb = half_to_float(B[k][j]);
                sum += fa * fb;
            }
            C[i][j] = float_to_half(sum);
        }
    }
}
