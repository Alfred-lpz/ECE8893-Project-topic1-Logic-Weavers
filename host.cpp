
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
	// data_8 a[DIM][DIM];
	// data_8 b[DIM][DIM];
	// data_8 c_HLS[DIM][DIM];
	// data_8 c_ref[DIM][DIM];
	// float c_ref_float[DIM][DIM];
	// float c_HLS_float[DIM][DIM];
	
	// // Multiply matrix_a_E4M3 with matrix_b_E4M3 and compare with matrix_c_E4M3
	
    // std::ifstream inf1("matrix_a_E4M3_1.bin", std::ios::binary);
	// std::ifstream inf2("matrix_b_E4M3_1.bin", std::ios::binary);
	// std::ifstream inf3("matrix_c_E4M3_1.bin", std::ios::binary);
	

    // for (int i = 0; i < DIM; ++i) {
    //     for (int j = 0; j < DIM; ++j) {
    //         inf1.read(reinterpret_cast<char*>(&a[i][j]), sizeof(data_8));
	// 		inf2.read(reinterpret_cast<char*>(&b[i][j]), sizeof(data_8));
	// 		inf3.read(reinterpret_cast<char*>(&c_ref[i][j]), sizeof(data_8));
    //     }
    // }
	// inf1.close();
	// inf2.close();
	// inf3.close();

	// // call HLS kernel
	// for(int i = 0; i < DIM; i++) {
	// 	for(int j = 0; j < DIM; j++) {
	// 		c_HLS[i][j] = 0;
	// 	}
	// }
	
	// printf("Calling HLS kernel to compute matrix C in E4M3...\n");
	// MatMul_E4M3(a, b, c_HLS);

	// printf("HLS kernel done.\n");

	// // compare c_HLS with c_ref

	// for(int i = 0; i < DIM; i++) {
	// 	for(int j = 0; j < DIM; j++) {
	// 		c_ref_float[i][j] = myFP2float_8(c_ref[i][j], 4, 3);
	// 		c_HLS_float[i][j] = myFP2float_8(c_HLS[i][j], 4, 3);
	// 	}
	// }
	// // Compute MSE
    // float mse = 0;
    // for (int i = 0; i < DIM; ++i)
    //     for (int j = 0; j < DIM; ++j) {
    //         mse += std::pow(c_ref_float[i][j] - c_HLS_float[i][j], 2);
    //         printf("float: %.8f, myFP: %.8f\n", c_ref_float[i][j], c_HLS_float[i][j]);
    //     }

    // mse /= (DIM * DIM);
    // std::cout << "MSE between floating-point and E5M2 matrix multiplication: " << mse << std::endl;



	// do the same for all other matrices

	
	// Top‐level wrapper: MatMul_mix_fixed(A,B,C,mode)
	// Modes: 0=E4M3, 1=E5M2, 2=E5M10, 3=AP8_4, 4=AP16_5

	constexpr int modes = 5;
	const char* suffixes[modes] = {"E4M3","E5M2","E5M10","AP8_4","AP16_5"};

	static data_16 A[DIM][DIM], B[DIM][DIM], C_HLS[DIM][DIM], C_ref[DIM][DIM];
	static float  C_HLS_f[DIM][DIM], C_ref_f[DIM][DIM];

	for (int mode = 0; mode < modes; ++mode) {
		const char* suf = suffixes[mode];
		std::cout << "Testing mode " << mode << " (" << suf << ")...\n";

		// --- Read inputs A, B ---
		std::ifstream finA(std::string("matrix_a_") + suf + "_1.bin", std::ios::binary);
		std::ifstream finB(std::string("matrix_b_") + suf + "_1.bin", std::ios::binary);

		if (!finA || !finB) {
			std::cerr << "Error: cannot open input files for mode " << mode << std::endl;
			return 1;
		}

		for (int i = 0; i < DIM; ++i) {
			for (int j = 0; j < DIM; ++j) {
				if (mode<=1 || mode==3) {
					uint8_t tmp8;
					finA.read(reinterpret_cast<char*>(&tmp8), sizeof(uint8_t));
					A[i][j] = (data_16)tmp8;
					finB.read(reinterpret_cast<char*>(&tmp8), sizeof(uint8_t));
					B[i][j] = (data_16)tmp8; // careful: actually separate reads
				} else {
					uint16_t tmp16;
					finA.read(reinterpret_cast<char*>(&tmp16), sizeof(uint16_t));
					A[i][j] = tmp16;
					finB.read(reinterpret_cast<char*>(&tmp16), sizeof(uint16_t));
					B[i][j] = tmp16;
				}
			}
		}
		finA.close(); finB.close();

		// --- Read reference C ---
		std::ifstream finC((std::string("matrix_c_") + suf + "_1.bin").c_str(), std::ios::binary);
		if (!finC) {
			std::cerr << "Error: cannot open reference file for mode " << mode << std::endl;
			return 1;
		}
		for (int i = 0; i < DIM; ++i) {
			for (int j = 0; j < DIM; ++j) {
				if (mode<=1 || mode==3) {
					uint8_t tmp8;
					finC.read(reinterpret_cast<char*>(&tmp8), sizeof(uint8_t));
					C_ref[i][j] = (data_16)tmp8;
				} else {
					uint16_t tmp16;
					finC.read(reinterpret_cast<char*>(&tmp16), sizeof(uint16_t));
					C_ref[i][j] = tmp16;
				}
			}
		}
		finC.close();

		// --- Call HLS kernel ---
		std::memset(C_HLS, 0, sizeof(C_HLS));
		MatMul_mix_fixed(A, B, C_HLS, mode);

		// --- Compute MSE ---
		double mse = 0.0;
		for (int i = 0; i < DIM; ++i) {
			for (int j = 0; j < DIM; ++j) {
				// decode to float
				switch (mode) {
					case 0:
					case 1:
						C_ref_f[i][j] = myFP2float_8((data_8)C_ref[i][j], mode==0?4:5, mode==0?3:2);
						C_HLS_f[i][j] = myFP2float_8((data_8)C_HLS[i][j], mode==0?4:5, mode==0?3:2);
						break;
					case 2:
						C_ref_f[i][j] = myFP2float_16(*(data_16*)&C_ref[i][j], 5,10);
						C_HLS_f[i][j] = myFP2float_16(*(data_16*)&C_HLS[i][j], 5,10);
						break;
					case 3:
						C_ref_f[i][j] = (*(ap_fixed<8,4>*)&C_ref[i][j]).to_float();
						C_HLS_f[i][j] = (*(ap_fixed<8,4>*)&C_HLS[i][j]).to_float();
						break;
					case 4:
						C_ref_f[i][j] = (*(ap_fixed<16,5>*)&C_ref[i][j]).to_float();
						C_HLS_f[i][j] = (*(ap_fixed<16,5>*)&C_HLS[i][j]).to_float();
						break;
				}
				printf("float: %.8f, myFP: %.8f\n", C_ref_f[i][j], C_HLS_f[i][j]);

				double diff = C_ref_f[i][j] - C_HLS_f[i][j];
				mse += diff*diff;
			}
		}
		mse /= (DIM*DIM);
		std::cout << "Mode " << mode << " (" << suf << ") MSE = " << mse << std::endl;
	}

	return 0;
}

