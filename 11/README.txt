===============================
FRVT validation package, NIST
===============================
The purpose of this validation package is to 
1) ensure that NIST's execution of your library submission produces the expected output and
2) prepare your submission package to send to NIST

The code provided here is meant only for validation purposes and does not reflect how 
NIST will perform actual testing.  Please note that this validation package must be installed
and run on Centos 7.6, which can be downloaded from 
http://nigos.nist.gov:8080/evaluations/CentOS-7-x86_64-Everything-1810.iso.

===============================
Validation Dataset
===============================
The ../common/images directory will contain all of the images necessary for validation.

NOTE: The validation images are used for the sole purpose of validation and stress-testing
your software.  The images are not necessarily representative of actual test data that will 
be used to evaluate the implementations.  Please do not contact NIST about actual testing
with such validation-type imagery.

===============================
Null Implementation
===============================
There is a null implementation of the FRVT 1:1 API in ./src/nullImpl.  While the 
null implementation doesn't actually provide any real functionality, more importantly, 
it demonstrates mechanically how one could go about implementing, compiling, and building 
a library against the API.

To compile and build the null implementation, from the top level validation directory
run ./scripts/build_null_impl.sh.  This will place the implementation library into ./lib.

===============================
Validation and Submission Preparation
===============================
To successfully complete the validation process, and to prepare your submission package
to send to NIST, please perform the following steps:

1) Put all required configuration files into ./config

2) Put your core implementation library and ALL dependent libraries into ./lib

3) Put a version.txt file into ./doc, which provides version control information for the submission.

4) From the root validation directory, execute the validation script.
   >> ./run_validate_11.sh

The validation script will
- Compile and link your library against the validation test harness. 
- Run the test harness that was built against your library on the validation
  dataset.
- Prepare your submission archive. 

5) Upon successful validation, an archive will be generated named 
   libfrvt_11_<company>_<three-digit submission sequence>.tar.gz

   This archive must be properly encrypted and signed before transmission to NIST.  
   This must be done according to these instructions - https://www.nist.gov/sites/default/files/nist_encryption.pdf
   using the LATEST FRVT Ongoing public key linked from - 
   https://www.nist.gov/itl/iad/image-group/products-and-services/encrypting-softwaredata-transmission-nist. 

   For example:
	gpg --default-key <ParticipantEmail> --output <filename>.gpg \\
	--encrypt --recipient frvt@nist.gov --sign \\
	libfrvt_11_<company>_<three-digit submission sequence>.tar.gz

6) Send the encrypted file and your public key to NIST.  You can
	a) Email the files to frvt@nist.gov if your package is less than 20MB OR
	b) Provide a download link from a generic http webserver (NIST will NOT register or establish any kind of
	   membership on the provided website).  The preferred mechanism is Google Drive.  We do NOT accept
	   Dropbox links. OR
	c) Mail a CD/DVD to NIST at the address provided in the participation agreement

Send any questions or concerns regarding this validation package to frvt@nist.gov.

===============================
Acceptance
===============================
NIST will validate the correct operation of the submissions on our platform
by attempting to duplicate the submitted results using our own test driver
linked with the participant's libraries.  In addition, NIST will perform
a separate timing test to ensure the implementation meets the timing requirements
for certain tasks as detailed in the FRVT Ongoing 1:1 API document.

Any discrepancies will be reported to the participant, and reasonable attempts
will be made to resolve the issue. 

