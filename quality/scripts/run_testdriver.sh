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
                rm -rf ${name}.${suffix}.*
        done
}

# Make sure there aren't any zombie processes
# left over from previous validation run
kill -9 $(ps -aef | grep "count_thread" | awk '{ print $2 }') 2> /dev/null

configDir=config
if [ ! -e "$configDir" ]; then
	echo "${bold}[ERROR] Missing ./$configDir folder!${normal}"
	exit $failure	
fi

outputDir=validation
rm -rf $outputDir; mkdir -p $outputDir

# Usage: ../bin/validate_quality -c configDir -o outputDir -h outputStem -i inputFile -t numForks
#
#   configDir: configuration directory
#   outputDir: directory where output logs are written to
#   outputStem: the string to prefix the output filename(s) with
#   inputFile: input file containing images to process (required for enroll and verif template creation)
#   numForks: number of processes to fork

echo "------------------------------"
echo " Running FRVT Quality Validation"
echo "------------------------------"

# Set number of child processes to fork()
numForks=1
inputFile=input/short_quality.txt
outputStem=quality

echo -n "Checking for hard-coded config directory "
tempConfigDir=otherConfig
chmod 775 $configDir; mv $configDir $tempConfigDir; chmod 550 $tempConfigDir
bin/validate_quality -c $tempConfigDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
ret=$?
if [[ $ret == 0 ]]; then
        echo "[SUCCESS]" 
        # Merge output files together
        merge $outputDir/$outputStem log
else
	chmod 775 $tempConfigDir
	mv $tempConfigDir $configDir
	echo "[ERROR] Detection of hard-coded config directory in your software.  Please fix!"
	exit $failure
fi
rm -rf $outputDir; mkdir -p $outputDir
chmod 775 $tempConfigDir; mv $tempConfigDir $configDir; chmod 550 $configDir

echo -n "Generating Quality Values (Single Process) "
# Start checking for threading
scripts/count_threads.sh $outputDir/thread.log & pid=$!

inputFile=input/quality.txt
bin/validate_quality -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
ret=$?

# End checking for threading
kill -9 "$pid"
wait "$pid" 2>/dev/null

if [[ $ret == 0 ]]; then
	echo "[SUCCESS]" 
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Quality validation (single process) failed${normal}"
	exit $failure
fi

maxThreads=$(cat $outputDir/thread.log | sort -u -n | tail -n1)
# 1 process for testdriver, 1 process for child
if [ "$maxThreads" -gt "2" ]; then
	echo "${bold}[WARNING] We've detected that your software may be threading or using other multiprocessing techniques.  The number of processes detected was $maxThreads and it should be 2.  Per the API document, implementations must run single-threaded.  In the test environment, there is no advantage to threading, because NIST will distribute workload across multiple blades and multiple processes.  We highly recommend that you fix this issue prior to submission.${normal}"
fi
rm -rf $outputDir; mkdir -p $outputDir


numForks=4
echo -n "Generating Quality Values (Multiple Processes) "
bin/validate_quality -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
ret=$?
if [[ $ret == 0 ]]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Quality validation (multiple process) failed.  Please ensure your software is compatible with fork(2).${normal}"
	exit $failure
fi

if [[ $ret != 0 ]]; then
	echo "${bold}There were errors during validation.  Please investigate and re-run this script.  Please ensure you've followed the validation instructions in the README.txt file.${normal}"
fi

