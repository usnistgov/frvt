#!/bin/bash
success=0
failure=1

FRVTLIB="libfrvt1N_<company>_<one digit sequence>.so"
root=$(pwd)
approot=$root/bin
libroot=$root/lib

echo -n "Looking for core implementation library in $libroot."
# Get libraries into a string to
# pass into Makefile
numLibs=$(ls $libroot/libfrvt1N_*_?.so | wc -l)
if [ $numLibs -eq 0 ]; then
	echo "[ERROR] Could not find core implementation library in $libroot.  Please make sure your core implementation library is named according to the required naming convention - $FRVTLIB and placed in $libroot."
	exit $failure
elif [ $numLibs -gt 1 ]; then
	echo "[ERROR] There are more than one libraries in $libroot that matches the core implementation library naming convention - $FRVTLIB.  There should only be ONE core submission library."
	exit $failure
fi

libstring=$(ls $libroot/libfrvt1N_*_?.so)
echo "[SUCCESS] Found core implementation library $libstring."

# We need to set this env variable that gets passed
# into cmake for it to link to
FRVT_IMPL_LIB=$libstring
export FRVT_IMPL_LIB

echo "Attempting to compile and link $libstring against test harness."
rm -rf build; mkdir -p build; cd build
cmake ../ > /dev/null; make
cd $root;
if [ ! -f "bin/validate1N" ]; then
	echo "[ERROR] There were errors during compilation of your library with the validation test harness.  Please investigate and re-compile."
	exit $failure
fi
echo "[SUCCESS] Built executable in $approot."

# Check for avx instructions and throw up a warning
# if detected.  The submission should run on CPUs
# that do not have avx instructions, per the API document
avxOutput=$(nm $libroot/libfrvt1N_*_?.so | grep avx)

if [ "$avxOutput" != "" ]; then
	echo "[WARNING] avx instructions were detected in your library!  Please verify that your library operates on machines that do not support avx instructions."
fi

exit $success
