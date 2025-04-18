#include "dcl.h"

void float2myFP_8(float *f, data_8 *h, int EB, int MB, bool *overflow)
{
    ap_uint<std_FP32_TOTAL_BIT>* ptr = (ap_uint<std_FP32_TOTAL_BIT>*)f;
    uint8_t myFP_total_bit = EB + MB + 1;

    (*h)[myFP_total_bit-1] = (*ptr)[std_FP32_TOTAL_BIT-1]; // sign bit
    ap_uint<std_FP32_EXPN_BIT + 2> e1 = (*ptr).range(std_FP32_TOTAL_BIT-2, std_FP32_MANT_BIT); // expn bits

    uint8_t bias_myFP = ((uint8_t)1 << (EB-1)) - 1; // my type bias
    ap_uint<std_FP32_EXPN_BIT + 2> e2 = 0;
    e2 = e1 - std_FP32_BIAS + bias_myFP;
    if( e2[EB+1] == 0 && e2[EB] == 1 ) {    // overflow: new expn is larger than max
        e2 = ~0;
        *overflow = true;
    }
    else if ( e2[EB+1] == 1 ) { // underflow: new expn is smaller than 0
        e2 = 0;
        *overflow = true;
    }
    (*h).range(myFP_total_bit-2, myFP_total_bit-1-EB) = e2.range(EB-1, 0); // expn bits
    (*h).range(MB-1, 0) = (*ptr).range(std_FP32_MANT_BIT-1, std_FP32_MANT_BIT-MB); // mant bits
}

float myFP2float_8(data_8 h, int EB, int MB)
{
    ap_uint<std_FP32_TOTAL_BIT> f = 0;
    
    uint8_t myFP_total_bit = EB + MB + 1;
    f[std_FP32_TOTAL_BIT-1] = h[myFP_total_bit - 1]; // sign bit

    uint8_t bias_myFP = ((uint8_t)1 << (EB-1)) - 1; // my type bias
    ap_uint<std_FP32_EXPN_BIT + 2> e1 = 0;
    e1.range(EB-1, 0) = h.range(myFP_total_bit-2, myFP_total_bit-1-EB);
    ap_uint<std_FP32_EXPN_BIT + 2> e2 = e1 - bias_myFP + std_FP32_BIAS;

    f.range(std_FP32_TOTAL_BIT-2, std_FP32_MANT_BIT) = e2.range(std_FP32_EXPN_BIT-1, 0);
    f.range(std_FP32_MANT_BIT-1, std_FP32_MANT_BIT-MB) = h.range(MB-1, 0);
    // remaining bits are pre-filled with zeros
    return *(float*)(&f);
}

// void MatMul_E5M2( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {

// }



void MatMul_E4M3(data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
{
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            float acc = 0.0;
            for (int k = 0; k < DIM; ++k) {
                float va = myFP2float_8(a[i][k], 4, 3);
                float vb = myFP2float_8(b[k][j], 4, 3);
                acc += va * vb;
            }
            bool of = false;
            float2myFP_8(&acc, &c[i][j], 4, 3, &of);
        }
}

// void MatMul_E5M10( data_16 a[DIM][DIM], data_16 b[DIM][DIM], data_16 c[DIM][DIM])
// {

// }

// void MatMul_Int_8( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {

// }

// void MatMul_AP_fix_16_5( data_16 a[DIM][DIM], data_16 b[DIM][DIM], data_16 c[DIM][DIM])
// {

// }

// void MatMul_AP_fix_8_4( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {

// }