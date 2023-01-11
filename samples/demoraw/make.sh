#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demoraw demoraw.cpp -logmacam -lncurses
