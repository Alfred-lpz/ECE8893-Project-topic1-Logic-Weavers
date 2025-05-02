#include "dcl.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

// kernel prototype from dcl.h
void MatMul_AP_fix_8_4(
    data_8 A[DIM][DIM],
    data_8 B[DIM][DIM],
    data_8 C[DIM][DIM]
);

// load raw data_8 (ap_uint<8>) from binary
static bool load_matrix(const char* fn, data_8 M[DIM][DIM]) {
    std::ifstream in(fn, std::ios::binary);
    if (!in) { std::cerr<<"ERROR opening "<<fn<<"\n"; return false; }
    for (int i = 0; i < DIM; i++)
      for (int j = 0; j < DIM; j++) {
        uint8_t raw;
        in.read((char*)&raw,1);
        if (!in) { std::cerr<<"EOF "<<fn<<" ["<<i<<"]["<<j<<"]\n"; return false;}
        M[i][j] = data_8(raw);
      }
    return true;
}

int main() {
    static data_8 A[DIM][DIM], B[DIM][DIM], C_hw[DIM][DIM], C_ref[DIM][DIM];

    if (!load_matrix("matrix_a_AP8_4.bin", A))   return 1;
    if (!load_matrix("matrix_b_AP8_4.bin", B))   return 1;
    if (!load_matrix("matrix_c_AP8_4.bin", C_ref)) return 1;

    MatMul_AP_fix_8_4(A,B,C_hw);

    // MSE in float
    double mse=0;
    for(int i=0;i<DIM;i++) for(int j=0;j<DIM;j++){
        float f_hw  = float(C_hw[i][j]);
        float f_ref = float(C_ref[i][j]);
        double d = double(f_hw)-double(f_ref);
        mse += d*d;
    }
    mse /= double(DIM)*double(DIM);
    std::cout<<"AP8_4 MSE = "<<mse<<"\n";
    return 0;
}
