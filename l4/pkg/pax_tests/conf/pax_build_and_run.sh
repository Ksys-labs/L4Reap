#!/bin/bash

#get the sources via
#svn cat http://svn.tudos.org/repos/oc/tudos/trunk/repomgr | perl - init http://svn.tudos.org/repos/oc/tudos fiasco l4re

#install the required software
#apt-get install make gawk g++ binutils pkg-config subversion grub2iso

#use double quotes in case some spaces or other weird chars are present
FOC_ROOT="$PWD"
FOC_BUILDDIR="${FOC_ROOT}/build/"
FOC_SRC="${FOC_ROOT}/tudos/trunk/"
L4RE_BUILDDIR="${FOC_ROOT}/build_l4re"
MAKE_OPTS="-j10"
LOGFILE="${FOC_ROOT}/pax_log.txt"

rm "$LOGFILE"
touch "$LOGFILE"

# build stuff only if required by the user (presumably the first time)
if [ "$1" = "rebuild" ]; then
	#create the make scripts in the build directory
	pushd .
		cd "${FOC_SRC}/kernel/fiasco/"
		make BUILDDIR="$FOC_BUILDDIR"
	popd

	pushd .
		cd "$FOC_BUILDDIR"
		make config
		make "$MAKE_OPTS"
	popd

	#create the l4re build directory
	pushd .
		cd "${FOC_SRC}/l4"
		make B="$L4RE_BUILDDIR"
	popd

	pushd .
		cd "${FOC_SRC}/l4"
		make O="$L4RE_BUILDDIR" config
		make "$MAKE_OPTS" O="$L4RE_BUILDDIR"
		#make "$MAKE_OPTS" qemu E=hello MODULE_SEARCH_PATH="$FOC_BUILDDIR" O="$L4RE_BUILDDIR"
	popd
fi

TESTS=(\
	anonmap \
	execbss \
	execdata \
	execheap \
	execstack \
	getamap \
	getheap \
	getmain \
	getshlib \
	getstack \
	mprotanon \
	mprotbss \
	mprotdata \
	mprotheap \
	mprotshbss \
	mprotshdata \
	mprotstack \
	randamap \
	randbody \
	randheap1 \
	randheap2 \
	randmain1 \
	randmain2 \
	randstack1 \
	randstack2 \
	rettofunc1 \
	rettofunc1x \
	rettofunc2 \
	rettofunc2x \
	shlibbss
	shlibdata \
	shlibtest \
	shlibtest2 \
	writetext \
	)

for TEST in ${TESTS[@]}; do
	TESTNAME=pax_$TEST
	
	# ensure we've built the test binary
	if [ ! -f "${L4RE_BUILDDIR}/bin/x86_586/l4f/$TESTNAME" ]; then
		continue
	fi

	pushd .
cat > pax_tests.cfg <<EOF 
loader = L4.default_loader;

print("pax test started: $TESTNAME...");
e = loader:start(
  {
	caps = {
	},
	log      = { "$TESTNAME", "g" },
	l4re_dbg = L4.Dbg.Warn,
  }, "rom/$TESTNAME");
print("$TESTNAME exited with: " .. e:wait());
print("");
EOF
	
	cd "${FOC_SRC}/l4"
	make "$MAKE_OPTS" grub2iso E=pax_tests MODULES_LIST="${FOC_SRC}/l4/pkg/pax_tests/conf/modules.list" MODULE_SEARCH_PATH="${FOC_ROOT}:${FOC_BUILDDIR}" O="$L4RE_BUILDDIR"

	popd
	./pax_qemu_test --test_name $TESTNAME --iso "${L4RE_BUILDDIR}/images/pax_tests.iso" --log "$LOGFILE"
done
