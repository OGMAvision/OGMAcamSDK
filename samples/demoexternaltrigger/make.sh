#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demoexternaltrigger demoexternaltrigger.cpp -logmacam
