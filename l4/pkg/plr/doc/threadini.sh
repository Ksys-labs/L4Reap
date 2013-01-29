#!/bin/bash

echo "# generated for '$1'"
echo "[general]"
echo "  page_fault_handling = rw"
echo "  threads             = yes"
echo "#  redundancy          = none"
echo "#  redundancy          = dual"
echo "#  redundancy          = triple"
echo "  logbuf = 12"
echo "  logrdtsc = true"
echo "  logreplica = true"
echo ""
echo "#  print_vcpu_state    = y"
echo "#  log                 = all"

echo ""

echo "[threads]"

function_list="__pthread_lock __pthread_unlock pthread_mutex_lock pthread_mutex_unlock"

for f in $function_list; do
	nm $1 | grep -E "\ $f" | sed -re "s/([0-9a-f]+) [TW] ($f(_rep)?)/  \2 = 0x\1/" | sed -re "s/[_]*pthread_//g"
done


kiptime=""
for line in `objdump -lSCd $1 | grep ff0a0 | cut -d: -f 1`; do
	kiptime+="0x$line "
done
kiptime=`echo $kiptime | sed -re 's/\w$//' | sed -re 's/ 0x/,0x/g'`
echo ""
echo "[kip-time]"
echo "target = $kiptime"
