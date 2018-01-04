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
#include <sys/wait.h>
#include <unistd.h>

#include "frvt1N.h"
#include "util.h"

using namespace std;
using namespace FRVT;

const int candListLength{20};
const std::string candListHeader{"searchId candidateRank searchRetCode isAssigned templateId score decision"};

int
enroll(shared_ptr<IdentInterface> &implPtr,
        const string &configDir,
        const string &inputFile,
        const string &outputLog,
        const string &edb,
        const string &manifest)
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

    /* header */
    logStream << "id image templateSizeBytes returnCode isLeftEyeAssigned "
            "isRightEyeAssigned xleft yleft xright yright" << endl;

    /* Open EDB file for writing */
    ofstream edbStream(edb);
    if (!edbStream.is_open()) {
        cerr << "Failed to open stream for " << edb << "." << endl;
        return FAILURE;
    }

    /* Open manifest for writing */
    ofstream manifestStream(manifest);
    if (!manifestStream.is_open()) {
        cerr << "Failed to open stream for " << manifest << "." << endl;
        return FAILURE;
    }

    string id, imagePath, desc;
    while (inputStream >> id >> imagePath >> desc) {
        Image image;
        if (!readImage(imagePath, image)) {
            cerr << "Failed to load image file: " << imagePath << "." << endl;
            return FAILURE;
        }
        image.description = getLabel(desc);

        Multiface faces{image};
        vector<uint8_t> templ;
        vector<EyePair> eyes;
        auto ret = implPtr->createTemplate(faces, TemplateRole::Enrollment_1N, templ, eyes);

        /* Write to edb and manifest */
        manifestStream << id << " "
                << templ.size() << " "
                << edbStream.tellp() << endl;
        edbStream.write(
                (char*)templ.data(),
                templ.size());

        /* Write template stats to log */
        logStream << id << " "
                << imagePath << " "
                << templ.size() << " "
                << static_cast<std::underlying_type<ReturnCode>::type>(ret.code) << " "
                << (eyes.size() > 0 ? eyes[0].isLeftAssigned : false) << " "
                << (eyes.size() > 0 ? eyes[0].isRightAssigned : false) << " "
                << (eyes.size() > 0 ? eyes[0].xleft : 0) << " "
                << (eyes.size() > 0 ? eyes[0].yleft : 0) << " "
                << (eyes.size() > 0 ? eyes[0].xright : 0) << " "
                << (eyes.size() > 0 ? eyes[0].yright : 0) << " "
                << endl;
    }
    inputStream.close();

    /* Remove the input file */
    if( remove(inputFile.c_str()) != 0 )
        cerr << "Error deleting file: " << inputFile << endl;

    return SUCCESS;
}

int
finalize(shared_ptr<IdentInterface> &implPtr,
        const string &edbDir,
		const string &enrollDir)
{
    string edb{edbDir+"/edb"}, manifest{edbDir+"/manifest"};
    /* Check file existence of edb and manifest */
    if (!(ifstream(edb) && ifstream(manifest))) {
        cerr << "EDB file: " << edb << " and/or manifest file: "
                << manifest << " is missing." << endl;
        return FAILURE;
    }

    auto ret = implPtr->finalizeEnrollment(enrollDir, edb, manifest, GalleryType::Unconsolidated);
    if (ret.code != ReturnCode::Success) {
        cerr << "finalizeEnrollment() returned error code: "
                << to_string(ret.code) << "." << endl;
        return FAILURE;
    }
    return SUCCESS;
}

void
searchAndLog(
        shared_ptr<IdentInterface> &implPtr,
        const string id,
        const vector<uint8_t> &templ,
        ofstream &candListStream,
        FRVT::ReturnStatus templGenRet)
{
    vector<Candidate> candidateList;
    bool decision{false};
    FRVT::ReturnStatus ret;

    /* If a valid search template was generated */
    if (templGenRet.code == ReturnCode::Success) {
        ret = implPtr->identifyTemplate(
                templ,
                candListLength,
                candidateList,
                decision);
        if (ret.code != ReturnCode::Success) {
            /* Populate candidate list with null entries */
            candidateList.resize(candListLength);
        }
    } else {
        ret = templGenRet;
        /* Populate candidate list with null entries */
        candidateList.resize(candListLength);
    }

    /* Write to candidate list file */
    int i{0};
    for (const auto& candidate : candidateList)
        candListStream << id << " " << i++ << " "
        << static_cast<underlying_type<ReturnCode>::type>(ret.code) << " "
        << candidate.isAssigned << " "
        << candidate.templateId << " "
        << candidate.similarityScore << " "
        << decision << endl;
}

