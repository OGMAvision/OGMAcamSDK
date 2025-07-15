#!/bin/bash
os=`uname -s`
if [[ $os = "Linux" ]]; then
	g++ -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demostill demostill.cpp -logmacam
else
	clang++ -Wl,-rpath -Wl,@executable_path -L. -g -o demostill demostill.cpp -logmacam
fi