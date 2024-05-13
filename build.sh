#!/bin/bash

if [ "$#" -eq 1 ];then
	if [ "$1" = "run" ];then
		make $1
		exit 0
	fi	
	
	if [ "$1" = "run-debug" ];then
		make $1
		exit 0
	fi

	if [ "$1" = "clean" ];then
		make $1
		exit 0
	fi	
	
	if [ "$1" = "dtb" ];then
		make $1
		exit 0
	fi	
	
	make $1
fi

make clean
make
