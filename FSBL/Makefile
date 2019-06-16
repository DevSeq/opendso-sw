 
all: fsbl/executable.elf

fsbl/executable.elf: ../HDF/top_wrapper.hdf
	echo $(XILINX_INSTALL_PATH)
	hsi -source hsi.tcl
