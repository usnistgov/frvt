#!/bin/bash
success=0
failure=1

bold=$(tput bold)
normal=$(tput sgr0)

# Check version of OS
reqOS="CentOS Linux release 8.2.2004 (Core) "
currentOS=$(cat /etc/centos-release)
if [ "$reqOS" != "$currentOS" ]; then
	echo "${bold}[ERROR] You are not running the correct version of the operating system, which should be $reqOS.  Please install the correct operating system and re-run this validation package.${normal}"
	exit $failure
fi

# Install the necessary packages to run validation
echo -n "Checking installation of required packages "
for package in coreutils gawk gcc gcc-c++ grep cmake make sed
do
	yum -q list installed $package &> /dev/null
	retcode=$?
	if [[ $retcode != 0 ]]; then
		sudo yum install -y $package
	fi
done
echo "[SUCCESS]"

# Check for ./doc/version.txt
if [ ! -s "./doc/version.txt" ]; then
	echo "[ERROR] ./doc/version.txt was not found.  Per the API document, ./doc/version.txt must document versioning information for the submitted software."
	echo "Please fix this and re-run."
	exit $failure
fi

# Compile and build implementation library against
# validation test driver
scripts/compile_and_link.sh
retcode=$?
if [[ $retcode != 0 ]]; then
	exit $failure
fi

# Run testdriver against linked library
# and validation images
scripts/run_testdriver.sh
retcode=$?
if [[ $retcode != 0 ]]; then
	exit $failure
fi

outputDir="validation"
stem="validation"
# Do some sanity checks against the output logs
echo -n "Sanity checking validation output "
for input in enroll_1N search_1N insert
do
	numInputLines=$(cat input/$input.txt | wc -l)
	numLogLines=$(cat $outputDir/$stem.$input | wc -l)
	if [ "$input" == "search_1N" ]; then
		# Number of searches * 20 candidates each + header
		numExpectedLines=$((($numInputLines * 20) + 1))
	elif [ "$input" == "enroll_1N" ]; then
		# Number of enrollments + header
		numExpectedLines=$(($numInputLines + 1))
	elif [ "$input" == "insert" ]; then
		# (N-1) searches (for insert) * 20 candidates each + header
		numExpectedLines=$(((($numInputLines - 1) * 20) + 1))
	fi

	# Check the output logs have the right number of lines
	if [ "$numLogLines" -ne "$numExpectedLines" ]; then
		echo "[ERROR] The $outputDir/$stem.$input file has the wrong number of lines.  It should contain $numExpectedLines but contains $numLogLines.  Please re-run the validation test."
		exit $failure
	fi	

	# Check template generation and search return codes
	if [ "$input" == "enroll_1N" ]; then
		numFail=$(sed '1d' $outputDir/$stem.$input | awk '{ if($4!=0) print }' | wc -l)
		if [ "$numFail" -ne "0" ]; then
			sed '1d' $outputDir/$stem.$input | awk '{ if($4!=0) print }'
		fi
	elif [ "$input" == "search_1N" ] || [ "$input" == "insert" ]; then
		numFail=$(sed '1d' $outputDir/$stem.$input | awk '{ if($3!=0) print }' | wc -l)
		if [ "$numFail" -ne "0" ]; then
			sed '1d' $outputDir/$stem.$input | awk '{ if($3!=0) print }'
		fi
	fi
done
echo "[SUCCESS]"

# Create submission archive
echo -n "Creating submission package "
libstring=$(basename `ls ./lib/libfrvt_1N_*_???.so`)
libstring=${libstring%.so}

for directory in config lib validation doc
do
        if [ ! -d "$directory" ]; then
                echo "[ERROR] Could not create submission package.  The $directory directory is missing."
                exit $failure
        fi
done

# write OS to text file
cat /etc/redhat-release > validation/os.txt

tar -zcf $libstring.tar.gz ./doc ./config ./lib ./validation
echo "[SUCCESS]"
echo "
#################################################################################################################
A submission package has been generated named $libstring.tar.gz.  

This archive must be properly encrypted and signed before transmission to NIST.
This must be done according to these instructions - https://www.nist.gov/sites/default/files/nist_encryption.pdf
using the FRVT public key linked from -
https://www.nist.gov/itl/iad/image-group/products-and-services/encrypting-softwaredata-transmission-nist.

For example:
      gpg --default-key <ParticipantEmail> --output <filename>.gpg \\\\
      --encrypt --recipient frvt@nist.gov --sign \\\\
      libfrvt_1N_<company>_<three-digit submission sequence>.tar.gz

Send the encrypted file and your public key to NIST.  You can
      a) Email the files to frvt@nist.gov if your package is less than 20MB OR
      b) Provide a download link from a generic http webserver (NIST will NOT register or establish any kind of
         membership on the provided website) OR
      c) Mail a CD/DVD to NIST at the address provided in the participation agreement
##################################################################################################################
"
