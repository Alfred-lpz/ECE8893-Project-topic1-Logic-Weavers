open_project project_AP8_4

# set top function of the HLS design
# set_top MatMul_AP8_4
set_top MatMul_AP_fix_8_4

# add source file
add_files top_AP8_4.cpp

# add testbench
add_files -tb host_AP8_4.cpp

# add data file

add_files -tb matrix_a_AP8_4.bin
add_files -tb matrix_b_AP8_4.bin
add_files -tb matrix_c_AP8_4.bin



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