/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef NULLIMPLFRVT1N_H_
#define NULLIMPLFRVT1N_H_

#include <map>
#include "frvt1N.h"

/*
 * Declare the implementation class of the FRVT 1:1 Interface
 */
namespace FRVT {
    class NullImplFRVT1N : public FRVT::IdentInterface {
public:

    NullImplFRVT1N();
    ~NullImplFRVT1N() override;

    ReturnStatus
    initializeTemplateCreation(
        const std::string &configDir,
        TemplateRole role) override;

    ReturnStatus
    createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates) override;

    ReturnStatus
    finalizeEnrollment(
        const std::string &configDir,
        const std::string &enrollmentDir,
        const std::string &edbName,
        const std::string &edbManifestName,
        GalleryType galleryType) override;

    ReturnStatus
    initializeIdentification(
        const std::string &configDir,
        const std::string &enrollmentDir) override;

    ReturnStatus
    identifyTemplate(
        const std::vector<uint8_t> &idTemplate,
        const uint32_t candidateListLength,
        std::vector<Candidate> &candidateList,
        bool &decision) override;

    ReturnStatus
    galleryInsertID(
        const std::vector<uint8_t> &templ,
        const std::string &id) override;

    ReturnStatus
    galleryDeleteID(
        const std::string &id) override;

    static std::shared_ptr<FRVT::IdentInterface>
    getImplementation();

private:
    std::map<std::string, std::vector<uint8_t>> templates;

    const std::string edb{"mei.edb"};
    const std::string manifest{"mei.manifest"};
};
}

#endif /* NULLIMPLFRVT1N_H_ */
