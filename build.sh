#!/bin/bash

if [ "$#" -eq 1 ];then
	if [ "$1" = "run" ];then
		make $1
		exit 0
	
	elif [ "$1" = "run-debug" ];then
		make $1
		exit 0

	elif [ "$1" = "clean" ];then
		make $1
		exit 0
	
	elif [ "$1" = "dtb" ];then
		make $1
		exit 0

	elif [ "$1" = "vcs-h" ];then
		make gos-vcs-h.dtb
		make vcs_h_defconfig

	elif [ "$1" = "vcs-minimum" ];then
		make gos-minimum.dtb
		make minimum_defconfig

	elif [ "$1" = "default" ];then
		make gos-dualcore.dtb
		make defconfig

	elif [ "$1" = "fpga" ];then
		make gos-dualcore.dtb
		make fpga_defconfig

	elif [ "$1" = "default-Sv48" ];then
		make gos-dualcore-Sv48.dtb
		make Sv48_defconfig

	elif [ "$1" = "default-Sv57" ];then
		make gos-dualcore-Sv57.dtb
		make Sv57_defconfig
	elif [ "$1" = "fpga-h" ];then
		make gos-singlecore.dtb
		make fpga_h_defconfig
	else
		make $1
	fi
fi

make clean
make

if [ "$1" = "fpga" ];then
	make fpga
fi
