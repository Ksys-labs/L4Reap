#!/bin/bash

# check arguments
if [ $# -ne 1 ]; then
    echo "Usage: $0 directory"
    exit 1
fi

DIR=$1

# check dir
if [ -d $DIR ]; then
    echo "Processing $DIR ..."
else
    echo "Directory not exists"
    exit 2
fi

# convert png files
for png_name in $(find $DIR -name "*.png")
do
    taff --mode c2t --exttype image --tafffile $png_name.taff --extfile $png_name
done

# convert xml files except diskorc*.xml and inputmap.xml
for xml_name in $(find $DIR -not -name "diskorc*.xml" -not -name "inputmap.xml" -name "*.xml")
do
    taff --mode c2t --tafffile $xml_name.taff --extfile $xml_name
done

CURDIR=`pwd`
cd $DIR

# check taff files
taff_files=$(ls *.taff 2> /dev/null | wc -l)

if [ $taff_files -ne 0 ]; then

    # remove image
    [ -e disko.whefs ] && rm disko.whefs

    # calculate image size
    IMGSIZE=`du -b -c *.taff | sed '$!d' | awk '{print $1}'`
    echo "image size: " $IMGSIZE

    BLCKSIZE=512
    BLCKCNT=$(( ($IMGSIZE/$BLCKSIZE) + 1 ))
    BLCKCNT=$(( $BLCKCNT + $BLCKCNT/10 ))
    echo "block count: " $BLCKCNT
    [ $BLCKCNT -le 2 ] && BLCKCNT=2
    echo "block count: " $BLCKCNT
    
    whefs-mkfs -b$BLCKSIZE -i$BLCKCNT -c$BLCKCNT disko.whefs
    whefs-cp disko.whefs *.taff
    echo "-------------------------------------------------------------------"
    whefs-ls disko.whefs
    echo "-------------------------------------------------------------------"

else
    echo "taff files not found"
    exit 3
fi

cd $CURDIR


