open_project project_1

# set top function of the HLS design
set_top MatMul_E4M3

# add source file
add_files hls_kernels.cpp

# add testbench
add_files -tb host.cpp

# add data file
add_files -tb matrix_a_float_1.bin
add_files -tb matrix_b_float_1.bin
add_files -tb matrix_c_float_1.bin
add_files -tb matrix_a_E4M3_1.bin
add_files -tb matrix_b_E4M3_1.bin
add_files -tb matrix_c_E4M3_1.bin
add_files -tb matrix_a_E5M2_1.bin
add_files -tb matrix_b_E5M2_1.bin
add_files -tb matrix_c_E5M2_1.bin
add_files -tb matrix_a_E5M10_1.bin
add_files -tb matrix_b_E5M10_1.bin
add_files -tb matrix_c_E5M10_1.bin
add_files -tb matrix_a_AP16_5_1.bin
add_files -tb matrix_b_AP16_5_1.bin
add_files -tb matrix_c_AP16_5_1.bin
add_files -tb matrix_a_AP8_4_1.bin
add_files -tb matrix_b_AP8_4_1.bin
add_files -tb matrix_c_AP8_4_1.bin


open_solution "solution1"

# FPGA part and clock configuration
set_part {xczu3eg-sbva484-1-e}

# default frequency is 100 MHz
#create_clock -period 4 -name default

csim_design
# C synthesis for HLS design, generating RTL
csynth_design

# C/RTL co-simulation; can be commented if not needed
cosim_design

# export generated RTL as an IP; can be commented if not needed
export_design -format ip_catalog -flow impl



exit