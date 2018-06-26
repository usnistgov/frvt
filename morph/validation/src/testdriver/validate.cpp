/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <fstream>
#include <iostream>
#include <cstring>
#include <iterator>
#include <sys/wait.h>
#include <unistd.h>

#include "frvt_morph.h"
#include "util.h"

using namespace std;
using namespace FRVT_MORPH;

std::map<std::string, Action> mapStringToAction =
{
    { "detectNonScannedMorph", Action::DetectNonScannedMorph },
    { "detectScannedMorph", Action::DetectScannedMorph },
    { "detectUnknownMorph", Action::DetectUnknownMorph },
    { "detectNonScannedMorphWithProbeImg", Action::DetectNonScannedMorphWithProbeImg },
    { "detectScannedMorphWithProbeImg", Action::DetectScannedMorphWithProbeImg },
    { "detectUnknownMorphWithProbeImg", Action::DetectUnknownMorphWithProbeImg },
    { "compare", Action::Compare },
};

std::map<Action, std::string> mapActionToString =
{
    { Action::DetectNonScannedMorph, "detectNonScannedMorph" },
    { Action::DetectScannedMorph, "detectScannedMorph" },
    { Action::DetectUnknownMorph, "detectUnknownMorph" },
    { Action::DetectNonScannedMorphWithProbeImg, "detectNonScannedMorphWithProbeImg" },
    { Action::DetectScannedMorphWithProbeImg, "detectScannedMorphWithProbeImg" },
    { Action::DetectUnknownMorphWithProbeImg, "detectUnknownMorphWithProbeImg" },
    { Action::Compare, "compare" },
};

int
detectMorph(
        std::shared_ptr<MorphInterface> &implPtr,
        const string &inputFile,
        const string &outputLog,
        Action action)
{
    /* Read input file */
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Failed to open stream for " << inputFile << "." << endl;
        return FAILURE;
    }

    /* Open output log for writing */
    ofstream logStream(outputLog);
    if (!logStream.is_open()) {
        cerr << "Failed to open stream for " << outputLog << "." << endl;
        return FAILURE;
    }

    string line;
    ReturnStatus ret;
    if (action == Action::DetectNonScannedMorph || action == Action::DetectScannedMorph || action == Action::DetectUnknownMorph ) {
        logStream << "image isMorph score returnCode" << endl;
    } else if (action == Action::DetectNonScannedMorphWithProbeImg || action == Action::DetectScannedMorphWithProbeImg) {
        logStream << "image probeImage isMorph score returnCode" << endl;
    }

    while(std::getline(inputStream, line)) {
        Image image, probeImage;
        auto imgs = split(line, ' ');
        if (!readImage(imgs[0], image)) {
            cerr << "Failed to load image file: " << imgs[0] << "." << endl;
            return FAILURE;
        }
        bool isMorph = false;
        double score = -1.0;

        if (action == Action::DetectNonScannedMorph ||
                action == Action::DetectScannedMorph ||
                action == Action::DetectUnknownMorph) {
            ret = implPtr->detectMorph(image, getLabel(action), isMorph, score);
        } else if (action == Action::DetectNonScannedMorphWithProbeImg ||
                action == Action::DetectScannedMorphWithProbeImg ||
                action == Action::DetectUnknownMorphWithProbeImg) {
            if (!readImage(imgs[1], probeImage)) {
                cerr << "Failed to load image file(s): " << imgs[1] << "." << endl;
                return FAILURE;
            }
            ret = implPtr->detectMorphDifferentially(image, getLabel(action), probeImage, isMorph, score);
        }

        /* If function is not implemented, clean up and exit */
        if (ret.code == ReturnCode::NotImplemented) {
            break;
        }

        /* Write template stats to log */
        logStream << imgs[0] << " ";
        if (action == Action::DetectNonScannedMorphWithProbeImg ||
                action == Action::DetectScannedMorphWithProbeImg ||
                action == Action::DetectUnknownMorphWithProbeImg)
            logStream << imgs[1] << " ";

        logStream << isMorph << " "
                << score << " "
                << static_cast<std::underlying_type<ReturnCode>::type>(ret.code)
                << endl;
    }
    inputStream.close();

    /* Remove the input file */
    if( remove(inputFile.c_str()) != 0 )
        cerr << "Error deleting file: " << inputFile << endl;

    if (ret.code == ReturnCode::NotImplemented) {
        /* Remove the output file */
        logStream.close();
        if( remove(outputLog.c_str()) != 0 )
            cerr << "Error deleting file: " << outputLog << endl;
        return NOT_IMPLEMENTED;
    }
    return SUCCESS;
}

