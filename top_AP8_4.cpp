#include "dcl.h"
#include <ap_fixed.h>
#include <cstring>

// fx = ap_fixed<8,4>, acc is wide enough to sum DIM products
using fx8_4   = ap_fixed<8,4>;
using fx_acc8 = ap_fixed<32,16>;

void MatMul_AP_fix_8_4(
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
            fx_acc8 acc = 0;
            for (int k = 0; k < DIM; k++) {
                fx8_4 a_val, b_val;
                // reinterpret the 8‐bit word as fx8_4
                a_val = *reinterpret_cast<ap_fixed<8, 4, AP_TRN, AP_SAT> *>(&A[i][k]);
                b_val = *reinterpret_cast<ap_fixed<8, 4, AP_TRN, AP_SAT> *>(&B[k][j]);
                acc += a_val * b_val;
            }
            // cast back to 8,4 and pack
            ap_fixed<8, 4, AP_TRN, AP_SAT> r = acc;
            C[i][j] = *reinterpret_cast<data_16 *>(&r);
        }
    }
}
