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
#include "frvt1N.h"

#define SUCCESS 0
#define FAILURE 1

/**
 * @brief
 * Task actions used internally by
 * the test harness
 */
enum class Action {
    Enroll_1N,
    Finalize_1N,
    Search_1N,
    InsertAndDelete
};

/** @brief This function converts a
 * string to an FRVT::Image::Label
 *
 * @param[in] desc
 * description
 *
 * @return
 * FRVT::Image::Label
 */
FRVT::Image::Label
getLabel(const std::string &desc);

/** @brief This function converts an Action
 * to a readable string
 *
 * @param[in] action
 * Action
 *
 * @return
 * Readable string
 */
const char*
to_string(Action action);

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
readImage(const std::string &file, FRVT::Image &image);

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
to_string(FRVT::ReturnCode code);

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
