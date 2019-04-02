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
templatesDir=$outputDir/templates
rm -rf $outputDir 
mkdir -p $templatesDir

# Usage: ../bin/validate11 createTemplate|match [-x enroll|verif] -c configDir -o outputDir -h outputStem -i inputFile -t numForks -j templatesDir
#
#   createTemplate|match: task to process
#		createTemplate: generate templates
#			enroll: generate enrollment templates
#			verif: generate verification templates
#		match: perform matching of templates
#
#   configDir: configuration directory
#   outputDir: directory where output logs are written to
#   outputStem: the string to prefix the output filename(s) with
#   inputFile: input file containing images to process (required for enroll and verif template creation)
#   numForks: number of processes to fork
#   templatesDir: directory where templates are written to/read from
echo "------------------------------"
echo " Running 1:1 validation"
echo "------------------------------"

# Set number of child processes to fork()
numForks=1
inputFile=input/short_enroll.txt
outputStem=enroll

echo -n "Checking for hard-coded config directory "
tempConfigDir=otherConfig
chmod 775 $configDir; mv $configDir $tempConfigDir; chmod 550 $tempConfigDir
bin/validate11 createTemplate -x enroll -c $tempConfigDir -o $outputDir -h $outputStem -i $inputFile -t $numForks -j $templatesDir
retEnroll=$?
if [[ $retEnroll == 0 ]]; then
        echo "[SUCCESS]" 
        # Merge output files together
        merge $outputDir/$outputStem log
else
	chmod 775 $tempConfigDir
	mv $tempConfigDir $configDir
        echo "[ERROR] Detection of hard-coded config directory in your software.  Please fix!"
        exit $failure
fi
rm -rf $outputDir; mkdir -p $templatesDir
chmod 775 $tempConfigDir; mv $tempConfigDir $configDir; chmod 550 $configDir

echo -n "Creating Enrollment Templates (Single Process) "
# Start checking for threading
scripts/count_threads.sh $outputDir/thread.log & pid=$!

inputFile=input/enroll.txt
bin/validate11 createTemplate -x enroll -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks -j $templatesDir
retEnroll=$?

# End checking for threading
kill -9 "$pid"
wait "$pid" 2>/dev/null

if [[ $retEnroll == 0 ]]; then
	echo "[SUCCESS]" 
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Enrollment template creation validation (single process) failed${normal}"
	exit $failure
fi

maxThreads=$(cat $outputDir/thread.log | sort -u -n | tail -n1)
# 1 process for testdriver, 1 process for child
if [ "$maxThreads" -gt "2" ]; then
	echo "${bold}[WARNING] We've detected that your software may be threading or using other multiprocessing techniques during template creation.  The number of threads detected was $maxThreads and it should be 2.  Per the API document, implementations must run single-threaded.  In the test environment, there is no advantage to threading, because NIST will distribute workload across multiple blades and multiple processes.  We highly recommend that you fix this issue prior to submission.${normal}"
fi
rm -rf $outputDir; mkdir -p $templatesDir


numForks=4
echo -n "Creating Enrollment Templates (Multiple Processes) "
bin/validate11 createTemplate -x enroll -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks -j $templatesDir
retEnroll=$?
if [[ $retEnroll == 0 ]]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Enrollment template creation validation (multiple process) failed.  Please ensure your software is compatible with fork(2).${normal}"
	exit $failure
fi

echo -n "Creating Verification Templates (Multiple Processes) "
inputFile=input/verif.txt
outputStem=verif
bin/validate11 createTemplate -x verif -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks -j $templatesDir
retVerif=$?
if [[ $retVerif == 0 ]]; then
	echo "[SUCCESS]" 
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Verification template creation validation failed{$normal}"
	exit $failure
fi

echo -n "Matching Templates (Multiple Processes) "
inputFile=input/match.txt
outputStem=match
bin/validate11 match -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks -j $templatesDir
retMatch=$?
if [[ $retMatch == 0 ]]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem log
else
	echo "${bold}[ERROR] Match validation failed${normal}"
	exit $failure
fi

if [[ $retEnroll != 0 ]] || [[ $retVerif != 0 ]] || [[ $retMatch != 0 ]]; then
	echo "${bold}There were errors during validation.  Please investigate and re-run this script.  Please ensure you've followed the validation instructions in the README.txt file.${normal}"
fi

