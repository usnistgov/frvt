/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT_MORPH_H_
#define FRVT_MORPH_H_

#include <cstdint>
#include <string>
#include <vector>

#include <frvt_structs.h>

namespace FRVT_MORPH {

/** Labels describing the type of image */
enum class ImageLabel {
    /** Image type is unknown or unassigned */
    Unknown = 0,
    /** Non-scanned image */
    NonScanned,
    /** Printed-and-scanned image */
    Scanned
};

/**
 * @brief
 * The interface to FRVT MORPH implementation
 *
 * @details
 * The submission software under test will implement this interface by
 * sub-classing this class and implementing each method therein.
 */
class MorphInterface {
public:
    virtual ~MorphInterface() {}

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
    virtual ReturnStatus
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
    virtual ReturnStatus
    detectMorph(
        const Image &suspectedMorph,
        const ImageLabel &label,
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
     */
    virtual ReturnStatus
    detectMorphDifferentially(
        const Image &suspectedMorph,
        const ImageLabel &label,
        const Image &probeFace,
        bool &isMorph,
        double &score) = 0;

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
    virtual ReturnStatus
    compareImages(
        const Image &enrollImage,
        const Image &verifImage,
        double &similarity) = 0;

    /**
     * @brief This function provides the implementation a list of face
     * images and whether they are morphs.  This function may or
     * may not be called prior to the various morph detection functions.  The
     * implementation’s ability to detect morphs should not be dependent on this
     * function.
     *
     * This function will be called from a single process/thread.
     *
     * If this function is not implemented, the algorithm shall return
     * ReturnCode::NotImplemented.
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data.  The name of this directory is assigned by
     * NIST, not hardwired by the provider.  The names of the files in this
     * directory are hardwired in the implementation and are unrestricted.
     * @param[in] trainedConfigDir
     * A directory with read-write permissions where the implementation can
     * store any training output.  The name of this directory is assigned by
     * NIST, not hardwired by the provider.  The names of the files in this
     * directory are hardwired in the implementation and are unrestricted.
     * This directory is what will subsequently be provided to the
     * implementation’s initialize() function as the input configuration
     * directory if this training function is invoked and implemented.
     * If this function is optionally not implemented by the developer,
     * the function shall do nothing and return ReturnCode::NotImplemented.
     * @param[in] faces
     * A vector of face images provided to the implementation for training purposes
     * @param[in] isMorph
     * A vector of boolean values indicating whether the corresponding face image
     * is a morph or not.  The value in isMorph[i] corresponds
     * to the face image in faces[i].
     */
    virtual ReturnStatus
    trainMorphDetector(
        const std::string &configDir,
        const std::string &trainedConfigDir,
        const std::vector<Image> &faces,
        const std::vector<bool> &isMorph) = 0;

    /**
     * @brief
     * Factory method to return a managed pointer to the MorphInterface object.
     * @details
     * This function is implemented by the submitted library and must return
     * a managed pointer to the MorphInterface object.
     *
     * This function MUST be implemented.
     *
     * @note
     * A possible implementation might be:
     * return (std::make_shared<Implementation>());
     */
    static std::shared_ptr<MorphInterface>
    getImplementation();
};
}

#endif /* FRVT_MORPH_H_ */
