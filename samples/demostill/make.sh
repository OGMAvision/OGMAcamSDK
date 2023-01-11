#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demostill demostill.cpp -logmacam