int
search(shared_ptr<IdentInterface> &implPtr,
		const string &configDir,
		const string &enrollDir,
		const string &inputFile,
		const string &candList)
{
    /* Read probes */
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Failed to open stream for " << inputFile << "." << endl;
        return FAILURE;
    }

    /* Open candidate list log for writing */
    ofstream candListStream(candList);
    if (!candListStream.is_open()) {
        cerr << "Failed to open stream for " << candList << "." << endl;
        return FAILURE;
    }
    /* header */
    candListStream << candListHeader << endl;

    /* Process each probe */
    string id, imagePath, desc;
    while (inputStream >> id >> imagePath >> desc) {
        Image image;
        if (!readImage(imagePath, image)) {
            cerr << "Failed to load image file: " << imagePath << "." << endl;
            return FAILURE;
        }
        image.description = getLabel(desc);

        Multiface faces{image};
        vector<uint8_t> templ;
        vector<EyePair> eyes;
        auto ret = implPtr->createTemplate(faces, TemplateRole::Search_1N, templ, eyes);

        /* Do search and log results to candidatelist file */
        searchAndLog(implPtr, id, templ, candListStream, ret);
    }
    inputStream.close();

    /* Remove the input file */
    if( remove(inputFile.c_str()) != 0 )
        cerr << "Error deleting file: " << inputFile << endl;

    return SUCCESS;
}

int
insertAndDelete(shared_ptr<IdentInterface> &implPtr,
        const string &inputFile,
        const string &candList)
{
    ifstream inputStream(inputFile);
    if (!inputStream.is_open()) {
        cerr << "Failed to open stream for " << inputFile << "." << endl;
        return FAILURE;
    }

    vector<string> ids;
    vector<vector<uint8_t>> templates;
    vector<FRVT::ReturnStatus> retCodes;
    string id, imagePath, desc;
    bool firstEntry{true};
    while (inputStream >> id >> imagePath >> desc) {
        ids.push_back(id);

        Image image;
        if (!readImage(imagePath, image)) {
            cerr << "Failed to load image file: " << imagePath << "." << endl;
            return FAILURE;
        }
        image.description = getLabel(desc);

        Multiface faces{image};
        vector<uint8_t> templ;
        vector<EyePair> eyes;
        TemplateRole role{TemplateRole::Enrollment_1N};
        /* First entry will be used to search while
         * all others will be inserted into gallery
         */
        if (firstEntry) {
            role = TemplateRole::Search_1N;
            firstEntry = false;
        }
        auto ret = implPtr->createTemplate(faces, role, templ, eyes);
        retCodes.push_back(ret);
        templates.push_back(templ);
    }
    inputStream.close();

    /* Open candidate list log for writing */
    ofstream candListStream(candList);
    if (!candListStream.is_open()) {
        cerr << "Failed to open stream for " << candList << "." << endl;
        return FAILURE;
    }
    /* header */
    candListStream << candListHeader << endl;

    /* Incrementally insert entries 2 to N and search with entry 1 */
    for( unsigned int i=1; i<ids.size(); i++ ) {
        auto ret = implPtr->galleryInsertID(
                templates[i],
                ids[i]);

        /* Do search and log results to candidatelist file */
        searchAndLog(implPtr, ids[0], templates[0], candListStream, retCodes[0]);
    }

    /* Incrementally delete entries 2 to N and search with entry 1 */
    for( unsigned int i=1; i<ids.size(); i++ ) {
        auto ret = implPtr->galleryDeleteID(ids[i]);

        /* Do search and log results to candidatelist file */
        searchAndLog(implPtr, ids[0], templates[0], candListStream, retCodes[0]);
    }
    return SUCCESS;
}

void usage(const string &executable)
{
    cerr << "Usage: " << executable << " enroll|finalize|search -c configDir -e enrollDir "
            "-o outputDir -h outputStem -i inputFile -t numForks" << endl;
    exit(EXIT_FAILURE);
}

