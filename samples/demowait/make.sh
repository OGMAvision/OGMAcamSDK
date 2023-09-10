#!/bin/bash
os=`uname -s`
if [[ $os = "Linux" ]]; then
	g++ -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demowait demowait.cpp -logmacam
else
	clang++ -Wl,-rpath -Wl,. -L. -g -o demowait demowait.cpp -logmacam
fi
