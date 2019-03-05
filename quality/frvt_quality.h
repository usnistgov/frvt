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
class QualityInterface {
public:
    virtual ~QualityInterface() {}

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
    virtual ReturnStatus
    initialize(const std::string &configDir) = 0;

    /**
     * @brief This function takes a Multiface and outputs a proprietary template
     * and associated eye coordinates.  The vectors to store the template and
     * eye coordinates will be initially empty, and it is up to the implementation
     * to populate them with the appropriate data.  In all cases, even when unable
     * to extract features, the output shall be a template that may be passed to
     * the match_templates function without error.  That is, this routine must
     * internally encode "template creation failed" and the matcher must
     * transparently handle this.
     *
     * param[in] faces
     * Implementations must alter their behavior according to the number of
     * images contained in the structure and the TemplateRole type.
     * param[in] role
     * Label describing the type/role of the template to be generated
     * param[out] templ
     * The output template.  The format is entirely unregulated.  This will be
     * an empty vector when passed into the function, and the implementation
     * can resize and populate it with the appropriate data.
     * param[out] eyeCoordinates
     * For each input image in the Multiface, the function shall return the
     * estimated eye centers. This will be an empty vector when passed into the
     * function, and the implementation shall populate it with the appropriate
     * number of entries.  Values in eyeCoordinates[i] shall correspond to faces[i].
     * param[out] quality
     * For each image in the faces vector, an assessment of image quality.
     * This will be an empty vector when passed into the function, and the
     * implementation shall populate it with the appropriate number of entries.
     * Values in quality[i] shall correspond to faces[i].  The legal values are
     * [0,100] - The value should have a monotonic decreasing relationship with
     * false non-match rate anticipated for this sample if it was compared with
     * a pristine image of the same person.  So, a low value indicates high
     * expected FNMR.
     * A value of -1.0 indicates a failed attempt to calculate a quality
     * score or the value is unassigned.
     */
    virtual ReturnStatus
    createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates,
        std::vector<double> &quality) = 0;

    /**
     * @brief
     * Factory method to return a managed pointer to the QualityInterface object.
     * @details
     * This function is implemented by the submitted library and must return
     * a managed pointer to the QualityInterface object.
     *
     * This function MUST be implemented.
     *
     * @note
     * A possible implementation might be:
     * return (std::make_shared<Implementation>());
     */
    static std::shared_ptr<QualityInterface>
    getImplementation();
};
}

#endif /* FRVT_QUALITY_H_ */
