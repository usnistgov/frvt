/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstring>
#include "nullimplfrvt11.h"

using namespace std;
using namespace FRVT;

void
writeJpgImage(
    std::string num,
    uint8_t* decompressed_data,
    int size,
    int width,
    int height)
{
    char w[50], h[50];
    sprintf(w,"%d",width);
    sprintf(h,"%d",height);
    std::string wstring(w), hstring(h);
    FILE *fp = fopen((num+"_"+wstring+"_"+hstring+".raw").c_str(), "wb");
    fwrite(decompressed_data, 1, size, fp);
    fclose(fp);
}

NullImplFRVT11::NullImplFRVT11() {}

NullImplFRVT11::~NullImplFRVT11() {}

ReturnStatus
NullImplFRVT11::train(
        const std::string &configDir,
        const std::string &trainedConfigDir,
        const std::vector<faceAttributePair> &faces)
{
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT11::initialize(const std::string &configDir)
{
    this->whichGPU = 0;
    this->counter = 0;
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT11::setGPU(uint8_t gpuNum)
{
    this->whichGPU = gpuNum;
	return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT11::createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates)
{
    string blurb{"Somewhere out there, beneath the pale moon light\n"};

    templ.resize(blurb.size());
    memcpy(templ.data(), blurb.c_str(), blurb.size());
    //cout << "size of personrep template: " << tattooTemplate.getTemplateSize() << endl;

    for (unsigned int i=0; i<faces.size(); i++) {
        eyeCoordinates.push_back(EyePair(true, true, i, i, i+1, i+1));
    }

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVT11::matchTemplates(
        const std::vector<uint8_t> &verifTemplate,
        const std::vector<uint8_t> &enrollTemplate,
        double &similarity)
{
    similarity = 0.88;
    return ReturnStatus(ReturnCode::Success);
}

std::shared_ptr<Interface>
Interface::getImplementation()
{
    return std::make_shared<NullImplFRVT11>();
}





