if {[file exists work]} {
	vdel -lib work -all
}

vlib work
vmap work work

set PROJ_PATH "/share/jinz/Xilinx_synthesis/MyResearch/ALL_WBM"

set PATH "$PROJ_PATH/Modelsim"
set LIB_PATH "$PROJ_PATH/vlib"
set SIM_OPTIONS "-t ns -voptargs=+acc"

vlog -work work $LIB_PATH/ShiftRegister.v 

vsim -t ns -voptargs=+acc work.TestShiftRegister
#eval vsim $SIM_LIB $SIM_OPTIONS work.pos_TB 

run -all
