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


configDir=config
mkdir -p $configDir
# Configuration directory is READ-ONLY
chmod -R 550 $configDir

outputDir=validation
outputStem=validation
enrollDir=$outputDir/enroll
if [ -d $outputDir ]; then
	chmod -R 777 $outputDir; rm -rf $outputDir 
fi
mkdir -p $enrollDir

root=$(pwd)
libstring=$(ls $root/lib/libfrvt1N_*_?.so)

# Usage: ../bin/validate1N enroll|finalize|search -c configDir -e enrollDir -o outputDir -h outputStem -i inputFile -t numForks
#   enroll|finalize|search: task to process
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
# Enrollment
inputFile=input/enroll.txt
bin/validate1N enroll -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [ $retEnrollment -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem enroll
	# Merge edb and manifest together
	mergeEDB $outputDir
else
	echo "[ERROR] Enrollment validation (single process) failed"
	exit $retEnrollment
fi

rm -rf $outputDir 
mkdir -p $enrollDir

echo -n "Running Enrollment on Multiple Images per Subject (Single Process) "
inputFile=input/enroll_multiface.txt
bin/validate1N enroll -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [ $retEnrollment -eq 0 ]; then
    echo "[SUCCESS]"
    # Merge output files together
    merge $outputDir/$outputStem enroll
    # Merge edb and manifest together
    mergeEDB $outputDir
else
    echo "[ERROR] Enrollment validation (multiple images per subject) failed"
    exit $retEnrollment
fi
rm -rf $outputDir $enrollDir
mkdir -p $enrollDir


# Set number of child processes to fork()
numForks=4

echo -n "Running Enrollment (Multiple Processes) "
# Enrollment
inputFile=input/enroll.txt
bin/validate1N enroll -c $configDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retEnrollment=$?
if [ $retEnrollment -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem enroll
	# Merge edb and manifest together
	mergeEDB $outputDir
	# Change edb and manifest to READ-ONLY
	chmod -R 550 $outputDir/edb $outputDir/manifest
else
	echo "[ERROR] Enrollment validation (multiple processes) failed.  Please ensure your software is compatible with fork(2)."
	exit $retEnrollment
fi

echo -n "Running Finalization "
# Finalization
bin/validate1N finalize -c $configDir -e $enrollDir -o $outputDir -h $outputStem
retFinalize=$?
if [ $retFinalize -eq 0 ]; then
	echo "[SUCCESS]"
else
	echo "[ERROR] Finalize validation failed"
	exit $retFinalize
fi

# Checking that the original EDB and manifest files are still there
# after finalization.  Developers should NOT be modifying or moving
# these files.
if [ ! -s $outputDir/edb ] || [ ! -s $outputDir/manifest ]; then
	echo “[ERROR] $outputDir/edb and/or $outputDir/manifest are missing.  You should not be moving or altering these files!”
	exit 1	
fi

# Changing finalized enrollment directory to READ-ONLY.  Developers only have READ-ONLY access to the
# enrollment directory during search.
chmod -R 550 $enrollDir

echo -n "Running Search (Multiple Processes) "
# Search
inputFile=input/search.txt
bin/validate1N search -c $configDir -e $enrollDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retSearch=$?
if [ $retSearch -eq 0 ]; then
	echo "[SUCCESS]"
	# Merge output files together
	merge $outputDir/$outputStem search
else
	echo "[ERROR] Search (multiple processes) validation failed" 
	exit $retSearch
fi


numForks=1
echo -n "Running Insert/Delete and Search (Single Process)"
# Insert and Delete Templates/Search
inputFile=input/insertAndDelete.txt
bin/validate1N insertAndDelete -c $configDir -e $enrollDir -o $outputDir -h $outputStem -i $inputFile -t $numForks
retInsertAndDelete=$?
if [ $retInsertAndDelete -eq 0 ]; then
	echo "[SUCCESS]"
else
	echo "[ERROR] Insert and Delete validation failed" 
	exit $retInsertAndDelete
fi

echo "------------------------------"
if [ $retEnrollment -eq 0 ] && [ $retFinalize -eq 0 ] && [ $retSearch -eq 0 ] && [ $retInsertAndDelete -eq 0 ]; then
	echo "Validation complete!"
else
	echo "There were errors during validation.  Please ensure you've followed the validation instructions in the README.txt file."
fi

