open_project project_AP16_5

# set top function of the HLS design
# set_top MatMul_AP16_5
set_top MatMul_AP_fix_16_5

# add source file
add_files top_AP16_5.cpp

# add testbench
add_files -tb host_AP16_5.cpp

# add data file

add_files -tb matrix_a_AP16_5.bin
add_files -tb matrix_b_AP16_5.bin
add_files -tb matrix_c_AP16_5.bin



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