gcc -ggdb -o tstlea tstlea.c -I ../src/ -I ../../selDebug/src/sdbg/ipc/ -DSEL_DEBUG ../lib/libfastbt.so.0.3.0
LD_LIBRARY_PATH=../lib ./tstlea
