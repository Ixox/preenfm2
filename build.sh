#/bin/bash

make cleanall
make -j12 pfm
make clean
make -j12 pfmo
make clean
make -j12 pfmcv
make clean
make -j12 pfmcvo
make zip




