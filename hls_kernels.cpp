#include "dcl.h"

// ----------------------------------------------------------------------------
// Generic FP conversion templates
// ----------------------------------------------------------------------------
// DataType: data_8 or data_16
// EB: exponent bits, MB: mantissa bits
// ----------------------------------------------------------------------------
template <typename DataType, int EB, int MB>
void float2myFP(float *f, DataType *h, bool *overflow)
{
    const int TOTAL_BIT = EB + MB + 1;
    // reinterpret float bits
    ap_uint<std_FP32_TOTAL_BIT> bits = *(ap_uint<std_FP32_TOTAL_BIT> *)f;
    // sign bit
    (*h)[TOTAL_BIT - 1] = bits[std_FP32_TOTAL_BIT - 1];
    // extract original exponent
    ap_uint<std_FP32_EXPN_BIT + 2> e1 = bits.range(std_FP32_TOTAL_BIT - 2, std_FP32_MANT_BIT);
    // compute new biased exponent
    uint8_t bias_myFP = ((uint8_t)1 << (EB - 1)) - 1;
    ap_uint<std_FP32_EXPN_BIT + 2> e2 = e1 - std_FP32_BIAS + bias_myFP;
    // handle overflow/underflow
    if (e2[EB + 1] == 0 && e2[EB] == 1)
    {
        e2 = ~0;
        *overflow = true;
    }
    else if (e2[EB + 1] == 1)
    {
        e2 = 0;
        *overflow = true;
    }
    // write new exponent and mantissa
    h->range(TOTAL_BIT - 2, TOTAL_BIT - 1 - EB) = e2.range(EB - 1, 0);
    h->range(MB - 1, 0) = bits.range(std_FP32_MANT_BIT - 1, std_FP32_MANT_BIT - MB);
}

// ----------------------------------------------------------------------------
// Convert custom FP back to float
// ----------------------------------------------------------------------------
template <typename DataType, int EB, int MB>
float myFP2float(const DataType &h)
{
    const int TOTAL_BIT = EB + MB + 1;
    ap_uint<std_FP32_TOTAL_BIT> bits = 0;
    // sign
    bits[std_FP32_TOTAL_BIT - 1] = h[TOTAL_BIT - 1];
    // recover exponent
    uint8_t bias_myFP = ((uint8_t)1 << (EB - 1)) - 1;
    ap_uint<std_FP32_EXPN_BIT + 2> e1 = 0;
    e1.range(EB - 1, 0) = h.range(TOTAL_BIT - 2, TOTAL_BIT - 1 - EB);
    ap_uint<std_FP32_EXPN_BIT + 2> e2 = e1 - bias_myFP + std_FP32_BIAS;
    bits.range(std_FP32_TOTAL_BIT - 2, std_FP32_MANT_BIT) = e2.range(std_FP32_EXPN_BIT - 1, 0);
    bits.range(std_FP32_MANT_BIT - 1, std_FP32_MANT_BIT - MB) = h.range(MB - 1, 0);
    return *(float *)(&bits);
}

// ----------------------------------------------------------------------------
// Mode definitions
// 0 -> E4M3 (8-bit float)
// 1 -> E5M2 (8-bit float)
// 2 -> E5M10 (16-bit float)
// 3 -> AP8_4  (8-bit fixed)
// 4 -> AP16_5 (16-bit fixed)
// ----------------------------------------------------------------------------

