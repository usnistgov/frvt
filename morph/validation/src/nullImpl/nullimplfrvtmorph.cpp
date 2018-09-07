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
#include "nullimplfrvtmorph.h"

using namespace std;
using namespace FRVT_MORPH;

NullImplFRVTMorph::NullImplFRVTMorph() {}

NullImplFRVTMorph::~NullImplFRVTMorph() {}

ReturnStatus
NullImplFRVTMorph::trainMorphDetector(
    const std::string &configDir,
    const std::string &trainedConfigDir,
    const std::vector<Image> &faces,
    const std::vector<bool> &isMorph)
{
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVTMorph::initialize(
    const std::string &configDir,
    const std::string &configValue)
{
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVTMorph::detectMorph(
    const Image &suspectedMorph,
    const ImageLabel &label,
    bool &isMorph,
    double &score)
{
    if (label==ImageLabel::Scanned)
        return ReturnStatus(ReturnCode::NotImplemented);

    isMorph = true;
    score = 0.99;
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVTMorph::detectMorphDifferentially(
    const Image &suspectedMorph,
    const ImageLabel &label,
    const Image &liveFace,
    bool &isMorph,
    double &score)
{
    if (label==ImageLabel::Unknown)
        return ReturnStatus(ReturnCode::NotImplemented);

    if (label==ImageLabel::NonScanned) {
        isMorph = false;
        score = 0.003;
    } else if (label==ImageLabel::Scanned)
        isMorph = true;
        score = 0.81;
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVTMorph::compareImages(
    const Image &enrollImage,
    const Image &verifImage,
    double &similarity)
{
    similarity = 0.88;
    return ReturnStatus(ReturnCode::Success);
}

std::shared_ptr<MorphInterface>
MorphInterface::getImplementation()
{
    return std::make_shared<NullImplFRVTMorph>();
}

