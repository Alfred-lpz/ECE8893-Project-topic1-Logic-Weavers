#include "dcl.h"
#include <ap_int.h>
#include <stdint.h>


//-----------------------------------------------------------------------------
// Convert E5M2 → float32
//-----------------------------------------------------------------------------
static float e5m2_to_float(data_8 hbits) {
    uint8_t h = (uint8_t)hbits;
    uint32_t sign = (h >> 7) & 0x1;
    uint32_t exp  = (h >> 2) & 0x1F;
    uint32_t frac =  h        & 0x3;

    uint32_t f_sign = sign << 31;
    uint32_t f_exp, f_frac;
    const int bias = 15;
    const int m    = 2;

    if (exp == 0) {
        if (frac == 0) {
            f_exp = 0; f_frac = 0;
        } else {
            int shift = 0;
            uint32_t mant = frac;
            while ((mant & (1u << m)) == 0) {
                mant <<= 1; shift++;
            }
            mant &= ((1u << m) - 1);
            int32_t new_exp = 127 - bias - shift + 1;
            f_exp  = (new_exp > 0 ? new_exp : 0) << 23;
            f_frac = mant << (23 - m);
        }
    }
    else if (exp == 0x1F) {
        f_exp  = 0xFF << 23;
        f_frac = frac ? (frac << (23 - m)) : 0;
    }
    else {
        int32_t new_exp = int32_t(exp) - bias + 127;
        f_exp  = uint32_t(new_exp) << 23;
        f_frac = frac << (23 - m);
    }

    uint32_t fbits = f_sign | f_exp | f_frac;
    union { uint32_t u; float f; } conv; conv.u = fbits;
    return conv.f;
}

//-----------------------------------------------------------------------------
// Convert float32 → E5M2
//-----------------------------------------------------------------------------
static data_8 float_to_e5m2(float vf) {
    union { uint32_t u; float f; } conv; conv.f = vf;
    uint32_t fbits = conv.u;

    uint32_t sign   = (fbits >> 31) & 0x1;
    int32_t  exp_f  = int32_t((fbits >> 23) & 0xFF) - 127;
    uint32_t frac_f =  fbits & 0x7FFFFF;

    const int bias        = 15;
    const int m           = 2;
    const int maxExpField = 0x1F;    // 5 bits all 1
    const int Emax_normal = (maxExpField - 1) - bias; // 30 - 15 = 15
    const int Emin_normal = 1 - bias;               // -14

    uint8_t h_sign = sign << 7;
    uint8_t h_exp  = 0;
    uint8_t h_frac = 0;

    if (exp_f < Emin_normal) {
        if (exp_f < (Emin_normal - m)) {
            h_exp = 0; h_frac = 0;
        } else {
            int shift = Emin_normal - exp_f;
            uint32_t mant = (1u << 23) | frac_f;
            uint32_t rnd = mant + (1u << (shift - 1));
            h_frac = uint8_t(rnd >> (shift + (23 - m)));
            h_exp  = 0;
        }
    }
    else if (exp_f > Emax_normal) {
        h_exp  = maxExpField << m;
        h_frac = 0;
    }
    else {
        uint32_t exp_field = uint32_t(exp_f + bias);
        h_exp = uint8_t(exp_field) << m;

        uint32_t rnd = frac_f + (1u << (23 - m - 1));
        h_frac = uint8_t(rnd >> (23 - m));

        if (h_frac & (1u << m)) {
            h_frac = 0;
            h_exp += (1u << m);
        }
        if ((h_exp >> m) >= maxExpField) {
            h_exp  = maxExpField << m;
            h_frac = 0;
        }
    }

    return data_8(h_sign | h_exp | h_frac);
}

//-----------------------------------------------------------------------------
// Top‐level mat‐mul in E5M2
//-----------------------------------------------------------------------------
void MatMul_E5M2(
    data_8 A[DIM][DIM],
    data_8 B[DIM][DIM],
    data_8 C[DIM][DIM]
) {
#pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2
#pragma HLS INTERFACE s_axilite port=A bundle=control
#pragma HLS INTERFACE s_axilite port=B bundle=control
#pragma HLS INTERFACE s_axilite port=C bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control

    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            float sum = 0.0f;
            for (int k = 0; k < DIM; k++) {
                float fa = e5m2_to_float(A[i][k]);
                float fb = e5m2_to_float(B[k][j]);
                sum += fa * fb;
            }
            C[i][j] = float_to_e5m2(sum);
        }
    }
}
