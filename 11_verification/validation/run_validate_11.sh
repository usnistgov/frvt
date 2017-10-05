#!/bin/bash
success=0
failure=1

# Download required packages if needed
# Development Tools, ImageMagick, CMake, unzip
echo -n "Checking installation of required packages "
for package in coreutils wget gawk gcc gcc-c++ grep cmake sed ImageMagick unzip
do
	yum -q list installed $package &> /dev/null
	retcode=$?
	if [[ $retcode != 0 ]]; then
		sudo yum install -y $package
	fi	
done
echo "[SUCCESS]"
# Download and process validation images
# Check if the images have already been downloaded first
numImages=$(ls images/*.ppm | wc -l)
if [ "$numImages" -ne "1308" ]; then
	scripts/get_images.sh
	retcode=$?
	if [ $retcode -ne 0 ]; then
		exit $failure
	fi
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

	# Check template generation return codes
	numFail=$(sed '1d' $outputDir/$input.log | awk '{ if($4!=0) print }' | wc -l)
	if [ "$numFail" != "0" ]; then
		echo -e "\n[WARNING] The following entries in $input.log generated non-successful return codes:"
		sed '1d' $outputDir/$input.log | awk '{ if($4!=0) print }'
	fi
done
echo "[SUCCESS]"

# Create submission archive
echo -n "Creating submission package "
libstring=$(basename `ls ./lib/libfrvt11_*_???_[cg]pu.so`)
libstring=${libstring%.so}
tar -zcf $libstring.tar.gz ./config ./lib ./validation
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
      libfrvt11_<company>_<three-digit submission sequence>_<cpu|gpu>.tar.gz

Send the encrypted file and your public key to NIST.  You can
      a) Email the files to frvt@nist.gov if your package is less than 20MB OR
      b) Provide a download link from a generic http webserver (NIST will NOT register or establish any kind of
         membership on the provided website) OR
      c) Mail a CD/DVD to NIST at the address provided in the participation agreement
##################################################################################################################
"
