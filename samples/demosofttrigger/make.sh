#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demosofttrigger demosofttrigger.cpp -logmacam
