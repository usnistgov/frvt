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
    class NullImplFRVTMorph : public FRVT_MORPH::Interface {
public:

    NullImplFRVTMorph();
    ~NullImplFRVTMorph() override;

    FRVT::ReturnStatus
    initialize(
        const std::string &configDir,
        const std::string &configValue) override;

    FRVT::ReturnStatus
    detectMorph(
        const FRVT::Image &suspectedMorph,
        const FRVT::ImageLabel &label,
        bool &isMorph,
        double &score) override;

    FRVT::ReturnStatus
    detectMorphDifferentially(
        const FRVT::Image &suspectedMorph,
        const FRVT::ImageLabel &label,
        const FRVT::Image &liveFace,
        bool &isMorph,
        double &score,
        const int &ageDeltaInDays = -1) override;

    FRVT::ReturnStatus
    compareImages(
        const FRVT::Image &enrollImage,
        const FRVT::Image &verifImage,
        double &similarity) override;

    static std::shared_ptr<FRVT_MORPH::Interface>
    getImplementation();

private:
    std::string configDir;
    // Some other members
};
}

#endif /* NULLIMPLFRVTMORPH_H_ */
