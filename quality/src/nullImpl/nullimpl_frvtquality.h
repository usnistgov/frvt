/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef NULLIMPL_FRVTQUALITY_H_
#define NULLIMPL_FRVTQUALITY_H_

#include "frvt_quality.h"

/*
 * Declare the implementation class of the FRVT 1:1 Interface
 */
namespace FRVT_QUALITY {
    class NullImplFRVTQuality : public FRVT_QUALITY::Interface {
public:

    NullImplFRVTQuality();
    ~NullImplFRVTQuality() override;

    FRVT::ReturnStatus
    initialize(const std::string &configDir) override;

    FRVT::ReturnStatus
    scalarQuality(
		const FRVT::Image &face,
		double &quality) override;

    static std::shared_ptr<FRVT_QUALITY::Interface>
    getImplementation();

private:
    std::string configDir;
    static const int featureVectorSize{4};
    // Some other members
};
}

#endif /* NULLIMPL_FRVTQUALITY_H_ */
