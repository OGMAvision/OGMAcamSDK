#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demostillraw demostillraw.cpp -logmacam
