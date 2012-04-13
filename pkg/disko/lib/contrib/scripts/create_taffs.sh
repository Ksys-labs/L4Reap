#|/bin/sh

##############################################################################
# This script generates TAFF files from dialog, theme files and PNGs.
#
# It may be used in installation steps for your disko applications.
#
# Please note that the original files will be removed if the TAFF creation
# was successfully. This leads to better performance in your application
# since timestamp checking (for changes in these files) is not necessary
# anymore.
##############################################################################

if [ ! $# -eq 1 ]; then
    echo "Generate TAFF files from dialog files and PNGs. The original files will be removed."
	echo
	echo "usage: $0 <directory>"
	exit 1;
fi;

DISKO_DIR=`pkg-config --variable=prefix disko`
TAFF="$DISKO_DIR/bin/taff"
DIR="$1"

if [ -z "$DISKO_DIR" ]; then
    echo "Could not find the disko installation!"
    echo "Did you set PKG_CONFIG_PATH?"
    exit 1;
fi;

if [ ! -x "$TAFF" ]; then
	echo "Could not find TAFF binary at $TAFF!";
	exit 1;
fi;

if [ ! -d "$DIR" ]; then
	echo "Directory $DIR does not exist!";
	exit 1;
fi;

export LD_LIBRARY_PATH="$DISKO_DIR/lib"

# convert dialog and theme files
for file in `find $DIR -name "*.xml" -printf "%p "`; do
    if [ -z "`egrep -i 'mmsdialog|mmstheme' $file`" ]; then
        continue;
    fi;
    $TAFF --warnings no --silent yes --mode c2t --extfile $file --tafffile ${file}.taff;
    if [ $? -eq 0 ]; then
        rm -f $file;
    else
        rm -f ${file}.taff;
		exit 1;
    fi;
done

# convert PNGs
for file in `find $DIR -name "*.png" -printf "%p "`; do
    $TAFF --warnings no --silent yes --mode c2t --exttype image --image:pf ABGR --image:premulti no --extfile $file --tafffile ${file}.taff;
    if [ $? -eq 0 ]; then
		rm -f $file;
    else
        rm -f ${file}.taff;
		exit 1;
    fi;
done
