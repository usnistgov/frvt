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
#include <cstdlib>

#include "nullimpl_frvtquality.h"

using namespace std;
using namespace FRVT;
using namespace FRVT_QUALITY;

NullImplFRVTQuality::NullImplFRVTQuality() {}

NullImplFRVTQuality::~NullImplFRVTQuality() {}

ReturnStatus
NullImplFRVTQuality::initialize(const std::string &configDir)
{
    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
NullImplFRVTQuality::scalarQuality(
		const FRVT::Image &face,
		double &quality)
{
	quality = 88.03;
	return ReturnStatus(ReturnCode::Success);
}

std::shared_ptr<Interface>
Interface::getImplementation()
{
    return std::make_shared<NullImplFRVTQuality>();
}





