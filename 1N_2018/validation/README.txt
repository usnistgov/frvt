===============================
FRVT 1:N validation package, NIST
===============================
The purpose of this validation package is to 
1) ensure that NIST's execution of your library submission produces the expected output and
2) prepare your submission package to send to NIST

The code provided here is meant only for validation purposes and does not reflect how 
NIST will perform actual testing.  Please note that this validation package must be installed
and run on Centos 7.2, which can be downloaded from 
http://nigos.nist.gov:8080/evaluations/CentOS-7-x86_64-Everything-1511.iso. 

===============================
Null Implementation
===============================
There is a null implementation of the FRVT 1:N API in ./src/nullImpl.  The null implementation has 
no real functionality but demonstrates mechanically how one could go about implementing the FRVT 1:N API.

===============================
Validation Dataset
===============================
The ./images directory will contain a few existing images, and the validation script will 
download and prepare the remainder of the images required for validation. 
Your machine must have access to the Internet in order to download the images.  

NOTE: The validation images are used for the sole purpose of validation/stress-testing the algorithm 
and are not necessarily representative of actual test data that will be used to evaluate the implementations.

===============================
Validation and Submission Preparation
===============================
To successfully complete the validation process, and to prepare your submission package
to send to NIST, please perform the following steps:
0) Ensure yum is installed on your system.  Yum is required, because it will 
   download and install all packages required for validation.

1) Put all required configuration files into ./config

2) Put your core implementation library and ALL dependent libraries into ./lib.

3) From the root validation directory, execute the validation script that corresponds
   to your submission.  
   >> ./run_validation_1N.sh

The validation script will
- Install required packages that don't already exist on your system.  You will need 
  sudo rights and connection to the Internet for this.
- Download and prepare the validation dataset if it hasn't already been done.  
  This also requires connection to the Internet.
- Compile and link your library against the validation test harness. 
- Run the test harness that was built against your library on the validation
  dataset.
- Prepare your submission archive. 

4) Upon successful validation, an archive will be generated and named
   libfrvt1N_<company>_<one-digit submission sequence>.tar.gz

   This archive must be properly encrypted and signed before transmission to NIST.  
   This must be done according to these instructions - https://www.nist.gov/sites/default/files/nist_encryption.pdf
   using the LATEST FRVT 1:N Ongoing public key linked from - 
   https://www.nist.gov/itl/iad/image-group/products-and-services/encrypting-softwaredata-transmission-nist. 

   For example:
	gpg --default-key <ParticipantEmail> --output <filename>.gpg \\
	--encrypt --recipient frvt@nist.gov --sign \\
	libfrvt1N_<company>_<one-digit submission sequence>.tar.gz

5) Send the encrypted file and your public key to NIST.  You can
	a) Email the files to frvt@nist.gov if your package is less than 20MB OR
	b) Provide a download link from a generic http webserver (NIST will NOT register or establish any kind of
	   membership on the provided website) OR
	c) Mail a CD/DVD to NIST at the address provided in the participation agreement

Send any questions or concerns regarding this validation package to frvt@nist.gov.

===============================
Acceptance
===============================
NIST will validate the correct operation of the submissions on our platform
by attempting to duplicate the submitted results using our own test driver
linked with the participant's libraries.  In addition, NIST will perform
a separate timing test to ensure the implementation meets the timing requirements
for certain tasks as detailed in the FRVT 1:N API document.

Any discrepancies will be reported to the participant, and reasonable attempts
will be made to resolve the issue. 

