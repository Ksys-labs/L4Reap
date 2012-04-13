#!/bin/bash
########################################################################
# See the help text beloe.
vfs="$1"
shift
if [[ "x" = "$@x" ]]; then
cat <<EOF 
Usage: $0 EFS_FILE list of files to import

This is a utility for libwefs:

    http://fossil.wanderinghorse.net/repos/whefs/

Pass it an EFS file name followed by a list of local files and it will
construct an EFS and put those files in it. It "tries" to use the
smallest EFS size necessary, but its calculation is *very* inexact and
may leave a significant amount of slack space in the target EFS (on
the order of 25%). By using a very small block size we can create a
EFS with only 5-10% slack space, but then the EFS-to-FILE overhead
ratio will go up, increasing the overall size of the EFS file.

EOF
    exit 1
fi

PATH="$(dirname $0):$PATH"
export PATH
emkfs=$(which whefs-mkfs)
els=$(which whefs-ls)
ecp=$(which whefs-cp)

if [[ ! -x "${emkfs}" ]] || [[ ! -x "${els}" ]] || [[ ! -x "${ecp}" ]]; then
    echo "This script requires the following programs to be in the path: " whefs-{ls,cp,mkfs}
    exit 2
else
    echo "Using these whefs tools: " whefs-{ls,cp,mkfs}
fi

if [[ -e "$vfs" ]]; then
    echo "File [$vfs] already exists. If you want to overwrite it as an EFS tap ENTER, otherwise tap Ctrl-C now."
    read
fi


BLOCKSIZE=$((1024 * 8))

TSIZE=0 # total size so far
FCOUNT=0
SLENMAX=0
SIZEMAX=0
SIZEMIN=200
BLOCKEST=0 # estimated number of blocks we'll need

function do_calc()
{
    BLOCKEST=0
    local myblockcount
    printf "%-12s %-10s Name:\n" "Size:" "Blocks:"
    for lfile in "$@"; do
	myblockcount=0
	sz=$(stat -c '%s' $lfile)
	# ??? [[ $sz -lt $((BLOCKSIZE - 1)) ]] && sz=$((BLOCKSIZE - 1))
	TSIZE=$((TSIZE + $sz))
	FCOUNT=$((FCOUNT + 1))
#	slen=$(printf "$lfile%n" SLEN >/dev/null; echo $SLEN)
	slen=$(echo -n "$lfile" | wc -c | sed -e 's/ //g')
	[[ $slen -gt $SLENMAX ]] && SLENMAX=$slen
	[[ $sz -gt $SIZEMAX ]] && SIZEMAX=$sz
	[[ $sz -lt $SIZEMIN ]] && SIZEMIN=$sz
	myblockcount=$((sz / BLOCKSIZE))
	[[ 0 -ne $((sz % BLOCKSIZE)) ]] && myblockcount=$((myblockcount + 1))
	BLOCKEST=$((BLOCKEST + myblockcount))
	printf "%-12s %-10s %s\n" $sz $myblockcount $lfile
    #echo BLOCKEST=$BLOCKEST
    done
}
BLOCKEST=$((BLOCKEST + 5))
flist="$@"
do_calc "$@"

if [[ $BLOCKSIZE -gt $SIZEMAX ]]; then
    BLOCKSIZE=$((SIZEMAX+1))
fi

SIZEAVG=$((TSIZE / FCOUNT + 1))
SIZEMIDDLE=$(( $((SIZEMIN + SIZEMAX)) / 2))
cat <<EOF

$FCOUNT file(s) totaling $TSIZE bytes across $BLOCKEST blocks.

File sizes:
    smallest: ${SIZEMIN}
    average:  ${SIZEAVG}
    middle:   ${SIZEMIDDLE}
    largest:  ${SIZEMAX}
Max filename length: ${SLENMAX}

EOF


########################################################################
# The real problem is in knowing what an optimal block size/count is.
# Tinker with this math to find a generically useful ratio. We could
# calculate the exact number, but doing so (especially in shell code,
# without access to some form of caching structure) would be tedious.
INODES=$((FCOUNT + 1)) # 1 is reserved for the root EFS dir entry
#BLOCKSIZE=$SIZEAVG
#BLOCKSIZE=$((SIZEMAX * 75 / 100))
#BLOCKCOUNT=$((INODES * 15 / 10))
BLOCKCOUNT=$BLOCKEST

if [[ $BLOCKCOUNT -lt $INODES ]]; then
    BLOCKCOUNT=$INODES
fi

cat <<EOF
Using these parameters:
    inode count: $INODES
    block count: $BLOCKCOUNT
    block size: $BLOCKSIZE

EOF

echo "Really continue creating (or overwriting) [${vfs}]?"
echo "Tap Ctrl-C to abort to ENTER to continue..."
read

echo "Creating EFS..."
(set -x; $emkfs -s$SLENMAX -i$INODES -b$BLOCKSIZE -c$BLOCKCOUNT $vfs) || exit $?
echo "Importing files..."
$ecp $vfs $flist || {
    err=$?
    cat <<EOF
The copy process failed. The most likely reason for this is that this script
mis-estimated the number or size of EFS data blocks. Try hacking this script
to fix it.
EOF
    exit $err
}

echo "Import complete. Contents look like:"

$els $vfs || exit $?

echo "Done processing EFS container file [$vfs]:"
ls -la $vfs
