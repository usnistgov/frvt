/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <map>
#include "frvt_morph.h"

#define SUCCESS 0
#define FAILURE 1
#define NOT_IMPLEMENTED 2

/**
 * @brief
 * Task actions used internally by
 * the test harness
 */
enum class Action {
    DetectNonScannedMorph,
    DetectScannedMorph,
    DetectUnknownMorph,
    DetectNonScannedMorphWithProbeImg,
    DetectScannedMorphWithProbeImg,
    DetectUnknownMorphWithProbeImg,
    Compare
};

/** @brief This function returns the image label associated with
 * the Action enum
 *
 * @param[in] action
 * ReturnCode
 *
 * @return
 * Associated image label
 */
FRVT_MORPH::ImageLabel
getLabel(const Action &action);

/** @brief This function reads a PPM file into a FRVT::Image data
 * structure
 *
 * @param[in] file
 * Path to image file
 * @param[out] image
 * The populated FRVT::Image data structure with raw image data
 * and associated metadata
 *
 * @return
 * true if successful; false otherwise
 */
bool
readImage(const std::string &file, FRVT_MORPH::Image &image);

/** @brief This function converts a FRVT::ReturnCode
 * to a readable string
 *
 * @param[in] code
 * ReturnCode
 *
 * @return
 * Readable string
 */
const char*
to_string(FRVT_MORPH::ReturnCode code);


/** @brief This function takes splits an input string
 * based on a provided delimiter
 *
 * @param[in] str
 * String to split
 * @param[in] delimiter
 * Delimiter to split on
 *
 * @return
 * Vector of strings, split on delimiter
 */
std::vector<std::string>
split(
        const std::string &str,
        const char delimiter);

/** @brief This function takes an input file and splits the contents
 * into different files depending on the provided numForks parameter
 *
 * @param[in] inputFile
 * Path to input file
 * @param[in] outputDir
 * Path to the output directory where the split files will be written to
 * @param[in,out] numForks
 * The number of files to split the input file into.
 * @param[out] fileVector
 * A list of file paths to the newly created split files
 *
 * @return
 * EXIT_SUCCESS if successful; EXIT_FAILURE otherwise
 */
int
splitInputFile(
        const std::string &inputFile,
        const std::string &outputDir,
        int &numForks,
        std::vector<std::string> &fileVector);

#endif /* UTIL_H_ */
