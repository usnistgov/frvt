#!/bin/bash

success=0
failure=1

bold=$(tput bold)
normal=$(tput sgr0)

# Function to merge output files together
# merge "filename"
function merge() {
        name=$1; shift; suffixes="$*"
        for suffix in $suffixes
        do
                tmp=`dirname $name`
                tmp=$tmp/tmp.txt
                firstfile=`ls ${name}.${suffix}.* | head -n1`
                # Get header
                head -n1 $firstfile > $tmp
                sed -i "1d" ${name}.${suffix}.*
                cat ${name}.${suffix}.* >> $tmp
                mv $tmp ${name}.${suffix}
                rm -rf ${name}.${suffix}.* $tmp
        done
}

configDir=config
configValue=""

if [ ! -e "$configDir" ]; then
	echo "${bold}[ERROR] Missing ./$configDir folder!${normal}"
	exit $failure
fi

outputDir=validation
rm -rf $outputDir; mkdir -p $outputDir

# Usage: ../bin/validate_morph detectNonScannedMorph|detectScannedMorph|detectUnknownMorph|detectNonScannedMorphWithProbeImg|detectScannedMorphWithProbeImg|detectUnknownMorphWithProbeImg|compare -c configDir -o outputDir -i inputFile -t numForks
#   detectScannedMorph ...: task to process
#   configDir: configuration directory
#   configValue: configuration parameter string
#   outputDir: directory where output logs are written to
#   inputFile: input file containing images to process
#   numForks: number of processes to fork
echo "------------------------------"
echo " Running FRVT MORPH Validation"
echo "------------------------------"

# Set number of child processes to fork()
numForks=2

for action in detectNonScannedMorph detectScannedMorph detectUnknownMorph detectNonScannedMorphWithProbeImg detectScannedMorphWithProbeImg detectUnknownMorphWithProbeImg compare
do
	inputFile=input/${action}.txt
	echo -n "Running $action "

	bin/validate_morph $action -c $configDir -v "$configValue" -o $outputDir -i $inputFile -t $numForks -h $action
	retEnroll=$?
	if [[ $retEnroll == 0 ]]; then
		echo "[SUCCESS]"
		# Merge output files together
		merge $outputDir/$action log
	elif [[ $retEnroll == 2 ]]; then
		echo "[NOT IMPLEMENTED]"
	else
		echo "${bold}[ERROR] $action validation failed.  There were errors during validation.  Please investigate and re-run this script.  Please ensure you've followed the validation instructions in the README.txt file.${normal}"
		exit $failure
	fi
done
