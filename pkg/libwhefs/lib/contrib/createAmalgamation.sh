#!/bin/bash
########################################################################
# Generates an "amalgamation build" for whefs. Output is two files,
# ${AMAL_C} and ${AMAL_H}. If all goes well they can be compiled
# as-is to get a standalone whefs/whio library.
#
########################################################################
inc_w=include/wh
inc_io=${inc_w}/whio
inc_efs=${inc_w}/whefs
srcd=src

WHIO_SRC="
${srcd}/whio.c
${srcd}/whio_common.c
${srcd}/whio_dev.c
${srcd}/whio_dev_FILE.c
${srcd}/whio_dev_fileno.c
${srcd}/whio_dev_mem.c
${srcd}/whio_dev_subdev.c
${srcd}/whio_stream.c
${srcd}/whio_stream_dev.c
${srcd}/whio_stream_FILE.c
${srcd}/whio_encode.c
${srcd}/whio_zlib.c
${srcd}/whprintf.c
"

# The ordering of headers is important for the amalgamation build
WHIO_HEADERS="
${inc_io}/whio_config.h
${inc_io}/whio_common.h
${inc_io}/whio_dev.h
${inc_io}/whio_stream.h
${inc_io}/whio_encode.h
${inc_io}/whio_devs.h
${inc_io}/whio_streams.h
${inc_io}/whio_zlib.h
${inc_w}/whprintf.h
"

WHEFS_SRC="
${srcd}/whefs_string.c
${srcd}/whefs_encode.c
${srcd}/whefs.c
${srcd}/whefs_fs.c
${srcd}/whefs_block.c
${srcd}/whefs_inode.c
${srcd}/whefs_hash.c
${srcd}/whefs_nodedev.c
${srcd}/whefs_file.c
${srcd}/whefs_cache.c
${srcd}/whefs_fs_closer.c
${srcd}/whbits.c
${srcd}/whdbg.c
${srcd}/whglob.c
${srcd}/whefs_client_util.c
"

AMAL_HEADERS="
${inc_efs}/whefs_license.h
${WHIO_HEADERS}
${inc_w}/whglob.h
${srcd}/whbits.h
${srcd}/whdbg.h
${inc_efs}/whefs_config.h
${inc_efs}/whefs.h
${srcd}/whefs_hash.h
${inc_efs}/whefs_string.h
${srcd}/whefs_encode.h
${srcd}/whefs_inode.h
${srcd}/whefs_cache.h
${inc_efs}/whefs_client_util.h
"
for i in ${AMAL_HEADERS} ${AMAL_SOURCES}; do
    test -e $i && continue
    echo "FATAL: source file [$i] not found."
    exit 127
done

AMAL_SRC1="${WHIO_SRC}"
AMAL_SRC2="${WHEFS_SRC}"
AMAL_SRC="${AMAL_SRC1} ${AMAL_SRC2}"
AMAL_NAME=whefs_amalgamation
AMAL_C=${AMAL_NAME}.c
AMAL_H=${AMAL_NAME}.h

function stripinc()
{
    sed -e '/# *include *"/d' -e '/# *include *<wh\//d'
}
function dofile()
{
    echo "/* begin file $1 */"
    echo "#line 8 \"$1\""
    stripinc < $1
    echo "/* end file $1 */"
}

timestamp=$(date)
echo "Creating ${AMAL_H}..."
echo "/* auto-generated on ${timestamp}. Do not edit! */" > "${AMAL_H}"
{
    cat <<EOF
#define WHEFS_AMALGAMATION_BUILD 1
#if ! defined __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#if defined(__cplusplus) && !defined(restrict)
#  define restrict
#endif
EOF
    for h in ${AMAL_HEADERS}; do
        dofile $h
    done
} >> "${AMAL_H}"


echo "Creating ${AMAL_C}..."
echo "/* auto-generated on ${timestamp}. Do not edit! */" > "${AMAL_C}"
{
    echo "#line 1 \"${AMAL_C}\""
    echo "#if !defined(_POSIX_C_SOURCE)"
    echo "#define _POSIX_C_SOURCE 200112L /* needed for ftello() and friends */"
    echo "#endif"
    echo "#if !defined(_ISOC99_SOURCE)"
    echo "#define _ISOC99_SOURCE 1 /* needed for snprintf() */"
    echo "#endif"
    echo "#include \"${AMAL_H}\""

    for h in ${AMAL_SRC1}; do
        dofile $h
    done
    for h in ${srcd}/whefs_details.c; do
        dofile $h
    done
    for h in ${AMAL_SRC2}; do
        dofile $h
    done

} >> "${AMAL_C}"

echo "Done:"

ls -la ${AMAL_C} ${AMAL_H}

cat <<EOF
Try to compile it with:

  gcc -c -std=c99 -pedantic -Wall ${AMAL_C}
or:
  tcc -c -Wall ${AMAL_C}
EOF
