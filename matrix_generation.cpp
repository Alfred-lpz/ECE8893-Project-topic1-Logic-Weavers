
#include "dcl.h"

#define DIM 200 // Matrix dimension

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



void float2myFP_16(float *f, data_16 *h, int EB, int MB, bool *overflow)
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


float myFP2float_8(const data_8 h, int EB, int MB)
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



float myFP2float_16(const data_16 h, int EB, int MB)
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


template <typename T>
void readMatrix(const char* filename, T matrix[DIM][DIM]) {
    std::ifstream infile(filename, std::ios::binary);
    infile.read(reinterpret_cast<char*>(matrix), DIM * DIM * sizeof(T));
    infile.close();
}

template <typename T>
void writeMatrix(const char* filename, T matrix[DIM][DIM]) {
    std::ofstream outfile(filename, std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(matrix), DIM * DIM * sizeof(T));
    outfile.close();
}

void generateFloatMatrix(const char* filename) {
    std::ofstream outfile(filename, std::ios::binary);
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            float value = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
            outfile.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    outfile.close();
}

void convertFloatToMyFP_8(const char* inputFile, const char* outputFile, int EB, int MB) {
    float matrixFloat[DIM][DIM];
    data_8 matrixMyFP[DIM][DIM];
    readMatrix(inputFile, matrixFloat);

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            bool overflow = false;
            float2myFP_8(&matrixFloat[i][j], &matrixMyFP[i][j], EB, MB, &overflow);
        }
    writeMatrix(outputFile, matrixMyFP);
}

void convertFloatToMyFP_16(const char* inputFile, const char* outputFile, int EB, int MB) {
    float matrixFloat[DIM][DIM];
    data_16 matrixMyFP[DIM][DIM];
    readMatrix(inputFile, matrixFloat);

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            bool overflow = false;
            float2myFP_16(&matrixFloat[i][j], &matrixMyFP[i][j], EB, MB, &overflow);
        }
    writeMatrix(outputFile, matrixMyFP);
}


void convertFloatToAPFix_16(const char* inputFile, const char* outputFile) {
    float matrixFloat[DIM][DIM];
    ap_fixed<16,5> matrixAPFix[DIM][DIM];
    readMatrix(inputFile, matrixFloat);

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            matrixAPFix[i][j] = (ap_fixed<16, 5>)matrixFloat[i][j];
        }

    writeMatrix(outputFile, matrixAPFix);
}

void convertFloatToAPFix_8(const char* inputFile, const char* outputFile) {
    float matrixFloat[DIM][DIM];
    ap_fixed<8, 4, AP_TRN, AP_SAT> matrixAPFix[DIM][DIM];
    readMatrix(inputFile, matrixFloat);

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            matrixAPFix[i][j] = (ap_fixed<8, 4, AP_TRN, AP_SAT>)matrixFloat[i][j];
        }
        
    writeMatrix(outputFile, matrixAPFix);
}

void convertFloatMatrixToMyFP_8(float matrixFloat[DIM][DIM], const char* outputFile, int EB, int MB) {
    data_8 matrixMyFP[DIM][DIM];

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            bool overflow = false;
            float2myFP_8(&matrixFloat[i][j], &matrixMyFP[i][j], EB, MB, &overflow);
        }

    writeMatrix(outputFile, matrixMyFP);
}

void convertFloatMatrixToMyFP_16(float matrixFloat[DIM][DIM], const char* outputFile, int EB, int MB) {
    data_16 matrixMyFP[DIM][DIM];

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            bool overflow = false;
            float2myFP_16(&matrixFloat[i][j], &matrixMyFP[i][j], EB, MB, &overflow);
        }

    writeMatrix(outputFile, matrixMyFP);
}


void convertFloatMatrixToAPFix_8(float matrixFloat[DIM][DIM], const char* outputFile) {
    data_8 matrixFix[DIM][DIM];  

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            ap_fixed<8, 4, AP_TRN, AP_SAT> fx = matrixFloat[i][j];
            matrixFix[i][j] = fx.range(7, 0); 
        }

    writeMatrix(outputFile, matrixFix);
}

void convertFloatMatrixToAPFix_16(float matrixFloat[DIM][DIM], const char* outputFile) {
    data_16 matrixFix[DIM][DIM];  

    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            ap_fixed<16, 5, AP_TRN, AP_SAT> fx = matrixFloat[i][j];
            matrixFix[i][j] = fx.range(15, 0); 
        }

    writeMatrix(outputFile, matrixFix);
}


