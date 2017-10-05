/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef NULLIMPLFRVT11_H_
#define NULLIMPLFRVT11_H_

#include "frvt11.h"

/*
 * Declare the implementation class of the FRVT 1:1 Interface
 */
namespace FRVT {
    class NullImplFRVT11 : public FRVT::Interface {
public:

    NullImplFRVT11();
    ~NullImplFRVT11() override;

    ReturnStatus
    train(
            const std::string &configDir,
            const std::string &trainedConfigDir,
            const std::vector<faceAttributePair> &faces) override;

    ReturnStatus
    initialize(const std::string &configDir) override;

    ReturnStatus
    setGPU(uint8_t gpuNum) override;

    ReturnStatus
    createTemplate(
            const Multiface &faces,
            TemplateRole role,
            std::vector<uint8_t> &templ,
            std::vector<EyePair> &eyeCoordinates) override;

    ReturnStatus
    matchTemplates(
            const std::vector<uint8_t> &verifTemplate,
            const std::vector<uint8_t> &enrollTemplate,
            double &similarity) override;

    static std::shared_ptr<FRVT::Interface>
    getImplementation();

private:
    std::string configDir;
    uint8_t whichGPU;
    int counter;
    // Some other members
};
}

#endif /* NULLIMPLFRVT11_H_ */
