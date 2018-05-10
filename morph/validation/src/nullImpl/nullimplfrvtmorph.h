/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef NULLIMPLFRVTMORPH_H_
#define NULLIMPLFRVTMORPH_H_

#include "frvt_morph.h"

/*
 * Declare the implementation class of the FRVT MORPH Interface
 */
namespace FRVT_MORPH {
    class NullImplFRVTMorph : public FRVT_MORPH::MorphInterface {
public:

    NullImplFRVTMorph();
    ~NullImplFRVTMorph() override;

    ReturnStatus
    trainMorphDetector(
        const std::string &configDir,
        const std::string &trainedConfigDir,
        const std::vector<Image> &faces,
        const std::vector<bool> &isMorph) override;

    ReturnStatus
    initialize(
        const std::string &configDir) override;

    ReturnStatus
    detectMorph(
        const Image &suspectedMorph,
        bool &isMorph,
        double &score) override;

    ReturnStatus
    detectMorph(
        const Image &suspectedMorph,
        const Image &liveFace,
        bool &isMorph,
        double &score) override;

    ReturnStatus
    detectScannedMorph(
        const Image &image,
        bool &isMorph,
        double &score) override;

    ReturnStatus
    matchImages(
        const Image &enrollImage,
        const Image &verifImage,
        double &similarity) override;

    static std::shared_ptr<FRVT_MORPH::MorphInterface>
    getImplementation();

private:
    std::string configDir;
    // Some other members
};
}

#endif /* NULLIMPLFRVTMORPH_H_ */
