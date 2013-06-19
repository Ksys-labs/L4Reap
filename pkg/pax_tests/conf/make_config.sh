#!/bin/bash

cat <<EOF
-- vim:set ft=lua:

loader = L4.default_loader;
EOF

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
cat <<EOF

print("pax_$TEST started ...");
e = loader:start(
  {
	caps = {
	},
	log      = { "pax_$TEST", "g" },
	l4re_dbg = L4.Dbg.Warn,
  }, "rom/pax_$TEST");
print("pax_$TEST exited with: " .. e:wait());
print("");
EOF
done
