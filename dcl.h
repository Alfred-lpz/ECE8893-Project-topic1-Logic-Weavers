
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <random>
#include <iostream>
#include <fstream>
#include <hls_stream.h>

#include "ap_fixed.h"

#define DIM 200 // matrix multiplication size
typedef ap_uint<8> data_8;
typedef ap_uint<16> data_16;
typedef ap_uint<512> wide_t;


// standard 32-bit floating point bitwidth
#define std_FP32_TOTAL_BIT      32
#define std_FP32_SIGN_BIT       1
#define std_FP32_EXPN_BIT       8
#define std_FP32_MANT_BIT       23
#define std_FP32_MANT_MUL_BIT   48
#define std_FP32_BIAS           127

#define myFP_MAX_TOTAL_BIT      8
#define myFP_MAX_SIGN_BIT       1
#define myFP_MAX_EXPN_BIT       6 // in testing, exp can go up to 6, although standard is 5-bit
#define myFP_MAX_MANT_BIT       13 // in testing, mant can go up to 13, although standard is 10-bit
#define myFP_MAX_MANT_MUL_BIT   28

typedef ap_uint<myFP_MAX_TOTAL_BIT> myFP;
typedef ap_uint<myFP_MAX_SIGN_BIT> myFP_sign;
typedef ap_uint<myFP_MAX_EXPN_BIT + 2> myFP_expn; // account for exp overflow
typedef ap_uint<myFP_MAX_MANT_BIT + 1> myFP_mant; // account for the implicit 1 before mantissa


float randomFloatRange(float min1, float max1, float min2, float max2);
void float2myFP_8(float *f, data_8 *h, int EB, int MB, bool *overflow);
float myFP2float_8(const data_8 h, int EB, int MB);
void float2myFP_16(float *f, data_16 *h, int EB, int MB, bool *overflow);
float myFP2float_16(const data_16 h, int EB, int MB);
// void float2myFP_8(float *f, myFP *h, int EB, int MB, bool *overflow);
// void float2myFP_8(float *f, myFP *h, int EB, int MB, bool *overflow);
// float myFP2float_8(const myFP h, int EB, int MB);

void MatMul_E5M2( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM]);
void MatMul_E4M3( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM]);
void MatMul_E5M10( data_16 a[DIM][DIM], data_16 b[DIM][DIM], data_16 c[DIM][DIM]);
void MatMul_Int_8( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM]);
void MatMul_AP_fix_16_5( data_16 a[DIM][DIM], data_16 b[DIM][DIM], data_16 c[DIM][DIM]);
void MatMul_AP_fix_8_4( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM]);

// Mode definitions
// 0 -> E4M3 (8-bit float -> fixed)
// 1 -> E5M2 (8-bit float -> fixed)
// 2 -> E5M10 (16-bit float -> fixed)
// 3 -> AP8_4  (8-bit fixed)
// 4 -> AP16_5 (16-bit fixed)
void MatMul_mix_fixed(
    wide_t a_wide[DIM][DIM / 32 + 1],
    // data_16 a[DIM][DIM],
    // data_16 b[DIM][DIM],
    wide_t b_wide[(DIM * DIM) / 32],
    data_16 c[DIM][DIM],
    // wide_t c_wide[(DIM * DIM) / 32],
    ap_uint<3> mode
);







