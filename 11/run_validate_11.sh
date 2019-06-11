#!/bin/bash
success=0
failure=1

bold=$(tput bold)
normal=$(tput sgr0)

# Check version of OS
reqOS="CentOS Linux release 7.6.1810 (Core) "
currentOS=$(cat /etc/centos-release)
if [ "$reqOS" != "$currentOS" ]; then
	echo "${bold}[ERROR] You are not running the correct version of the operating system, which should be $reqOS.  Please install the correct operating system and re-run this validation package.${normal}"
	exit $failure
fi

# Install the necessary packages to run validation
echo -n "Checking installation of required packages "
for package in coreutils gawk gcc gcc-c++ grep cmake sed bc
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
# Do some sanity checks against the output logs
echo -n "Sanity checking validation output "
for input in enroll verif match
do
	numInputLines=$(cat input/$input.txt | wc -l)
	numLogLines=$(sed '1d' $outputDir/$input.log | wc -l)
	if [ "$numInputLines" != "$numLogLines" ]; then
		echo "[ERROR] The $outputDir/$input.log file has the wrong number of lines.  It should contain $numInputLines but contains $numLogLines.  Please re-run the validation test."
		exit $failure
	fi

	# Check return codes
	numFail=$(sed '1d' $outputDir/$input.log | awk '{ if($4!=0) print }' | wc -l)
	if [ "$numFail" != "0" ]; then
		echo -e "\n${bold}[WARNING] The following entries in $input.log generated non-successful return codes:${normal}"
		sed '1d' $outputDir/$input.log | awk '{ if($4!=0) print }'
	fi

	# Check that at least 50% of match scores are unique
	if [ "$input" == "match" ]; then
		minUniqScores=$(echo "$numInputLines * 0.5" | bc | awk '{printf("%d\n",$1 + 0.5)}')
		numUniqScores=$(sed '1d' $outputDir/$input.log | awk '{ print $3 }' | uniq | wc -l)
		if [ "$numUniqScores" -lt "$minUniqScores" ]; then
			echo -e "\n${bold}[WARNING] Your software produces $numUniqScores unique match scores, which is less than 50% unique match score values.  In order to conduct useful analysis, we highly recommend that you fix your software such that it generates at least 50% unique match scores on this validation set.${normal}"
		fi
	fi	
done
echo "[SUCCESS]"

# Create submission archive
echo -n "Creating submission package "
libstring=$(basename `ls ./lib/libfrvt_11_*_???.so`)
libstring=${libstring%.so}

for directory in config lib validation doc
do
	if [ ! -d "$directory" ]; then
		echo "[ERROR] Could not create submission package.  The $directory directory is missing."
		exit $failure	
	fi
done

tar -zcf $libstring.tar.gz ./config ./lib ./validation ./doc
echo "[SUCCESS]"
echo "
#################################################################################################################
A submission package has been generated named $libstring.tar.gz.  

This archive must be properly encrypted and signed before transmission to NIST.
This must be done according to these instructions - https://www.nist.gov/sites/default/files/nist_encryption.pdf
using the LATEST FRVT Ongoing public key linked from -
https://www.nist.gov/itl/iad/image-group/products-and-services/encrypting-softwaredata-transmission-nist.

For example:
      gpg --default-key <ParticipantEmail> --output <filename>.gpg \\\\
      --encrypt --recipient frvt@nist.gov --sign \\\\
      libfrvt_11_<organization>_<three-digit submission sequence>.tar.gz

Send the encrypted file and your public key to NIST.  You can
      a) Email the files to frvt@nist.gov if your package is less than 20MB OR
      b) Provide a download link from a generic http webserver (NIST will NOT register or establish any kind of
         membership on the provided website) OR
      c) Mail a CD/DVD to NIST at the address provided in the participation agreement
##################################################################################################################
"
