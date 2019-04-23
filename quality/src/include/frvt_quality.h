/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT_QUALITY_H_
#define FRVT_QUALITY_H_

#include <cstdint>
#include <string>
#include <vector>

#include <frvt_structs.h>

namespace FRVT_QUALITY {

/**
 * @brief
 * The interface to FRVT QUALITY implementation
 *
 * @details
 * The submission software under test will implement this interface by
 * sub-classing this class and implementing each method therein.
 */
class Interface {
public:
    virtual ~Interface() {}

    /**
     * @brief This function initializes the implementation under test.  It will
     * be called by the NIST application before any call to createTemplate() or
     * matchTemplates().  The implementation under test should set all parameters.
     * This function will be called N=1 times by the NIST application, prior to
     * parallelizing M >= 1 calls to createTemplate() via fork().
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data files.  The name of this directory is
     * assigned by NIST, not hardwired by the provider.  The names of the
     * files in this directory are hardwired in the implementation and are
     * unrestricted.
     */
    virtual FRVT::ReturnStatus
    initialize(const std::string &configDir) = 0;

    /**
     * @brief This function takes an image and outputs a quality scalar.
     * The algorithm will be supplied with a label describing the type of image
     * via Image::Label, and it is up to the implementation to alter its behavior
     * based on the image type (e.g., Iso (full-frontal) versus Wild (off-angle).
     *
     * param[in] face
     * Single face image
     * param[out] quality
     * A scalar value assessment of image quality.  The legal values are
     * [0,100] - The value should have a monotonic decreasing relationship with
     * false non-match rate anticipated for this sample if it was compared with
     * a pristine image of the same person.  So, a low value indicates high
     * expected FNMR.
     * A value of -1.0 indicates a failed attempt to calculate a quality
     * score or the value is unassigned.
     */
    virtual FRVT::ReturnStatus
    scalarQuality(
        const FRVT::Image &face,
        double &quality) = 0;

    /**
     * @brief
     * Factory method to return a managed pointer to the Interface object.
     * @details
     * This function is implemented by the submitted library and must return
     * a managed pointer to the Interface object.
     *
     * This function MUST be implemented.
     *
     * @note
     * A possible implementation might be:
     * return (std::make_shared<Implementation>());
     */
    static std::shared_ptr<Interface>
    getImplementation();
};

/*
 * API versioning
 *
 * NIST code will extern the version number symbols.
 * Participant shall compile them into their core library.
 */
#ifdef NIST_EXTERN_API_VERSION
/** API major version number. */
extern uint16_t API_MAJOR_VERSION;
/** API minor version number. */
extern uint16_t API_MINOR_VERSION;
#else /* NIST_EXTERN_API_VERSION */
/** API major version number. */
uint16_t API_MAJOR_VERSION{1};
/** API minor version number. */
uint16_t API_MINOR_VERSION{0};
#endif /* NIST_EXTERN_API_VERSION */
}

#endif /* FRVT_QUALITY_H_ */
