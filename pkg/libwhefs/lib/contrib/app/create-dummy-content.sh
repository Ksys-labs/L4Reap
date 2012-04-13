#!/bin/bash
########################################################################
# A quick hack to generate dummy files for an EFS container.

if [[ x = "x$2" ]]; then
    cat <<EOF
Usage: $0 WHEFS-FILE COUNT

The EFS file is created if it does not exist (sized to fit the
randomly-generated input data). If it exists but cannot hold COUNT
items, the process will eventually fail with an EFS-full
(whefs_rc.FSFull).

It creates COUNT dummy files with a few bytes of random content
and adds them to the EFS.

EOF

    exit 1
fi

efs="$1"
num=$2 # inode count
blocksz=$((1024*2)) # block size
slen=$((${#num} + 10)) # inode name length: dummy-###.out
inoc=$((num+1)) # inode count
blockc=$inoc # block count
if [[ ! -f "$efs" ]]; then
    echo "EFS [$efs] not found. Creating it..."
    whefs-mkfs -i${inoc} -c${blockc} -b${blocksz} -s${slen} "$efs" || {
        err=$?
        echo "whefs-mkfs failed with rc $err!"
        exit $err
    }
    echo "The new EFS:"
    ls -la "${efs}"
fi

# imports AND DELETES $@
function doimport()
{
   whefs-cp -i "${efs}" "$@" || {
        err=$?
        echo "whefs-cp failed with rc $err!"
        exit $err
    }
    rm -f "$@"
}
trap "rm -f dummy-*.out" 0
flist=""
groupSize=50 # how many files to import at a time. Low values are very slow.
rndCalls=10
#num=$((num - (num%groupSize)))
echo "Importing $num dummy files into [$efs]..."
while [[ $num -gt 0 ]]; do
    fn="dummy-$num.out"
    flist="${flist} ${fn}"
    {
        i=$rndCalls
        while [[ $i -gt 0 ]]; do
            echo -n $RANDOM
            i=$((i - 1))
        done
        echo
    } > $fn
    if [[ 0 -eq $((num % groupSize)) ]]; then
        echo -en "\r$num...    "
        doimport $flist
        flist=""
    fi
    num=$((num - 1))
done
if [[ "x" != "$flist" ]]; then
    echo -en "\ralmost done...    "
    doimport $flist
fi
echo
echo -n "Done importing. File count in EFS, according to whefs-ls: "
whefs-ls -1 "${efs}" | wc -l
echo "The EFS file:"
ls -la "${efs}"