int
compare(
        std::shared_ptr<MorphInterface> &implPtr,
        const string &inputFile,
        const string &scoresLog)
{
    /* Read probes */
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Failed to open stream for " << inputFile << "." << endl;
        return FAILURE;
    }

    /* Open scores log for writing */
    ofstream scoresStream(scoresLog);
    if (!scoresStream.is_open()) {
        cerr << "Failed to open stream for " << scoresLog << "." << endl;
        return FAILURE;
    }
    /* header */
    scoresStream << "enrollImage verifImage score returnCode" << endl;

    /* Process each probe */
    string enroll, verif;
    Image enrollImage, verifImage;
    ReturnStatus ret;
    while (inputStream >> enroll >> verif) {
        if (!readImage(enroll, enrollImage)) {
            cerr << "Failed to load image file: " << enroll << "." << endl;
            return FAILURE;
        }
        if (!readImage(verif, verifImage)) {
            cerr << "Failed to load image file: " << verif << "." << endl;
            return FAILURE;
        }

        double similarity = -1.0;
        /* Call compare */
        ret = implPtr->compareImages(enrollImage, verifImage, similarity);

        /* If function is not implemented, clean up and exit */
        if (ret.code == ReturnCode::NotImplemented) {
            break;
        }

        /* Write to scores log file */
        scoresStream << enroll << " "
                << verif << " "
                << similarity << " "
                << static_cast<std::underlying_type<ReturnCode>::type>(ret.code)
                << endl;
    }
    inputStream.close();

    /* Remove the input file */
    if( remove(inputFile.c_str()) != 0 )
        cerr << "Error deleting file: " << inputFile << endl;

    if (ret.code == ReturnCode::NotImplemented) {
        /* Remove the output file */
        scoresStream.close();
        if( remove(scoresLog.c_str()) != 0 )
            cerr << "Error deleting file: " << scoresLog << endl;
        return NOT_IMPLEMENTED;
    }

    return SUCCESS;
}

void usage(const string &executable)
{
    cerr << "Usage: " << executable <<
            " detectNonScannedMorph"
            "|detectScannedMorph"
            "|detectUnknownMorph"
            "|detectNonScannedMorphWithProbeImg"
            "|detectScannedMorphWithProbeImg"
            "|detectUnknownMorphWithProbeImg"
            "|compare -c configDir "
            "-o outputDir -h outputStem -i inputFile -t numForks" << endl;
    exit(EXIT_FAILURE);
}

int
main(
        int argc,
        char* argv[])
{
    auto exitStatus = SUCCESS;
    int requiredArgs = 2; /* exec name and action */
    if (argc < requiredArgs)
        usage(argv[0]);

    string actionstr{argv[1]},
        configDir{"config"},
        outputDir{"output"},
        outputFileStem{"stem"},
        inputFile;
    int numForks = 1;

    for (int i = 0; i < argc - requiredArgs; i++) {
        if (strcmp(argv[requiredArgs+i],"-c") == 0)
            configDir = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-o") == 0)
            outputDir = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-h") == 0)
            outputFileStem = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-i") == 0)
            inputFile = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-t") == 0)
            numForks = atoi(argv[requiredArgs+(++i)]);
        else {
            cerr << "Unrecognized flag: " << argv[requiredArgs+i] << endl;;
            usage(argv[0]);
        }
    }

    Action action = mapStringToAction[actionstr];
    switch (action) {
        case Action::DetectNonScannedMorph:
        case Action::DetectScannedMorph:
        case Action::DetectUnknownMorph:
        case Action::DetectNonScannedMorphWithProbeImg:
        case Action::DetectScannedMorphWithProbeImg:
        case Action::DetectUnknownMorphWithProbeImg:
        case Action::Compare:
            break;
        default:
            cerr << "Unknown command: " << actionstr << endl;
            usage(argv[0]);
    }

    /* Get implementation pointer */
    auto implPtr = MorphInterface::getImplementation();
    /* Initialization */
    auto ret = implPtr->initialize(configDir);
    if (ret.code != ReturnCode::Success) {
        cerr << "initialize() returned error code: "
                << ret.code << "." << endl;
        return FAILURE;
    }

    /* Split input file into appropriate number of splits */
    vector<string> inputFileVector;
    if (splitInputFile(inputFile, outputDir, numForks, inputFileVector) != SUCCESS) {
        cerr << "An error occurred with processing the input file." << endl;
        return FAILURE;
    }

    bool parent = false;
	int i = 0;
    for (auto &inputFile : inputFileVector) {
		/* Fork */
		switch(fork()) {
		case 0: /* Child */
            switch (action) {
                case Action::DetectNonScannedMorph:
                case Action::DetectScannedMorph:
                case Action::DetectUnknownMorph:
                case Action::DetectNonScannedMorphWithProbeImg:
                case Action::DetectScannedMorphWithProbeImg:
                case Action::DetectUnknownMorphWithProbeImg:
                    return detectMorph(
                            implPtr,
                            inputFile,
                            outputDir + "/" + outputFileStem + ".log." + to_string(i),
                            action);
                case Action::Compare:
                    return compare(
                            implPtr,
                            inputFile,
                            outputDir + "/" + outputFileStem + ".log." + to_string(i));
            }
		case -1: /* Error */
			cerr << "Problem forking" << endl;
			break;
		default: /* Parent */
			parent = true;
			break;
		}
		i++;
	}

    /* Parent -- wait for children */
    if (parent) {
        while (numForks > 0) {
            int stat_val;
            pid_t cpid;

            cpid = wait(&stat_val);
            if (WIFEXITED(stat_val)) { exitStatus = WEXITSTATUS(stat_val); }
            else if (WIFSIGNALED(stat_val)) {
                cerr << "PID " << cpid << " exited due to signal " <<
                        WTERMSIG(stat_val) << endl;
                exitStatus = FAILURE;
            } else {
                cerr << "PID " << cpid << " exited with unknown status." << endl;
                exitStatus = FAILURE;
            }
            numForks--;
        }
    }

    return exitStatus;
}
