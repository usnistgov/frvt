#!/bin/bash

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
mkdir -p $configDir

outputDir=validation
rm -rf $outputDir; mkdir $outputDir

# Usage: ../bin/validate detectSingleMorph|detectScannedMorph|detectMorphWithLiveImg|match -c configDir -o outputDir -i inputFile -t numForks
#   detectSingleMorph|detectScannedMorph|detectMorphWithLiveImg|match: task to process
#   configDir: configuration directory
#   outputDir: directory where output logs are written to
#   inputFile: input file containing images to process
#   numForks: number of processes to fork
echo "------------------------------"
echo " Running morph detection validation"
echo "------------------------------"

# Set number of child processes to fork()
numForks=2

for action in detectSingleMorph detectScannedMorph detectMorphWithLiveImg match
do
	inputFile=input/${action}.txt
	echo -n "Running $action "

	bin/validate $action -c $configDir -o $outputDir -i $inputFile -t $numForks -h $action
	retEnroll=$?
	if [[ $retEnroll == 0 ]]; then
		echo "[SUCCESS]"
		#Merge output files together
		merge $outputDir/$action log
	elif [[ $retEnroll == 2 ]]; then
		echo "[NOT IMPLEMENTED]"
	else
		echo "[ERROR] $action validation failed.  There were errors during validation.  Please investigate and re-run this script.  Please ensure you've followed the validation instructions in the README.txt file."
		exit
	fi
done


