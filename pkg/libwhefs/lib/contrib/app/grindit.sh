#!/bin/bash
if [[ x != "x$1" ]]; then
    app=./$1
    shift
else
    app=./test
fi
make ${app} || rm -f ${app}
test -x $app || {
    echo "${app} not executable."
    exit 3
}

export LD_LIBRARY_PATH=".:../:${LD_LIBRARY_PATH}"

log=${app}.valgrind
valgrind --log-fd=3 \
    --leak-check=full -v \
    --show-reachable=yes \
    ${app} "$@" 3>${log}
x=$?
grep malloc/free ${log} && {
    echo "Details are in [${log}]"
} && grep -A4 'definitely lost' "${log}"

exit $x
