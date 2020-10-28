#/bin/bash

make cleanall
make -j12 pfm
make clean
make -j12 pfmcv
make clean
make -j12 boot
make zip




