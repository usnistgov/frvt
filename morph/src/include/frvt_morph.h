/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT_MORPH_H_
#define FRVT_MORPH_H_

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include <frvt_structs.h>

namespace FRVT_MORPH {
/**
 * @brief
 * The interface to FRVT MORPH implementation
 *
 * @details
 * The submission software under test will implement this interface by
 * sub-classing this class and implementing each method therein.
 */
class Interface {
public:
    virtual ~Interface() {}

    /**
     * @brief Before images are sent to any morph detection or match function,
     * the test harness will call this initialization function.
     * @details This function will be called N=1 times by the NIST application,
     * prior to parallelizing M >= 1 calls to morph detection or matching
     * functions via fork().
     *
     * This function will be called from a single process/thread.
     *
     * If this function is not implemented, the algorithm shall return
     * ReturnCode::NotImplemented.
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data files.
     * @param[in] configValue
     * An optional string value encoding developer-specific configuration parameters
     */
    virtual FRVT::ReturnStatus
    initialize(
        const std::string &configDir,
        const std::string &configValue) = 0;

    /**
     * @brief This function takes an input image and outputs
     * 1. a binary decision on whether the image is a morph
     * 2. a "morphiness" score on [0, 1] indicating how confident the algorithm
     * thinks the image is a morph, with 0 meaning confidence that the image
     * is not a morph and 1 representing absolute confidence that it is a morph
     *
     * If this function is not implemented, the algorithm shall return
     * ReturnCode::NotImplemented.  If this function is not implemented for
     * a certain type of image, for example, the function supports non-scanned
     * photos but not scanned photos, then the function should return
     * ReturnCode::NotImplemented when the function is called with the particular
     * unsupported image type.
     *
     * @param[in] suspectedMorph
     * Input image
     * @param[in] label
     * Label indicating the type of imagery for the suspected morph.  Possible
     * types are non-scanned photo, printed-and-scanned photo, or unknown.
     * @param[out] isMorph
     * True if image contains a morph; False otherwise
     * @param[out] score
     * A score on [0, 1] representing how confident the algorithm is that the
     * image contains a morph.  0 means certainty that image does not contain
     * a morph and 1 represents certainty that image contains a morph
     */
    virtual FRVT::ReturnStatus
    detectMorph(
        const FRVT::Image &suspectedMorph,
        const FRVT::ImageLabel &label,
        bool &isMorph,
        double &score) = 0;

    /**
     * @brief This function takes two input images - a known unaltered/not morphed
     * image of the subject and an image of the same subject that's in question
     * (may or may not be a morph).  This function outputs
     * 1. a binary decision on whether <b>suspectedMorph</b> is a morph
     * (given <b>probeFace</b> as a prior)
     * 2. a "morphiness" score on [0, 1] indicating how confident the algorithm
     * thinks the image is a morph, with 0 meaning confidence that the image
     * is not a morph and 1 representing absolute confidence that it is a morph
     *
     * If this function is not implemented, the algorithm shall return
     * ReturnCode::NotImplemented.  If this function is not implemented for
     * a certain type of image, for example, the function supports non-scanned
     * photos but not scanned photos, then the function should return
     * ReturnCode::NotImplemented when the function is called with the particular
     * unsupported image type.
     *
     * @param[in] suspectedMorph
     * An image in question of being a morph (or not)
     * @param[in] label
     * Label indicating the type of imagery for the suspected morph.  Possible
     * types are non-scanned photo, printed-and-scanned photo, or unknown.
     * @param[in] probeFace
     * An image of the subject known not to be a morph (i.e., live capture
     * image)
     * @param[out] isMorph
     * True if suspectedMorph image contains a morph; False otherwise
     * @param[out] score
     * A score on [0, 1] representing how confident the algorithm is that the
     * image contains a morph.  0 means certainty that image does not contain
     * a morph and 1 represents certainty that image contains a morph
     * @param[in] ageDeltaInDays
     * Optional input parameter representing the time/age difference 
     * (in days) between the suspected morph and the live probe image.
     * Default value is -1, which means the information is not provided
     * to the function
     */
    virtual FRVT::ReturnStatus
    detectMorphDifferentially(
        const FRVT::Image &suspectedMorph,
        const FRVT::ImageLabel &label,
        const FRVT::Image &probeFace,
        bool &isMorph,
        double &score,
		const int &ageDeltaInDays = -1) = 0;

    /**
     * @brief This function compares two images and outputs a
     * similarity score. In the event the algorithm cannot perform the comparison
     * operation, the similarity score shall be set to -1 and the function
     * return code value shall be set appropriately.
     *
     * If this function is not implemented, the algorithm shall return
     * ReturnCode::NotImplemented.
     *
     * param[in] enrollImage
     * The enrollment image
     * param[in] verifImage
     * The verification image
     * param[out] similarity
     * A similarity score resulting from comparison of the two images,
     * on the range [0,DBL_MAX].
     *
     */
    virtual FRVT::ReturnStatus
    compareImages(
        const FRVT::Image &enrollImage,
        const FRVT::Image &verifImage,
        double &similarity) = 0;

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
uint16_t API_MAJOR_VERSION{2};
/** API minor version number. */
uint16_t API_MINOR_VERSION{1};
#endif /* NIST_EXTERN_API_VERSION */
}

#endif /* FRVT_MORPH_H_ */
