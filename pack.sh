#!/bin/sh

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
	dd if=$FILE_GOS of=$FILE_OUT bs=1024 seek=64
	echo "pack success. $FILE_OUT"
fi