int main() {
    const char* matrixA_float_file = "matrix_a_float_1.bin";
    const char* matrixB_float_file = "matrix_b_float_1.bin";
    const char* matrixC_float_file = "matrix_c_float_1.bin";

    const char* matrixA_E4M3_file = "matrix_a_E4M3_1.bin";
    const char* matrixB_E4M3_file = "matrix_b_E4M3_1.bin";
    const char* matrixC_E4M3_file = "matrix_c_E4M3_1.bin";

    const char* matrixA_E5M2_file = "matrix_a_E5M2_1.bin";
    const char* matrixB_E5M2_file = "matrix_b_E5M2_1.bin";
    const char* matrixC_E5M2_file = "matrix_c_E5M2_1.bin";


    const char* matrixA_E5M10_file = "matrix_a_E5M10_1.bin";
    const char* matrixB_E5M10_file = "matrix_b_E5M10_1.bin";
    const char* matrixC_E5M10_file = "matrix_c_E5M10_1.bin";

    const char* matrixA_AP16_5_file = "matrix_a_AP16_5_1.bin";
    const char* matrixB_AP16_5_file = "matrix_b_AP16_5_1.bin";
    const char* matrixC_AP16_5_file = "matrix_c_AP16_5_1.bin";

    const char* matrixA_AP8_4_file = "matrix_a_AP8_4_1.bin";
    const char* matrixB_AP8_4_file = "matrix_b_AP8_4_1.bin";
    const char* matrixC_AP8_4_file = "matrix_c_AP8_4_1.bin";

    // Generate floating-point matrices
    generateFloatMatrix(matrixA_float_file);
    generateFloatMatrix(matrixB_float_file);

    // Convert floating-point matrices to custom floating-point formats
    convertFloatToMyFP_8(matrixA_float_file, matrixA_E4M3_file, 4, 3);
    convertFloatToMyFP_8(matrixB_float_file, matrixB_E4M3_file, 4, 3);
    convertFloatToMyFP_8(matrixA_float_file, matrixA_E5M2_file, 5, 2);
    convertFloatToMyFP_8(matrixB_float_file, matrixB_E5M2_file, 5, 2);
    convertFloatToMyFP_16(matrixA_float_file, matrixA_E5M10_file, 5, 10);
    convertFloatToMyFP_16(matrixB_float_file, matrixB_E5M10_file, 5, 10);

    // Convert floating-point matrices to ap_fixed formats
    convertFloatToAPFix_16(matrixA_float_file, matrixA_AP16_5_file);
    convertFloatToAPFix_16(matrixB_float_file, matrixB_AP16_5_file);

    convertFloatToAPFix_8(matrixA_float_file, matrixA_AP8_4_file);
    convertFloatToAPFix_8(matrixB_float_file, matrixB_AP8_4_file);

    std::cout << "All computations complete. Results saved to respective files." << std::endl;

    float matrixA_float[DIM][DIM], matrixB_float[DIM][DIM], matrixC_float[DIM][DIM] = {0};
    readMatrix(matrixA_float_file, matrixA_float);
    readMatrix(matrixB_float_file, matrixB_float);

    // Perform floating-point matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k)
                matrixC_float[i][j] += matrixA_float[i][k] * matrixB_float[k][j];
    writeMatrix(matrixC_float_file, matrixC_float);


    // test on multiplication between matrix_a_E4M3 and matrix_b_E4M3
    data_8 matrixA_E4M3[DIM][DIM], matrixB_E4M3[DIM][DIM];
    float matrixC_E4M3_float[DIM][DIM] = {0};

    // Read matrices
    readMatrix(matrixA_E4M3_file, matrixA_E4M3);
    readMatrix(matrixB_E4M3_file, matrixB_E4M3);

    // Perform E4M3 matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                float valueA = myFP2float_8(matrixA_E4M3[i][k], 4, 3);
                float valueB = myFP2float_8(matrixB_E4M3[k][j], 4, 3);
                matrixC_E4M3_float[i][j] += valueA * valueB;
            }

    // Compute MSE
    float mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(matrixC_float[i][j] - matrixC_E4M3_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", matrixC_float[i][j], matrixC_E4M3_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and E4M3 matrix multiplication: " << mse << std::endl;

    // Save E4M3 output to file ===
    convertFloatMatrixToMyFP_8(matrixC_E4M3_float, matrixC_E4M3_file, 4, 3);



    // test on multiplication between matrix_a_E5M2 and matrix_b_E5M2
    data_8 matrixA_E5M2[DIM][DIM], matrixB_E5M2[DIM][DIM];
    float matrixC_E5M2_float[DIM][DIM] = {0};

    // Read matrices
    readMatrix(matrixA_E5M2_file, matrixA_E5M2);
    readMatrix(matrixB_E5M2_file, matrixB_E5M2);

    // Perform floating-point matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                matrixC_E5M2_float[i][j] = 0;
            }

    // Perform E5M2 matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                float valueA = myFP2float_8(matrixA_E5M2[i][k], 5, 2);
                float valueB = myFP2float_8(matrixB_E5M2[k][j], 5, 2);
                matrixC_E5M2_float[i][j] += valueA * valueB;
            }

    // Compute MSE
    mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(matrixC_float[i][j] - matrixC_E5M2_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", matrixC_float[i][j], matrixC_E5M2_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and E5M2 matrix multiplication: " << mse << std::endl;

    // Save E5M2 output to file
    convertFloatMatrixToMyFP_8(matrixC_E5M2_float, matrixC_E5M2_file, 5, 2);


    // test on multiplication between matrix_a_E5M10 and matrix_b_E5M10
    data_16 matrixA_E5M10[DIM][DIM], matrixB_E5M10[DIM][DIM];
    float matrixC_E5M10_float[DIM][DIM] = {0};

    // Read matrices
    readMatrix(matrixA_E5M10_file, matrixA_E5M10);
    readMatrix(matrixB_E5M10_file, matrixB_E5M10);

    // Perform floating-point matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                matrixC_E5M10_float[i][j] = 0;
            }

    // Perform E5M10 matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                float valueA = myFP2float_16(matrixA_E5M10[i][k], 5, 10);
                float valueB = myFP2float_16(matrixB_E5M10[k][j], 5, 10);
                matrixC_E5M10_float[i][j] += valueA * valueB;
            }

    // Compute MSE
    mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(matrixC_float[i][j] - matrixC_E5M10_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", matrixC_float[i][j], matrixC_E5M10_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and E5M10 matrix multiplication: " << mse << std::endl;
    // Save E5M10 output to file
    convertFloatMatrixToMyFP_16(matrixC_E5M10_float, matrixC_E5M10_file, 5, 10);


    // test on multiplication between matrix_a_AP8_4 and matrix_b_AP8_4
    ap_fixed<8, 4, AP_TRN, AP_SAT> matrixA_AP8_4[DIM][DIM], matrixB_AP8_4[DIM][DIM];
    float matrixC_AP8_4_float[DIM][DIM] = {0};

    // Read matrices
    readMatrix(matrixA_AP8_4_file, matrixA_AP8_4);
    readMatrix(matrixB_AP8_4_file, matrixB_AP8_4);

    // Perform floating-point matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                matrixC_AP8_4_float[i][j] = 0;
            }

    // Perform AP8_4 matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                float valueA = (float)matrixA_AP8_4[i][k];
                float valueB = (float)matrixB_AP8_4[k][j];
                matrixC_AP8_4_float[i][j] += valueA * valueB;
            }

    // Compute MSE
    mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(matrixC_float[i][j] - matrixC_AP8_4_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", matrixC_float[i][j], matrixC_AP8_4_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and AP8_4 matrix multiplication: " << mse << std::endl;
    // Save AP8_4 output to file
    convertFloatMatrixToAPFix_8(matrixC_AP8_4_float, matrixC_AP8_4_file);


    // test on multiplication between matrix_a_AP16_5 and matrix_b_AP16_5
    ap_fixed<16, 5, AP_TRN, AP_SAT> matrixA_AP16_5[DIM][DIM], matrixB_AP16_5[DIM][DIM];
    float matrixC_AP16_5_float[DIM][DIM] = {0};

    // Read matrices
    readMatrix(matrixA_AP16_5_file, matrixA_AP16_5);
    readMatrix(matrixB_AP16_5_file, matrixB_AP16_5);

    // Perform floating-point matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                matrixC_AP16_5_float[i][j] = 0;
            }

    // Perform AP16_5 matrix multiplication
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            for (int k = 0; k < DIM; ++k) {
                float valueA = (float)matrixA_AP16_5[i][k];
                float valueB = (float)matrixB_AP16_5[k][j];
                matrixC_AP16_5_float[i][j] += valueA * valueB;
            }

    // Compute MSE
    mse = 0;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) {
            mse += std::pow(matrixC_float[i][j] - matrixC_AP16_5_float[i][j], 2);
            printf("float: %.8f, myFP: %.8f\n", matrixC_float[i][j], matrixC_AP16_5_float[i][j]);
        }

    mse /= (DIM * DIM);
    std::cout << "MSE between floating-point and AP16_5 matrix multiplication: " << mse << std::endl;
    // Save AP16_5 output to file
    convertFloatMatrixToAPFix_16(matrixC_AP16_5_float, matrixC_AP16_5_file);
    
    
    
    return 0;
}