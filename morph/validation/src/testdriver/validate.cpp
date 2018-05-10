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
    { "detectSingleMorph", Action::DetectSingleMorph },
    { "detectScannedMorph", Action::DetectScannedMorph },
    { "detectMorphWithLiveImg", Action::DetectMorphWithLiveImg },
    { "match", Action::Match },
};

std::map<Action, std::string> mapActionToString =
{
    { Action::DetectSingleMorph, "detectSingleMorph" },
    { Action::DetectScannedMorph, "detectScannedMorph" },
    { Action::DetectMorphWithLiveImg, "detectMorphWithLiveImg" },
    { Action::Match, "match" },
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
    if (action == Action::DetectSingleMorph || action == Action::DetectScannedMorph) {
        logStream << "image isMorph score returnCode" << endl;
    } else if (action == Action::DetectMorphWithLiveImg) {
        logStream << "image liveImage isMorph score returnCode" << endl;
    }

    while(std::getline(inputStream, line)) {
        Image image, liveImage;
        auto imgs = split(line, ' ');
        if (!readImage(imgs[0], image)) {
            cerr << "Failed to load image file: " << imgs[0] << "." << endl;
            return FAILURE;
        }
        bool isMorph = false;
        double score = -1.0;

        if (action == Action::DetectSingleMorph) {
            ret = implPtr->detectMorph(image, isMorph, score);
        } else if (action == Action::DetectScannedMorph) {
            ret = implPtr->detectScannedMorph(image, isMorph, score);
        } else if (action == Action::DetectMorphWithLiveImg) {
            if (!readImage(imgs[1], liveImage)) {
                cerr << "Failed to load image file(s): " << imgs[1] << "." << endl;
                return FAILURE;
            }
            ret = implPtr->detectMorph(image, liveImage, isMorph, score);
        }

        /* If function is not implemented, clean up and exit */
        if (ret.code == ReturnCode::NotImplemented) {
            break;
        }

        /* Write template stats to log */
        logStream << imgs[0] << " ";
        if (action == Action::DetectMorphWithLiveImg)
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
match(
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
        /* Call match */
        ret = implPtr->matchImages(enrollImage, verifImage, similarity);

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
    cerr << "Usage: " << executable << " detectSingleMorph|detectScannedMorph|detectMorphWithLiveImg|match -c configDir "
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
        case Action::DetectSingleMorph:
        case Action::DetectScannedMorph:
        case Action::DetectMorphWithLiveImg:
        case Action::Match:
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
                case Action::DetectSingleMorph:
                case Action::DetectScannedMorph:
                case Action::DetectMorphWithLiveImg:
                    return detectMorph(
                            implPtr,
                            inputFile,
                            outputDir + "/" + outputFileStem + ".log." + to_string(i),
                            action);
                case Action::Match:
                    return match(
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
