#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demotriggerout demotriggerout.cpp -logmacam