void MatMul_mix_fixed(
    wide_t a_wide[DIM][DIM / 32 + 1],
    wide_t b_wide[(DIM * DIM) / 32],
    data_16 c[DIM][DIM],
    ap_uint<3> mode)
{
#pragma HLS interface m_axi port = a_wide offset = slave bundle = memA
#pragma HLS interface m_axi port = b_wide offset = slave bundle = memB
#pragma HLS interface m_axi port = c offset = slave bundle = memC
#pragma HLS interface s_axilite port = mode bundle = control
#pragma HLS interface s_axilite port = return bundle = control

    // local BRAM buffers
    static data_16 a_buf[DIM];
    static data_16 b_buf[DIM][DIM];
#pragma HLS array_partition variable = a_buf complete
#pragma HLS array_partition variable = b_buf complete dim = 1

    // Unpack B matrix into b_buf
    const int total_words = (DIM * DIM) / 32;
    for (int wi = 0; wi < total_words; ++wi)
    {
#pragma HLS pipeline II = 1
        wide_t word = b_wide[wi];
        for (int lane = 0; lane < 32; ++lane)
        {
#pragma HLS unroll
            int idx = wi * 32 + lane;
            int row = idx % DIM;
            int col = idx / DIM;
            b_buf[row][col] = word.range(16 * (lane + 1) - 1, 16 * lane);
        }
    }

    // Process each row of A
    for (int i = 0; i < DIM; ++i)
    {
        // load one row of A
        for (int k = 0; k < DIM; k += 32)
        {
#pragma HLS pipeline II = 1
            wide_t word = a_wide[i][k / 32];
            for (int lane = 0; lane < 32; ++lane)
            {
#pragma HLS unroll
                int idx = k + lane;
                if (idx < DIM)
                    a_buf[idx] = word.range(16 * (lane + 1) - 1, 16 * lane);
            }
        }

        // compute each element c[i][j]
        for (int j = 0; j < DIM; ++j)
        {
            ap_fixed<16, 6, AP_TRN, AP_SAT> acc_array[DIM];
#pragma HLS array_partition variable = acc_array cyclic factor = 16
            bool is8 = (mode == 0) || (mode == 1) || (mode == 3);
            for (int k = 0; k < DIM; ++k)
            {
#pragma HLS UNROLL factor = 16
                data_16 aval = a_buf[k];
                data_16 bval = b_buf[k][j];
                if (is8)
                {
                    data_8 ai = aval.range(7, 0);
                    data_8 bi = bval.range(7, 0);
                    ap_fixed<8, 4, AP_TRN, AP_SAT> fa, fb;
                    if (mode == 0)
                    {

                        fa = myFP2float<data_8, 4, 3>(ai);
                        fb = myFP2float<data_8, 4, 3>(bi);
                    }
                    else if (mode == 1)
                    {
                        fa = myFP2float<data_8, 5, 2>(ai);
                        fb = myFP2float<data_8, 5, 2>(bi);
                    }
                    else
                    {
                        fa = *reinterpret_cast<ap_fixed<8, 4, AP_TRN, AP_SAT> *>(&ai);
                        fb = *reinterpret_cast<ap_fixed<8, 4, AP_TRN, AP_SAT> *>(&bi);
                    }
                    acc_array[k] = fa * fb;
                }
                else
                {
                    ap_fixed<16, 5, AP_TRN, AP_SAT> fa, fb;
                    if (mode == 2)
                    {
                        fa = myFP2float<data_16, 5, 10>(a_buf[k]);
                        fb = myFP2float<data_16, 5, 10>(b_buf[k][j]);
                    }
                    else
                    {
                        fa = *reinterpret_cast<ap_fixed<16, 5, AP_TRN, AP_SAT> *>(&aval);
                        fb = *reinterpret_cast<ap_fixed<16, 5, AP_TRN, AP_SAT> *>(&bval);
                    }
                    acc_array[k] = fa * fb;
                }
            }
            // accumulation
            ap_fixed<32, 8, AP_TRN, AP_SAT> acc = 0;
            for (int k = 0; k < DIM; ++k)
            {
#pragma HLS UNROLL factor = 16
                acc += acc_array[k];
            }
            // convert back and write
            float acc_f = acc.to_float();
            bool overflow = false;
            data_16 out;
            switch (mode)
            {
            case 0:
                float2myFP<data_8, 4, 3>(&acc_f, reinterpret_cast<data_8 *>(&out), &overflow);
                break;
            case 1:
                float2myFP<data_8, 5, 2>(&acc_f, reinterpret_cast<data_8 *>(&out), &overflow);
                break;
            case 2:
                float2myFP<data_16, 5, 10>(&acc_f, &out, &overflow);
                break;
            case 3:
            {
                ap_fixed<8, 4, AP_TRN, AP_SAT> r = acc;
                out = *reinterpret_cast<data_16 *>(&r);
                break;
            }
            default:
            {
                ap_fixed<16, 5, AP_TRN, AP_SAT> r = acc;
                out = *reinterpret_cast<data_16 *>(&r);
                break;
            }
            }
            c[i][j] = out;
        }
    }
}
