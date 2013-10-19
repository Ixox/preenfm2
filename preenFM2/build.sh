#/bin/bash

make cleanall
make -j5 pfm
make clean
make -j5 pfmo
make clean
make -j5 boot
make clean
make sysex
make zip




