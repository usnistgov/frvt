/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#include "nullimplfrvt1N.h"

using namespace std;
using namespace FRVT;

NullImplFRVT1N::NullImplFRVT1N() {}

NullImplFRVT1N::~NullImplFRVT1N() {}

ReturnStatus
NullImplFRVT1N::initializeTemplateCreation(
    const std::string &configDir,
    TemplateRole role)
{
    // Load some stuff from the configuration directory, etc...
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::createTemplate(
    const Multiface &faces,
    TemplateRole role,
    std::vector<uint8_t> &templ,
    std::vector<EyePair> &eyeCoordinates)
{
    auto templString = std::to_string(faces.size()) +
            " Somewhere out there, beneath the pale moon light\n";
    templ.resize(templString.size());
    memcpy(templ.data(), templString.c_str(), templString.size());

    for (unsigned int i=0; i<faces.size(); i++)
        eyeCoordinates.push_back(EyePair(true, true, i, i, i+1, i+1));

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::finalizeEnrollment(
    const std::string &configDir,
    const std::string &enrollmentDir,
    const std::string &edbName,
    const std::string &edbManifestName,
    GalleryType galleryType)
{
    ifstream edbsrc(edbName, ios::binary);
    ofstream edbdest(enrollmentDir+"/"+this->edb, ios::binary);
    ifstream manifestsrc(edbManifestName, ios::binary);
    ofstream manifestdest(enrollmentDir+"/"+this->manifest, ios::binary);

    edbdest << edbsrc.rdbuf();
    manifestdest << manifestsrc.rdbuf();

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::initializeIdentification(
    const std::string &configDir,
    const std::string &enrollmentDir)
{
    auto edbManifestName = enrollmentDir + "/" + this->manifest;
    auto edbName = enrollmentDir + "/" + this->edb;

    ifstream manifestStream(edbManifestName.c_str());
    if (!manifestStream.is_open()) {
        cerr << "Failed to open stream for " << edbManifestName << "." << endl;
        return ReturnStatus(ReturnCode::ConfigError);
    }

    ifstream edbStream(edbName, ios::in | ios::binary);
    if (!edbStream.is_open()) {
        cerr << "Failed to open stream for " << edbName << "." << endl;
        return ReturnStatus(ReturnCode::ConfigError);
    }

    string templId, size, offset;
    while (manifestStream >> templId >> size >> offset) {
        edbStream.seekg(atol(offset.c_str()), ios::beg);
        std::vector<uint8_t> templData(atol(size.c_str()));
        edbStream.read((char*) &templData[0], atol(size.c_str()));

        // Insert template + id into map
        this->templates.insert(std::make_pair(templId, templData));
    }

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::identifyTemplate(
    const std::vector<uint8_t> &idTemplate,
    const uint32_t candidateListLength,
    std::vector<Candidate> &candidateList,
    bool &decision)
{
    std::vector<std::string> templateIds;
    for (auto const& element : this->templates)
        templateIds.push_back(element.first);

    for (unsigned int i=0; i<candidateListLength; i++) {
        candidateList.push_back(Candidate(true, templateIds[i%(templateIds.size())], candidateListLength-i));
    }
    decision = true;

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::galleryInsertID(
    const std::vector<uint8_t> &templ,
    const std::string &id)
{
    this->templates.insert(std::make_pair(id, templ));

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT1N::galleryDeleteID(
    const std::string &id)
{
    this->templates.erase(id);

    return ReturnStatus(ReturnCode::Success);
}

std::shared_ptr<IdentInterface>
IdentInterface::getImplementation()
{
    return make_shared<NullImplFRVT1N>();
}
