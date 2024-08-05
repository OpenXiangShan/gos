#!/bin/bash
# Copyright (c) 2024 Beijing Institute of Open Source Chip (BOSC)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2 or later, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.


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
	elif [ "$1" = "vcs-minimum-sv48" ];then
		make gos-minimum-sv48.dtb
		make minimum_sv48_defconfig
	elif [ "$1" = "vcs-vs-multi-test" ];then
		make gos-vcs-h.dtb
		make multi_imsic_defconfig
	elif [ "$1" = "cmn-fpga-imsic-multi-test" ];then
		make gos-singlecore.dtb
		make multi_imsic_cmn_fpga_defconfig
	else
		make $1
	fi
fi

make clean
make

if [ "$1" = "fpga" ];then
	make fpga
fi
