/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef CustomImplFRVT11_H_
#define CustomImplFRVT11_H_

#include "frvt11.h"

#include "face_align.hpp"
#include "mxnet_mtcnn.hpp"
#include "feature_extract.hpp"
// #include "make_label.hpp"
// #include "comm_lib.hpp"

/*
 * Declare the implementation class of the FRVT 1:1 Interface
 */
namespace FRVT_11 {
    class CustomImplFRVT11 : public FRVT_11::Interface {
public:

    CustomImplFRVT11();
    ~CustomImplFRVT11() override;

    FRVT::ReturnStatus
    initialize(const std::string &configDir) override;

    FRVT::ReturnStatus
    createTemplate(
            const FRVT::Multiface &faces,
            FRVT::TemplateRole role,
            std::vector<uint8_t> &templ,
            std::vector<FRVT::EyePair> &eyeCoordinates) override;

    FRVT::ReturnStatus
    matchTemplates(
            const std::vector<uint8_t> &verifTemplate,
            const std::vector<uint8_t> &enrollTemplate,
            double &similarity) override;

    static std::shared_ptr<FRVT_11::Interface>
    getImplementation();

private:
    std::string configDir;
    static const int featureVectorSize{4};
    // Some other members

    // Mtcnn * m_mtcnn;
    // MxNetMtcnn* mDetector;
    // Mxnet_extract* mExtract;
    std::shared_ptr<MxNetMtcnn> mFaceDetector;
    std::shared_ptr<Mxnet_extract> mExtractor;
};
}

#endif /* CustomImplFRVT11_H_ */
