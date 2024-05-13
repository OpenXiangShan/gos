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

	elif [ "$1" = "minimum" ];then
		make gos-minimum.dtb
		make minimum_defconfig

	elif [ "$1" = "default" ];then
		make gos-dualcore.dtb
		make defconfig

	elif [ "$1" = "fpga" ];then
		make gos-dualcore.dtb
		make fpga_defconfig

	else
		make $1
	fi
fi

make clean
make

if [ "$1" = "fpga" ];then
	make fpga
fi
