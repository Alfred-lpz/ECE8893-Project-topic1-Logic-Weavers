#include "dcl.h"

void float2myFP_8(float *f, data_8 *h, int EB, int MB, bool *overflow)
{
// #pragma HLS inline off
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

void float2myFP_16(float *f, data_16 *h, int EB, int MB, bool *overflow)
{
// #pragma HLS inline off
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

float myFP2float_8(const data_8 h, int EB, int MB)
{
// #pragma HLS inline off
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

float myFP2float_16(const data_16 h, int EB, int MB)
{
// #pragma HLS inline off
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

// Mode definitions
// 0 -> E4M3 (8-bit float -> fixed)
// 1 -> E5M2 (8-bit float -> fixed)
// 2 -> E5M10 (16-bit float -> fixed)
// 3 -> AP8_4  (8-bit fixed)
// 4 -> AP16_5 (16-bit fixed)

void MatMul_mix_fixed(
    wide_t a_wide[DIM][DIM / 32 + 1],
    // data_16 b[DIM][DIM],
    wide_t b_wide[(DIM * DIM) / 32],
    data_16 c[DIM][DIM],
    ap_uint<3> mode
) {
#pragma HLS interface m_axi port=a_wide offset=slave bundle=memA
#pragma HLS interface m_axi port=b_wide offset=slave bundle=memB
#pragma HLS interface m_axi port=c offset=slave bundle=memC
#pragma HLS interface s_axilite port=mode bundle=control
#pragma HLS interface s_axilite port=return bundle=control

    // local BRAM buffers
    static data_16 a_buf[DIM];
    static data_16 b_buf[DIM][DIM];
#pragma HLS array_partition variable=a_buf complete
#pragma HLS array_partition variable=b_buf complete dim=1

    // Load and unpack matrix B (512-bit interface, 32 packed 16-bit values)
    const int total_words = (DIM * DIM) / 32;
    for (int wi = 0; wi < total_words; ++wi) {
#pragma HLS pipeline II=1
        wide_t word = b_wide[wi];
        for (int lane = 0; lane < 32; ++lane) {
#pragma HLS unroll
            int linear_idx = wi * 32 + lane;
            int col = linear_idx / DIM;
            int row = linear_idx % DIM;
            b_buf[row][col] = word.range(16 * (lane + 1) - 1, 16 * lane);
        }
    }
    // Process each row of A
    for (int i = 0; i < DIM; i++) {
        // Load and unpack one row of A
        for (int k = 0; k < DIM; k += 32) {
#pragma HLS pipeline II=1
            wide_t word = a_wide[i][k / 32];
            for (int lane = 0; lane < 32; ++lane) {
#pragma HLS unroll
                if (k + lane < DIM)
                    a_buf[k + lane] = word.range(16 * (lane + 1) - 1, 16 * lane);
            }
        }

        // Matrix multiply
        for (int j = 0; j < DIM; j++) {
            ap_fixed<16, 6, AP_TRN, AP_SAT> acc_array[DIM];
            #pragma HLS array_partition variable=acc_array cyclic factor=16
            #pragma HLS resource variable=acc_array core=RAM_1P_BRAM
            bool is8 = (mode == 0) || (mode == 1) || (mode == 3);

            for (int k = 0; k < DIM; ++k) {
            #pragma HLS UNROLL factor=16
                data_16 a_w = a_buf[k];
                data_16 b_w = b_buf[k][j];

                if (is8) {
                    data_8 ai = a_w.range(7, 0);
                    data_8 bi = b_w.range(7, 0);

                    ap_fixed<8,4,AP_TRN,AP_SAT> fx_a, fx_b;

                    if (mode == 0 || mode == 1) {
                        int EB = (mode == 0 ? 4 : 5);
                        int MB = (mode == 0 ? 3 : 2);
                        fx_a = myFP2float_8(ai, EB, MB);
                        fx_b = myFP2float_8(bi, EB, MB);
                    } else {
                        fx_a = *reinterpret_cast<ap_fixed<8,4,AP_TRN,AP_SAT>*>(&ai);
                        fx_b = *reinterpret_cast<ap_fixed<8,4,AP_TRN,AP_SAT>*>(&bi);
                    }
                    acc_array[k] = fx_a * fx_b;

                } else {
                    ap_fixed<16,5,AP_TRN,AP_SAT> fx_a, fx_b;
                    if (mode == 2) {
                        fx_a = myFP2float_16(a_w, 5, 10);
                        fx_b = myFP2float_16(b_w, 5, 10);
                    } else {
                        fx_a = *reinterpret_cast<ap_fixed<16,5,AP_TRN,AP_SAT>*>(&a_w);
                        fx_b = *reinterpret_cast<ap_fixed<16,5,AP_TRN,AP_SAT>*>(&b_w);
                    }
                    acc_array[k] = fx_a * fx_b;
                }
            }

            ap_fixed<32, 8, AP_TRN, AP_SAT> acc = 0;
            for (int k = 0; k < DIM; ++k) {
            #pragma HLS UNROLL factor=16
                acc += acc_array[k];
            }



            float acc_f = acc.to_float();
            bool overflow = false;
            data_16 out16;

            switch (mode) {
                case 0:
                    float2myFP_8(&acc_f, reinterpret_cast<data_8*>(&out16), 4,3, &overflow);
                    break;
                case 1:
                    float2myFP_8(&acc_f, reinterpret_cast<data_8*>(&out16), 5,2, &overflow);
                    break;
                case 2:
                    float2myFP_16(&acc_f, &out16, 5, 10, &overflow);
                    break;
                case 3: // AP8_4
                {
                    ap_fixed<8,4,AP_TRN,AP_SAT> r = acc;
                    out16 = *reinterpret_cast<data_16*>(&r);
                    break;
                }
                default: // AP16_5
                {
                    ap_fixed<16,5,AP_TRN,AP_SAT> r = acc;
                    out16 = *reinterpret_cast<data_16*>(&r);
                    break;
                }
            }
            c[i][j] = out16;
        }
    }

}

// void MatMul_mix_fixed(
//     wide_t a_wide[DIM][DIM / 32 + 1],
//     // data_16 b[DIM][DIM],
//     wide_t b_wide[(DIM * DIM) / 32],
//     // data_16 c[DIM][DIM],
//     wide_t c_wide[(DIM * DIM) / 32],
//     ap_uint<3> mode
// ) {
// #pragma HLS interface m_axi port=a_wide offset=slave bundle=memA
// #pragma HLS interface m_axi port=b_wide offset=slave bundle=memB
// #pragma HLS interface m_axi port=c_wide offset=slave bundle=memC
// #pragma HLS interface s_axilite port=mode bundle=control
// #pragma HLS interface s_axilite port=return bundle=control

//     // local BRAM buffers
//     static data_16 a_buf[DIM];
//     static data_16 b_buf[DIM][DIM];
// #pragma HLS array_partition variable=a_buf complete
// #pragma HLS array_partition variable=b_buf complete dim=1

//     // Load and unpack matrix B (512-bit interface, 32 packed 16-bit values)
//     const int total_words = (DIM * DIM) / 32;
//     for (int wi = 0; wi < total_words; ++wi) {
// #pragma HLS pipeline II=1
//         wide_t word = b_wide[wi];
//         for (int lane = 0; lane < 32; ++lane) {
// #pragma HLS unroll
//             int linear_idx = wi * 32 + lane;
//             int col = linear_idx / DIM;
//             int row = linear_idx % DIM;
//             b_buf[row][col] = word.range(16 * (lane + 1) - 1, 16 * lane);
//         }
//     }
// #pragma HLS dataflow
//     hls::stream<data_16, 8> C_stream;
//     // Process each row of A
//     for (int i = 0; i < DIM; i++) {
//         // Load and unpack one row of A
//         for (int k = 0; k < DIM; k += 32) {
// #pragma HLS pipeline II=1
//             wide_t word = a_wide[i][k / 32];
//             for (int lane = 0; lane < 32; ++lane) {
// #pragma HLS unroll
//                 if (k + lane < DIM)
//                     a_buf[k + lane] = word.range(16 * (lane + 1) - 1, 16 * lane);
//             }
//         }

//         // Matrix multiply
//         for (int j = 0; j < DIM; j++) {
//             ap_fixed<16, 6, AP_TRN, AP_SAT> acc_array[DIM];
//             #pragma HLS array_partition variable=acc_array cyclic factor=16
//             #pragma HLS resource variable=acc_array core=RAM_1P_BRAM
//             bool is8 = (mode == 0) || (mode == 1) || (mode == 3);

//             for (int k = 0; k < DIM; ++k) {
//             #pragma HLS UNROLL factor=16
//                 data_16 a_w = a_buf[k];
//                 data_16 b_w = b_buf[k][j];

//                 if (is8) {
//                     data_8 ai = a_w.range(7, 0);
//                     data_8 bi = b_w.range(7, 0);

//                     ap_fixed<8,4,AP_TRN,AP_SAT> fx_a, fx_b;

//                     if (mode == 0 || mode == 1) {
//                         int EB = (mode == 0 ? 4 : 5);
//                         int MB = (mode == 0 ? 3 : 2);
//                         fx_a = myFP2float_8(ai, EB, MB);
//                         fx_b = myFP2float_8(bi, EB, MB);
//                     } else {
//                         fx_a = *reinterpret_cast<ap_fixed<8,4,AP_TRN,AP_SAT>*>(&ai);
//                         fx_b = *reinterpret_cast<ap_fixed<8,4,AP_TRN,AP_SAT>*>(&bi);
//                     }
//                     acc_array[k] = fx_a * fx_b;

//                 } else {
//                     ap_fixed<16,5,AP_TRN,AP_SAT> fx_a, fx_b;
//                     if (mode == 2) {
//                         fx_a = myFP2float_16(a_w, 5, 10);
//                         fx_b = myFP2float_16(b_w, 5, 10);
//                     } else {
//                         fx_a = *reinterpret_cast<ap_fixed<16,5,AP_TRN,AP_SAT>*>(&a_w);
//                         fx_b = *reinterpret_cast<ap_fixed<16,5,AP_TRN,AP_SAT>*>(&b_w);
//                     }
//                     acc_array[k] = fx_a * fx_b;
//                 }
//             }

//             ap_fixed<32, 8, AP_TRN, AP_SAT> acc = 0;
//             for (int k = 0; k < DIM; ++k) {
//             #pragma HLS UNROLL factor=16
//                 acc += acc_array[k];
//             }



//             float acc_f = acc.to_float();
//             bool overflow = false;
//             data_16 out16;

//             switch (mode) {
//                 case 0:
//                     float2myFP_8(&acc_f, reinterpret_cast<data_8*>(&out16), 4,3, &overflow);
//                     break;
//                 case 1:
//                     float2myFP_8(&acc_f, reinterpret_cast<data_8*>(&out16), 5,2, &overflow);
//                     break;
//                 case 2:
//                     float2myFP_16(&acc_f, &out16, 5, 10, &overflow);
//                     break;
//                 case 3: // AP8_4
//                 {
//                     ap_fixed<8,4,AP_TRN,AP_SAT> r = acc;
//                     out16 = *reinterpret_cast<data_16*>(&r);
//                     break;
//                 }
//                 default: // AP16_5
//                 {
//                     ap_fixed<16,5,AP_TRN,AP_SAT> r = acc;
//                     out16 = *reinterpret_cast<data_16*>(&r);
//                     break;
//                 }
//             }
//             C_stream.write(out16);
//         }
//     }

//     wide_t c_buf;
//     for (int i = 0;i<DIM*DIM/32;i++) {
//         for(int lane = 0; lane < 32; ++lane) {
//             c_buf.range(16 * (lane + 1) - 1, 16 * lane) = C_stream.read();
//         }
//         c_wide[i] = c_buf;

//     }
// }



// void MatMul_E5M2( data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {

// }


// void MatMul_E4M3(data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {
//     for (int i = 0; i < DIM; ++i)
//         for (int j = 0; j < DIM; ++j) {
//             ap_fixed<16, 5, AP_TRN, AP_SAT> acc = 0;
//             // float acc_float = 0.0;
//             for (int k = 0; k < DIM; ++k) {
//                 float va = myFP2float_8(a[i][k], 4, 3);
//                 float vb = myFP2float_8(b[k][j], 4, 3);
//                 ap_fixed<8, 4, AP_TRN, AP_SAT> fx_a = va;
//                 ap_fixed<8, 4, AP_TRN, AP_SAT> fx_b = vb;
//                 // acc_float += va * vb;
//                 acc += fx_a * fx_b;
//             }
//             ap_fixed<8, 4, AP_TRN, AP_SAT> final_fixed = acc;
//             c[i][j] = final_fixed.range(7, 0);
//         }
// }


// void MatMul_E4M3(data_8 a[DIM][DIM], data_8 b[DIM][DIM], data_8 c[DIM][DIM])
// {
// #pragma HLS interface m_axi port=a offset=slave max_widen_bitwidth=512 bundle=a
// #pragma HLS interface m_axi port=b offset=slave max_widen_bitwidth=512 bundle=b
// #pragma HLS interface m_axi port=c offset=slave max_widen_bitwidth=512 bundle=c
// #pragma HLS interface s_axilite port=return

//     for (int i = 0; i < DIM; ++i)
//         for (int j = 0; j < DIM; ++j) {
//             ap_fixed<16, 5, AP_TRN, AP_SAT> acc = 0;

//             for (int k = 0; k < DIM; ++k) {
//                 float va = myFP2float_8(a[i][k], 4, 3);
//                 float vb = myFP2float_8(b[k][j], 4, 3);
//                 ap_fixed<8, 4, AP_TRN, AP_SAT> fx_a = va;
//                 ap_fixed<8, 4, AP_TRN, AP_SAT> fx_b = vb;
//                 acc += fx_a * fx_b;
//             }
//             float acc_f = acc.to_float(); 
//             bool overflow = false;
//             float2myFP_8(&acc_f, &c[i][j], 4, 3, &overflow);  
//         }
// }




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