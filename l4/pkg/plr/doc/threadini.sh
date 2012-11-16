#!/bin/bash

echo "[general]"
#echo "  print_vcpu_state    = y"
echo "  page_fault_handling = rw"
echo "  log                 = all"
echo "  threads             = yes"
echo "  redundancy          = none"
#echo "  debug               = simple"
echo ""

echo "[threads]"

function_list="__pthread_lock __pthread_unlock pthread_mutex_init pthread_mutex_lock pthread_mutex_unlock"

for f in $function_list; do
	nm $1 | grep -E "\ $f" | sed -re "s/([0-9a-f]+) [TW] ($f(_rep)?)/  \2 = 0x\1/" | sed -re "s/[_]*pthread_//g"
done
