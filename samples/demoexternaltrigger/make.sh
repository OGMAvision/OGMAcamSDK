#!/bin/bash
os=`uname -s`
if [[ $os = "Linux" ]]; then
	g++ -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demoexternaltrigger demoexternaltrigger.cpp -logmacam
else
	clang++ -Wl,-rpath -Wl,@executable_path -L. -g -o demoexternaltrigger demoexternaltrigger.cpp -logmacam
fi
