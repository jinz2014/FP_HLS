if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}

vlib rtl_work

# copy the modelsim.ini file from the installation directory 
# to the current working directory
vmap -c work rtl_work

set PROJ_PATH "/share/jinz/Xilinx_synthesis/MyResearch/PipeGen4"

set PATH "$PROJ_PATH/Modelsim"
set LIB_PATH "$PROJ_PATH/vlib"
#set COREGEN_PATH "/share/jinz/SCC/examples/smbl/coregen/vhdl"
set TEST_DATA_PATH "$PROJ_PATH/TestData"
set SIM_OPTIONS "-t ns -voptargs=+acc"


# Compile vhdl float library
set SIM_LIB "-L /usr/local/3rdparty/mentor/xilinx_libs/xilinxcorelib_ver"

# verilog only 
#-L /usr/local/3rdparty/mentor/xilinx_libs/unisims_ver"


vcom -work rtl_work -explicit $LIB_PATH/float.vhd
#vlog -work rtl_work $LIB_PATH/float.v
vlog -work rtl_work $LIB_PATH/MUX.v 
vlog -work rtl_work $LIB_PATH/Register.v 
vlog -work rtl_work $LIB_PATH/ShiftRegister.v 
vlog -work rtl_work $LIB_PATH/ShiftCtlRegister.v 
vlog -work rtl_work $LIB_PATH/RegWriteCtl.v 
#vcom -work rtl_work $COREGEN_PATH/*.vhd
