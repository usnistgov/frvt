/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 **/

#include <algorithm>
#include <limits>
#include <fstream>

#include "util.h"

using namespace std;
using namespace FRVT;

static const string inputFileStem = "input.txt.";

Image::Label getLabel(const std::string &desc)
{
    if(desc == "UNKNOWN") return Image::Label::UNKNOWN;
    else if(desc == "ISO") return Image::Label::ISO;
    else if(desc == "MUGSHOT") return Image::Label::MUGSHOT;
    else if(desc == "PHOTOJOURNALISM") return Image::Label::PHOTOJOURNALISM;
    else if(desc == "EXPLOITATION") return Image::Label::EXPLOITATION;
    else if(desc == "WILD") return Image::Label::WILD;
    throw runtime_error("Unknown Image::Label value: " + desc);
}

const char*
to_string(ReturnCode code)
{
    switch (code) {
    case ReturnCode::Success: return "Success";
    case ReturnCode::ConfigError: return "ConfigError";
    case ReturnCode::RefuseInput: return "RefuseInput";
    case ReturnCode::ExtractError: return "ExtractError";
    case ReturnCode::ParseError: return "ParseError";
    case ReturnCode::TemplateCreationError: return "TemplateCreationError";
    case ReturnCode::VerifTemplateError: return "VerifTemplateError";
    case ReturnCode::FaceDetectionError: return "FaceDetectionError";
    case ReturnCode::NumDataError: return "NumDataError";
    case ReturnCode::TemplateFormatError: return "TemplateFormatError";
    case ReturnCode::EnrollDirError: return "EnrollDirError";
    case ReturnCode::InputLocationError: return "InputLocationError";
    case ReturnCode::MemoryError: return "MemoryError";
    case ReturnCode::NotImplemented: return "NotImplemented";
    case ReturnCode::VendorError: return "VendorError";
    default: return "Unknown ReturnCode";
    }
}

const char*
to_string(Action action)
{
    switch (action) {
    case Action::Enroll_1N: return "enroll";
    case Action::Finalize_1N: return "finalize";
    case Action::Search_1N: return "search";
    case Action::InsertAndDelete: return "insertAndDelete";
    default: return "Unknown Action";
    }
}

/**
 * Reads a PPM file into an Image object.
 * This function isn't intended to fully support the PPM format, only enough
 * to read the validation images.
 */
bool
readImage(
        const string &file,
        Image &image)
{
    /* Open PPM file. */
    ifstream input(file, ios::binary);
    if (!input.is_open()) {
        cerr << "Cannot open the input file " << file << endl;
        return false;
    }

    /* Read in magic number. */
    string magicNumber;
    input >> magicNumber;
    if (magicNumber != "P6") {
        cerr << "Error reading magic number from file." << endl;
        return false;
    }

    uint16_t maxValue;
    /* Read in image width, height, and max intensity value. */
    input >> image.width >> image.height >> maxValue;
    if (!input.good()) {
        cerr << "Error, premature end of file while reading header." << endl;
        return false;
    }
    image.depth = 24;

    /* Skip line break. */
    input.ignore(numeric_limits<streamsize>::max(), '\n');

    uint8_t *data = new uint8_t[image.size()];
    image.data.reset(data, std::default_delete<uint8_t[]>());

    /* Read in raw pixel data. */
    input.read((char*)image.data.get(), image.size());
    if (!input.good()) {
        cerr << "Error, only read " << input.gcount() << " bytes." << endl;
        return false;
    }
    return true;
}

int
splitInputFile(
        const string &inputFile,
        const string &outputDir,
        int &numForks,
        vector<string> &fileVector)
{
    /* Read input file */
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Failed to open stream for " << inputFile << "." << endl;
        return FAILURE;
    }

    /* Count number of lines in file */
    auto numLines = count(
            istreambuf_iterator<char>(inputStream),
            istreambuf_iterator<char>(), '\n');

    inputStream.clear();
    inputStream.seekg(0, ios::beg);

    /**
     * If the number of processes to fork is more than
     * the number of lines in the file, set numForks
     * to numLines
     */
    if (numLines < (uint32_t)numForks)
        numForks = numLines;
    /* This is the round-up of numLines/numForks */
    int numLinesPerFork = numLines / numForks + (numLines
            % numForks > 0 ? 1 : 0);
    /* re-assign the number of forks if the numbers don't work out */
    numForks = (numLines/numLinesPerFork) + ((numLines
            % numLinesPerFork) > 0 ? 1 : 0);

    for (int i = 0; i < numForks; i++) {
        string filepath = outputDir + "/" + inputFileStem + to_string(i);
        ofstream outputStream(filepath);
        if (!outputStream.is_open()) {
            cerr << "Failed to open stream for " << filepath << "." << endl;
            return FAILURE;
        }
        fileVector.push_back(filepath);

        for (int line_num = 1; line_num < numLinesPerFork + 1; line_num++) {
            string line;
            if (getline(inputStream, line))
                outputStream << line << endl;
            else
                break;
        }
    }

    return SUCCESS;
}
