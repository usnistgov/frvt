#!/bin/bash

success=0
failure=1

bold=$(tput bold)
normal=$(tput sgr0)

reqOS="Ubuntu 20.04"

# Function to check version of OS
check_os() {
    currentOS=$(lsb_release -a 2> /dev/null | grep "Description" | awk -F":" '{ print $2 }' | sed -e 's/^[[:space:]]*//')
    if [[ $currentOS != $reqOS* ]]; then
        echo "[ERROR] You are not running the correct version of the operating system. You should be running Ubuntu 20.04.x. Please install the correct operating system and re-run this validation package."
        exit 1
    fi
}

# Function to check and install the necessary 
# packages to run validation
check_packages() {
    echo -n "Checking installation of required packages "
    for package in g++ cmake sed bc gawk grep
    do
        if [ $(dpkg-query -W -f='${Status}' $package 2>/dev/null | grep -c "ok installed") -eq 0 ];
    then
            if [ "$package" == "g++" ]; then
                package="g++=4:9.3.0-1ubuntu2"
            fi
            sudo apt-get install -y $package;
        fi
    done
    echo "[SUCCESS]"
}

# Function to check for the existence of
# required folders
check_folders() {
    if [ ! -d "./config" ]; then
        echo "[ERROR] ./config was not found.  Please fix this and re-run."
        exit $failure
    fi

    if [ ! -d "./lib" ]; then
        echo "[ERROR] ./lib was not found.  Please fix this and re-run."
        exit $failure
    else
        if [ ! "$(ls -A ./lib)" ]; then
            echo "[ERROR] The ./lib directory is empty.  Please place your software library files in ./lib and re-run."
            exit $failure
        fi
    fi

    if [ ! -s "./doc/version.txt" ]; then
        echo "[ERROR] ./doc/version.txt was not found.  Per the API document, ./doc/version.txt must document versioning information for the submitted software."
        echo "Please fix this and re-run."
        exit $failure
    fi
}

# Function to log OS info to text file
log_os() {
    echo "$reqOS" > validation/os.txt    
}

# Function to get current frvt_structs.h version
get_frvt_header_version() {
    major=$(grep "FRVT_STRUCTS_MAJOR_VERSION{" ../common/src/include/frvt_structs.h | awk -F'{' '{ print $2 }' | awk -F'}' '{ print $1 }')
    minor=$(grep "FRVT_STRUCTS_MINOR_VERSION{" ../common/src/include/frvt_structs.h | awk -F'{' '{ print $2 }' | awk -F'}' '{ print $1 }')
    echo "${major}.${minor}"
}

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