int
initialize(
        shared_ptr<IdentInterface> &implPtr,
        const string &configDir,
        const string &enrollDir,
        Action action)
{
    if (action == Action::Enroll_1N) {
        /* Initialization */
        auto ret = implPtr->initializeTemplateCreation(configDir, TemplateRole::Enrollment_1N);
        if (ret.code != ReturnCode::Success) {
            cerr << "initializeTemplateCreation(TemplateRole::Enrollment_1N) returned error code: "
                    << to_string(ret.code) << "." << endl;
            return FAILURE;
        }
    } else if (action == Action::Search_1N || action == Action::InsertAndDelete) {
        /* Initialize probe feature extraction */
        auto ret = implPtr->initializeTemplateCreation(configDir, TemplateRole::Search_1N);
        if (ret.code != ReturnCode::Success) {
            cerr << "initializeTemplateCreation(TemplateRole::Search_1N) returned error code: "
                    << to_string(ret.code) << "." << endl;
            return FAILURE;
        }

        /* Initialize search */
        ret = implPtr->initializeIdentification(configDir, enrollDir);
        if (ret.code != ReturnCode::Success) {
            cerr << "initializeIdentification() returned error code: "
                    << to_string(ret.code) << "." << endl;
            return FAILURE;
        }
    }
    return SUCCESS;
}

int
main(int argc, char* argv[])
{
    string actionstr{argv[1]},
    configDir{"config"},
    enrollDir{"enroll"},
    outputDir{"output"},
    outputFileStem{"stem"},
    inputFile;
    int numForks = 1;

    int requiredArgs = 2; /* exec name and action */
    for (int i = 0; i < argc - requiredArgs; i++) {
        if (strcmp(argv[requiredArgs+i],"-c") == 0)
            configDir = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-e") == 0)
            enrollDir = argv[requiredArgs+(++i)];
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
            return EXIT_FAILURE;
        }
    }

    auto implPtr = IdentInterface::getImplementation();
    Action action;
    if (actionstr == "enroll")
        action = Action::Enroll_1N;
    else if(actionstr == "search")
        action = Action::Search_1N;
    else if(actionstr == "finalize")
        action = Action::Finalize_1N;
    else if(actionstr == "insertAndDelete")
        action = Action::InsertAndDelete;
    else {
        cerr << "Unknown command: " << actionstr << endl;
        usage(argv[0]);
    }

    if (action == Action::Enroll_1N || action == Action::Search_1N) {
        /* Initialization */
        if (initialize(implPtr, configDir, enrollDir, action) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        /* Split input file into appropriate number of splits */
        vector<string> inputFileVector;
        if (splitInputFile(inputFile, outputDir, numForks, inputFileVector) != EXIT_SUCCESS) {
            cerr << "An error occurred with processing the input file." << endl;
            return EXIT_FAILURE;
        }

        bool parent = false;
        int i = 0;
        ReturnStatus ret;
        for (auto &inputFile : inputFileVector) {
            /* Fork */
            switch(fork()) {
            case 0: /* Child */
                if (action == Action::Enroll_1N)
                    return enroll(
                            implPtr,
                            configDir,
                            inputFile,
                            outputDir + "/" + outputFileStem + "." + to_string(action) + "." + to_string(i),
                            outputDir + "/edb." + to_string(i),
                            outputDir + "/manifest." + to_string(i));
                else if (action == Action::Search_1N)
                    return search(
                            implPtr,
                            configDir,
                            enrollDir,
                            inputFile,
                            outputDir + "/" + outputFileStem + "." + to_string(action) + "." + to_string(i));
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
                if (WIFEXITED(stat_val)) {}
                else if (WIFSIGNALED(stat_val))
                    cerr << "PID " << cpid << " exited due to signal " <<
                    WTERMSIG(stat_val) << endl;
                else
                    cerr << "PID " << cpid << " exited with unknown status." << endl;

                numForks--;
            }
        }
    } else if (action == Action::Finalize_1N) {
        return finalize(implPtr, outputDir, enrollDir);
    } else if (action == Action::InsertAndDelete) {
        /* Initialization */
        if (initialize(implPtr, configDir, enrollDir, action) != EXIT_SUCCESS)
            return EXIT_FAILURE;

        return insertAndDelete(implPtr,
                inputFile,
                outputDir + "/" + outputFileStem + "." + to_string(action));
    }

    return EXIT_SUCCESS;
}
