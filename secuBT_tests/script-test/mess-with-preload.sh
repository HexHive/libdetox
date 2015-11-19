#!/bin/bash
# this script tries to break out of binary translation by unsetting 
# or modifying LD_PRELOAD

#sleep 10

# this one would still work if we were to read LD_PRELOAD at the beginning 
# of fbt_check_execve instead of during initialization of secuBT
#LD_PRELOAD="" cat /proc/self/stat


unset LD_PRELOAD
cat /proc/self/stat


