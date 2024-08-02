#!/bin/sh
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

FILE_SBI=./build/mysbi.bin
FILE_GOS=./build/gos.bin
FILE_OUT=./out/Image.bin

mkdir -p out

if [ ! -f "$FILE_SBI" ]; then
	echo "$FILE_SBI does not exit!!"
fi

if [ ! -f "$FILE_GOS" ]; then
	echo "$FILE_GOS does not exit!!"
fi

if [ -f "$FILE_SBI" -a -f "$FILE_GOS" ]; then
	dd if=$FILE_SBI of=$FILE_OUT
	dd if=$FILE_GOS of=$FILE_OUT bs=1M seek=2
	echo "pack success. $FILE_OUT"
fi
