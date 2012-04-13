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

log=${app}.valgrind

export LD_LIBRARY_PATH=".:../:${LD_LIBRARY_PATH}"
rm -f callgrind.out.*
valgrind --tool=callgrind --log-fd=3 \
    ${app} "$@" 3>${log}
of=$(ls -1t callgrind.out.* | head -1)
rc=$?
grep malloc/free ${log} && {
    echo "Details are in [${log}]"
}
if [[ x != "x${of}" && -f "$of" ]]; then
    kcachegrind $of
else
    echo "Could not find callgrind log file!"
    exit 0
fi
exit $rc
