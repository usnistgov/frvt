#!/bin/bash
success=0;
failure=1;

root=$(pwd)
meds_link="http://nigos.nist.gov:8080/nist/sd/32/NIST_SD32_MEDS-II_face.zip" 
meds_file=$(basename $meds_link)
image_dir=$root/images
########################################
# Clean up all MEDS images and start fresh
########################################
rm -rf $image_dir/S*.ppm $image_dir/*.zip

echo "Downloading MEDS images."
# Download MEDS-II face images
wget $meds_link -P $image_dir;

if [ ! -f $root/images/$meds_file ]; then
	echo "[ERROR] Unable to download $meds_file.  Please check your internet connection and try again."
	exit $failure
fi

echo -n "Checking hash of downloaded zip file "
meds_hash_orig="284e7dfadb7d37e447396b31e98e7ff0"
meds_hash_dl=$(md5sum $image_dir/$meds_file | awk '{ print $1 }')

if [ "$meds_hash_orig" != "$meds_hash_dl" ]; then
	echo "[ERROR] The downloaded MEDS-II zip file hash does not match the expected hash.  Please re-download."
	exit $failure
fi
echo "[SUCCESS]"
########################################
# Unzip downloaded file
########################################
echo -n "Unzipping imagery.  Please wait."
unzip -q $image_dir/$meds_file -d $image_dir/
retcode=$?
if [[ $retcode != 0 ]]; then 
	echo "command: unzip $image_dir/$meds_file failed with exit code: $retcode.  Please ensure you have the unzip utility installed (i.e., sudo yum install -y unzip)"
	exit $failure
fi
echo " [SUCCESS]"
########################################
# Consolidate and convert images to ppm
########################################
mv $image_dir/data/*/*.jpg $image_dir; rm -rf $image_dir/data

# Convert JPG to PPM files
echo -n "Convert JPGs to PPMs.  Please wait."
mogrify -format ppm -strip -path $image_dir/ $image_dir/*.jpg
retcode=$?
if [[ $retcode != 0 ]]; then 
	echo "command: mogrify -format ppm -strip -path $image_dir/ *.jpg failed with exit code: $retcode.  Please ensure you have the mogrify utility installed (i.e., sudo yum install -y ImageMagick)"
	exit $failure
fi
echo " [SUCCESS]"

numImages=$(ls $image_dir/*.ppm | wc -l)
if [ "$numImages" -lt "1308" ]; then
	echo "The directory $image_dir doesn't have the expected 1308 ppm files.  Please re-run this script"
	exit $failure
fi
########################################
# Clean up 
########################################
rm -rf $image_dir/data $image_dir/*.jpg $image_dir/$meds_file

echo "[SUCCESS]:  All validation images were processed successfully."

exit $success;
