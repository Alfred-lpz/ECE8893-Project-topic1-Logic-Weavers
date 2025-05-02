// host.cpp
#include <ap_int.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <cstring>

#include "dcl.h"


// copy of converter
static float e5m2_to_float(data_8 hbits) {
    uint8_t h = (uint8_t)hbits;
    uint32_t sign = (h >> 7) & 0x1;
    uint32_t exp  = (h >> 2) & 0x1F;
    uint32_t frac =  h        & 0x3;
    uint32_t f_sign = sign << 31, f_exp, f_frac;
    const int bias=15, m=2;
    if(exp==0){
        if(frac==0){ f_exp=0; f_frac=0; }
        else {
            int shift=0; uint32_t mant=frac;
            while((mant&(1u<<m))==0){mant<<=1;shift++;}
            mant &= ((1u<<m)-1);
            int32_t ne=127-bias-shift+1;
            f_exp  = (ne>0?ne:0)<<23;
            f_frac = mant<<(23-m);
        }
    }
    else if(exp==0x1F){
        f_exp=0xFF<<23; f_frac= frac?(frac<<(23-m)):0;
    }
    else{
        int32_t ne=int32_t(exp)-bias+127;
        f_exp = uint32_t(ne)<<23;
        f_frac= frac<<(23-m);
    }
    uint32_t fbits=f_sign|f_exp|f_frac;
    union{uint32_t u; float f;} conv; conv.u=fbits;
    return conv.f;
}

// kernel prototype
void MatMul_E5M2(data_8 A[DIM][DIM],
                 data_8 B[DIM][DIM],
                 data_8 C[DIM][DIM]);

// load helper
bool load_matrix(const char* fn, data_8 M[DIM][DIM]) {
    std::ifstream in(fn, std::ios::binary);
    if(!in){ std::cerr<<"Cannot open "<<fn<<"\n"; return false; }
    for(int i=0;i<DIM;i++) for(int j=0;j<DIM;j++){
        uint8_t raw; in.read((char*)&raw,1);
        if(!in){ std::cerr<<"EOF "<<fn<<" ["<<i<<"]["<<j<<"]\n"; return false; }
        M[i][j]=data_8(raw);
    }
    return true;
}

int main(){
    static data_8 A[DIM][DIM], B[DIM][DIM], C_hw[DIM][DIM], C_ref[DIM][DIM];

    if(!load_matrix("matrix_a_E5M2.bin", A))    return 1;
    if(!load_matrix("matrix_b_E5M2.bin", B))    return 1;
    if(!load_matrix("matrix_c_E5M2.bin", C_ref))return 1;

    MatMul_E5M2(A,B,C_hw);

    double mse=0.0;
    for(int i=0;i<DIM;i++) for(int j=0;j<DIM;j++){
        double df = double(e5m2_to_float(C_hw[i][j]))
                  - double(e5m2_to_float(C_ref[i][j]));
        mse += df*df;
    }
    mse /= double(DIM)*double(DIM);
    std::cout<<"E5M2 MSE = "<<mse<<"\n";
    return 0;
}
