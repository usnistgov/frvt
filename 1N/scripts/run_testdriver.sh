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

# Function to merge edb and manifest files
# together
function mergeEDB() {
	dir=$1
	edb=$dir/tmp.edb
	manifest=$dir/tmp.manifest
	currOffset=0
	for f in $dir/manifest.*
	do
		seq=`basename $f | awk -F"." '{ print $2 }'`
		while read line
		do
			id=`echo $line | awk '{ print $1 }'`
			size=`echo $line | awk '{ print $2 }'`
			echo "$id $size $currOffset" >> $manifest
			currOffset=$((currOffset+size))
		done < $f	
		cat $dir/edb.$seq >> $edb
	done
	mv $edb $dir/edb
	mv $manifest $dir/manifest

	# Clean up old files	
	rm -rf $dir/edb.* $dir/manifest.*
}

# Make sure there aren't any zombie processes
# left over from previous validation run
kill -9 $(ps -aef | grep "count_thread" | awk '{ print $2 }') 2> /dev/null

configDir=config
if [ ! -e "$configDir" ]; then
	echo "${bold}[ERROR] Missing ./$configDir folder!${normal}"
	exit $failure
fi
# Configuration directory is READ-ONLY
chmod -R 550 $configDir

outputDir=validation
outputStem=validation
enrollDir=$outputDir/enroll
if [ -d $outputDir ]; then
	chmod -R 777 $outputDir; rm -rf $outputDir 
fi
mkdir -p $enrollDir

# Usage: ../bin/validate1N enroll_1N|finalize_1N|search_1N|insert -c configDir -e enrollDir -o outputDir -h outputStem -i inputFile -t numForks
#   enroll_1N|finalize_1N|search_1N|insert: task to process
#   configDir: configuration directory
#   enrollDir: enrollment directory
#   outputDir: directory where output logs are written to
#   outputStem: the string to prefix the output filename(s) with
#   inputFile: input file containing images to process (required for enroll and search tasks)
#   numForks: number of processes to fork.
echo "------------------------------"
echo " Running 1:N validation"
echo "------------------------------"

# Set number of child processes to fork()
numForks=1

echo -n "Running Enrollment (Single Process) "
# Start checking for threading
scripts/count_threads.sh $outputDir/thread.log & pid=$!

# Enrollment
inputFile=input/enroll_1N.txt
bin/validate1N enroll_1N -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?

# End checking for threading
kill -9 "$pid"
wait "$pid" 2>/dev/null

if [ $retEnrollment -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem enroll_1N
	# Merge edb and manifest together
	mergeEDB $outputDir
else
	echo "${bold}[ERROR] Enrollment validation (single process) failed${normal}"
	exit $failure
fi

maxThreads=$(cat $outputDir/thread.log | sort -u -n | tail -n1)
# 1 process for testdriver, 1 process for child
if [ "$maxThreads" -gt "2" ]; then
	echo "${bold}[WARNING] We've detected that your software may be threading or using other multiprocessing techniques during template creation.  The number of threads detected was $maxThreads and it should be 2.  Per the API document, implementations must run single-threaded.  In the test environment, there is no advantage to threading, because NIST will distribute workload across multiple blades and multiple processes.  We highly recommend that you fix this issue prior to submission.${normal}"
fi
rm -rf $outputDir; mkdir -p $enrollDir

echo -n "Checking for hard-coded config directory "
inputFile=input/enroll_1N_short.txt
tempConfigDir=otherConfig
chmod 775 $configDir; mv $configDir $tempConfigDir; chmod 550 $tempConfigDir
bin/validate1N enroll_1N -c $tempConfigDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [[ $retEnrollment == 0 ]]; then
        echo "[SUCCESS]"
else
	chmod 775 $tempConfigDir
	mv $tempConfigDir $configDir
        echo "[ERROR] Detection of hard-coded config directory in your software.  Please fix!"
        exit $failure
fi
rm -rf $outputDir; mkdir -p $enrollDir
chmod 775 $tempConfigDir; mv $tempConfigDir $configDir; chmod 550 $configDir

echo -n "Running Enrollment on Multiple Images per Subject (Single Process) "
inputFile=input/enroll_1N_multiface.txt
bin/validate1N enroll_1N -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [ $retEnrollment -eq 0 ]; then
    echo "[SUCCESS]"
    # Merge output files together
    merge $outputDir/$outputStem enroll_1N
    # Merge edb and manifest together
    mergeEDB $outputDir
else
    echo "${bold}[ERROR] Enrollment validation (multiple images per subject) failed${normal}"
    exit $failure
fi
rm -rf $outputDir $enrollDir
mkdir -p $enrollDir


# Set number of child processes to fork()
numForks=4

echo -n "Running Enrollment (Multiple Processes) "
# Enrollment
inputFile=input/enroll_1N.txt
bin/validate1N enroll_1N -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [ $retEnrollment -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem enroll_1N
	# Merge edb and manifest together
	mergeEDB $outputDir
	# Change edb and manifest to READ-ONLY
	chmod -R 550 $outputDir/edb $outputDir/manifest
else
	echo "${bold}[ERROR] Enrollment validation (multiple processes) failed.  Please ensure your software is compatible with fork(2).${normal}"
	exit $failure
fi

echo -n "Running Finalization "
# Finalization
bin/validate1N finalize_1N -c $configDir -e $enrollDir -o $outputDir -h $outputStem
retFinalize=$?
if [ $retFinalize -eq 0 ]; then
	echo "[SUCCESS]"
else
	echo "${bold}[ERROR] Finalize validation failed${normal}"
	exit $failure
fi

# Checking that the original EDB and manifest files are still there
# after finalization.  Developers should NOT be modifying or moving
# these files.
if [ ! -s $outputDir/edb ] || [ ! -s $outputDir/manifest ]; then
	echo "${bold}[ERROR] $outputDir/edb and/or $outputDir/manifest are missing.  You should not be moving or altering these files!${normal}"
	exit $failure
fi

# Changing finalized enrollment directory to READ-ONLY.  Developers only have READ-ONLY access to the
# enrollment directory during search.
chmod -R 550 $enrollDir

echo -n "Running Search (Multiple Processes) "
# Search
inputFile=input/search_1N.txt
bin/validate1N search_1N -c $configDir -e $enrollDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retSearch=$?
if [ $retSearch -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem search_1N
else
	echo "${bold}[ERROR] Search (multiple processes) validation failed${normal}"
	exit $failure
fi

numForks=1
echo -n "Running Insert Search (Single Process)"
# Insert Templates/Search
inputFile=input/insert.txt
bin/validate1N insert -c $configDir -e $enrollDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retInsert=$?
if [ $retInsert -eq 0 ]; then
	echo "[SUCCESS]"
else
	echo "${bold}[ERROR] Insert validation failed${normal}"
	exit $failure
fi

echo "------------------------------"
if [ $retEnrollment -eq 0 ] && [ $retFinalize -eq 0 ] && [ $retSearch -eq 0 ] && [ $retInsert -eq 0 ]; then
	echo "Validation complete!"
else
	echo "There were errors during validation.  Please ensure you've followed the validation instructions in the README.txt file."
fi

