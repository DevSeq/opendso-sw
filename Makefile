include config.mk

.PHONY: buildroot fsbl

all: images
	cp -f IMAGE/boot.bin SD/boot.bin
	cp -f IMAGE/image.itb SD/image.itb

linux_defconfig: linux-xlnx/.config

linux-xlnx/.config:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) xilinx_zynq_defconfig
	
linux_menuconfig: 
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) menuconfig
	
linux: linux-xlnx/.config
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) UIMAGE_LOADADDR=$(UIMAGE_LOADADDR) uImage -j8

linux_modules: linux-xlnx/.config
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) UIMAGE_LOADADDR=$(UIMAGE_LOADADDR) modules -j8

linux_new:
#	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) clean
#	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) zynq_zturn_defconfig
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./linux-xlnx/ ARCH=$(ARCH) UIMAGE_LOADADDR=$(UIMAGE_LOADADDR) uImage -j8
	
u-boot-xlnx/.config: configs/uboot_zynq_z_turn_defconfig
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make distclean -C ./u-boot-xlnx/
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make ../../configs/uboot_zynq_z_turn_defconfig -C ./u-boot-xlnx/
	
	
uboot: u-boot-xlnx/.config buildroot
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./u-boot-xlnx/

uboot-only:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./u-boot-xlnx/
	
		
uboot_menuconfig:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./u-boot-xlnx/ menuconfig
	
uboot_savedefconfig:	
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./u-boot-xlnx/ savedefconfig
	cp ./u-boot-xlnx/defconfig configs/uboot_zynq_z_turn_defconfig
	
dt.dtb: uboot
#	PATH=$(PATHS):$(DTC_PATH) make -C ./DT/

dt-only:
	#PATH=$(PATHS):$(DTC_PATH) make -C 
	$(DTC) ./DT/DTC/uboot-zynq-zturn.dts > dt.dtb

	
buildroot_source:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./buildroot/ source
	
buildroot/.config:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) BR2_DEFCONFIG=../configs/buildroot_openDSO_defconfig make -C ./buildroot/ defconfig
	
buildroot_savedefconfig:	
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) BR2_DEFCONFIG=../configs/buildroot_openDSO_defconfig make -C ./buildroot/ savedefconfig
	
buildroot_menuconfig: 
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) BR2_DEFCONFIG=../configs/buildroot_openDSO_defconfig make -C ./buildroot/ menuconfig
	
	
buildroot: buildroot/.config
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) BR2_DEFCONFIG=../configs/buildroot_openDSO_defconfig make -C ./buildroot/ 
	
	
fsbl:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./FSBL/

images-only:
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./IMAGE/

images: fsbl uboot buildroot dt.dtb linux 
	PATH=$(PATHS) CROSS_COMPILE=$(CROSS_COMPILE) make -C ./IMAGE/
#git clone --depth 1 --recurse-submodules --shallow-submodules -j3 https://github.com/gitmodimo/openDSO.git
#cd openDSO
#make all

# fatload mmc 0 0x1000000 top_wrapper.bit; fpga loadb 0 0x1000000 2083850
# mmc dev 0 && fatload mmc 0 0x10000000 image.itb && bootm 0x10000000

update-sd: dt-only images-only
	cp IMAGE/boot.bin IMAGE/image.itb /media/twl/98CE-C84C
	sync

bootcmd:
	tftpboot 0x1000000 top_wrapper.bit; fpga loadb 0 0x1000000 2083850;tftpboot 0x10000000 image.itb && bootm 0x10000000

	fatload mmc 0 0x1000000 top_wrapper.bit; fpga loadb 0 0x1000000 2083850;fatload mmc 0 0x10000000 image.itb && bootm 0x10000000
	mw 0xF8000008  0xDF0D
	mw F8000240 1
	md F8000240 
	md 41200000 



