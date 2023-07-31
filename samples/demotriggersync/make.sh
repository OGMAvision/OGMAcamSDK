#!/bin/bash
gcc -Wl,-rpath -Wl,'$ORIGIN' -L. -g -o demotriggersync demotriggersync.cpp -logmacam
