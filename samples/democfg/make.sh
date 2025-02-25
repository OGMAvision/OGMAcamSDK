#!/bin/bash
os=`uname -s`
if [[ $os = "Linux" ]]; then
	g++ -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o democfg democfg.cpp -logmacam
else
	clang++ -Wl,-rpath -Wl,. -L. -g -o democfg democfg.cpp -logmacam
fi
