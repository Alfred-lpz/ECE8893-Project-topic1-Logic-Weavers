
/* This is the host code for the HLS kernel, used to control, provide inputs, and collect results from HLS kernel */
/* Also known as the testbench, used to check the functionality of HLS kernel */

#include "dcl.h"

// float myFP2float_8(data_8 h, int EB, int MB)
// {
//     ap_uint<std_FP32_TOTAL_BIT> f = 0;
    
//     uint8_t myFP_total_bit = EB + MB + 1;
//     f[std_FP32_TOTAL_BIT-1] = h[myFP_total_bit - 1]; // sign bit

//     uint8_t bias_myFP = ((uint8_t)1 << (EB-1)) - 1; // my type bias
//     ap_uint<std_FP32_EXPN_BIT + 2> e1 = 0;
//     e1.range(EB-1, 0) = h.range(myFP_total_bit-2, myFP_total_bit-1-EB);
//     ap_uint<std_FP32_EXPN_BIT + 2> e2 = e1 - bias_myFP + std_FP32_BIAS;

//     f.range(std_FP32_TOTAL_BIT-2, std_FP32_MANT_BIT) = e2.range(std_FP32_EXPN_BIT-1, 0);
//     f.range(std_FP32_MANT_BIT-1, std_FP32_MANT_BIT-MB) = h.range(MB-1, 0);
//     // remaining bits are pre-filled with zeros
//     return *(float*)(&f);
// }
int main()
{
	data_8 a[DIM][DIM];
	data_8 b[DIM][DIM];
	data_8 c_HLS[DIM][DIM];
	data_8 c_ref[DIM][DIM];
	float c_ref_float[DIM][DIM];
	float c_HLS_float[DIM][DIM];
	
	// Multiply matrix_a_E4M3 with matrix_b_E4M3 and compare with matrix_c_E4M3
	
    std::ifstream inf1("matrix_a_E4M3_1.bin", std::ios::binary);
	std::ifstream inf2("matrix_b_E4M3_1.bin", std::ios::binary);
	std::ifstream inf3("matrix_c_E4M3_1.bin", std::ios::binary);
	

    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            inf1.read(reinterpret_cast<char*>(&a[i][j]), sizeof(data_8));
			inf2.read(reinterpret_cast<char*>(&b[i][j]), sizeof(data_8));
			inf3.read(reinterpret_cast<char*>(&c_ref[i][j]), sizeof(data_8));
        }
    }
	inf1.close();
	inf2.close();
	inf3.close();

	// call HLS kernel
	for(int i = 0; i < DIM; i++) {
		for(int j = 0; j < DIM; j++) {
			c_HLS[i][j] = 0;
		}
	}
	
	printf("Calling HLS kernel to compute matrix C in E4M3...\n");
	MatMul_E4M3(a, b, c_HLS);

	printf("HLS kernel done.\n");

	// compare c_HLS with c_ref

	for(int i = 0; i < DIM; i++) {
		for(int j = 0; j < DIM; j++) {
			c_ref_float[i][j] = myFP2float_8(c_ref[i][j], 4, 3);
			c_HLS_float[i][j] = myFP2float_8(c_HLS[i][j], 4, 3);
		}
	}
	// Compute MSE
    float mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(c_ref_float[i][j] - c_HLS_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", c_ref_float[i][j], c_HLS_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and E5M2 matrix multiplication: " << mse << std::endl;



	// do the same for all other matrices




	

	return 0;
}

