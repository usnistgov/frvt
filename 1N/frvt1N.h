/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT1N_H_
#define FRVT1N_H_

#include <cstdint>
#include <string>
#include <vector>

#include <frvt_structs.h>

namespace FRVT_1N {

/** Labels describing the composition of the 1:N gallery
 *  (provided as input into gallery finalization function)
 */
enum class GalleryType {
    /** Consolidated, subject-based */
    Consolidated,
    /** Unconsolidated, event-based */
    Unconsolidated
};

/**
 * @brief
 * Data structure for result of an identification search
 */
typedef struct Candidate {
    /** @brief If the candidate is valid, this should be set to true. If
     * the candidate computation failed, this should be set to false.
     * If value is set to false, similarityScore and templateId
     * will be ignored entirely. */
    bool isAssigned;

    /** @brief The template ID from the enrollment database manifest */
    std::string templateId;

    /** @brief Measure of similarity between the identification template
     * and the enrolled candidate.  Higher scores mean more likelihood that
     * the samples are of the same person.  An algorithm is free to assign
     * any value to a candidate.
     * The distribution of values will have an impact on the appearance of a
     * plot of false-negative and false-positive identification rates. */
    double similarityScore;

    Candidate() :
        isAssigned{false},
        templateId{""},
        similarityScore{0.0}
        {}

    Candidate(
        bool isAssigned,
        std::string templateId,
        double similarityScore) :
        isAssigned{isAssigned},
        templateId{templateId},
        similarityScore{similarityScore}
        {}
} Candidate;

/**
 * @brief
 * The interface to FRVT 1:N implementation
 *
 * @details
 * The submission software under test will implement this interface by
 * sub-classing this class and implementing each method therein.
 */
class IdentInterface {
public:
    virtual ~IdentInterface() {}

    /**
     * @brief Before images are sent to the template
     * creation function, the test harness will call this initialization
     * function.
     * @details This function will be called N=1 times by the NIST application,
     * prior to parallelizing M >= 1 calls to createTemplate() via fork().
     *
     * This function will be called from a single process/thread.
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data files.
     * @param[in] role
     * A value from the TemplateRole enumeration that indicates the intended
     * usage of the template to be generated.  In this case, either a 1:N
     * enrollment template used for gallery enrollment or 1:N identification
     * template used for search.
     */
    virtual ReturnStatus
    initializeTemplateCreation(
        const std::string &configDir,
        TemplateRole role) = 0;

    /**
     * @brief This function takes an input Multiface and outputs a template
     * and associated eye coordinates
     *
     * @details For enrollment templates: If the function
     * executes correctly (i.e. returns a successful exit status),
     * the template will be enrolled into a gallery.  The NIST
     * calling application may store the resulting template,
     * concatenate many templates, and pass the result to the enrollment
     * finalization function.  The resulting template may also
     * be inserted immediately into a previously finalized gallery.
     * When the implementation fails to produce a
     * template, it shall still return a blank template (which can be zero
     * bytes in length). The template will be included in the
     * enrollment database/manifest like all other enrollment templates, but
     * is not expected to contain any feature information.
     * <br>For identification templates: If the function returns a
     * non-successful return status, the output template will be not be used
     * in subsequent search operations.
     *
     * In the event that more than one face is detected in an image,
     * features should be extracted from the foreground face, that is, the
     * largest face in the image.
     *
     * @param[in] faces
     * The input Multiface object.
     * @param[in] role
     * A value from the TemplateRole enumeration that indicates the intended
     * usage of the template to be generated.  In this case, either a 1:N
     * enrollment template used for gallery enrollment or 1:N identification
     * template used for search.
     * @param[out] templ
     * The output template.  The format is entirely unregulated.  This will be
     * an empty vector when passed into the function, and the implementation can
     * resize and populate it with the appropriate data.
     * @param[out] eyeCoordinates
     * (Optional) The function may choose to return the estimated eye centers
     * for the input face images.
     */
    virtual ReturnStatus
    createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates) = 0;

     /**
      * @brief This function will be called after all enrollment templates have
      * been created and freezes the enrollment data.
      * After this call, the enrollment dataset will be forever read-only.
      *
      * @details This function allows the implementation to conduct,
      * for example, statistical processing of the feature data, indexing and
      * data re-organization.  The function may create its own data structure.
      * It may increase or decrease the size of the stored data.  No output is
      * expected from this function, except a return code.  The function will
      * generally be called in a separate process after all the enrollment processes
      * are complete.
      * NOTE: Implementations shall not move the input data.  Implementations
      * shall not point to the input data.
      * Implementations should not assume the input data would be readable
      * after the call.  Implementations must,
      * <b>at a minimum, copy the input data</b> or otherwise extract what is
      * needed for search.
      *
      * This function will be called from a single process/thread.
      *
      * @param[in] configDir
      * A read-only directory containing any developer-supplied configuration
      * parameters or run-time data files.
      * @param[in] enrollmentDir
      * The top-level directory in which enrollment data was placed. This
      * variable allows an implementation
      * to locate any private initialization data it elected to place in the
      * directory.
      * @param[in] edbName
      * The name of a single file containing concatenated templates, i.e. the
      * EDB described in <em>File structure for enrolled template collection</em>.
      * While the file will have read-write-delete permission, the implementation
      * should only alter the file if it preserves the necessary content, in
      * other files for example.
      * The file may be opened directly.  It is not necessary to prepend a
      * directory name.  This is a NIST-provided
      * input - implementers shall not internally hard-code or assume any values.
      * @param[in] edbManifestName
      * The name of a single file containing the EDB manifest described in
      * <em>File structure for enrolled template collection</em>.
      * The file may be opened directly.  It is not necessary to prepend a
      * directory name.  This is a NIST-provided
      * input - implementers shall not internally hard-code or assume any values.
      * @param[in] galleryType
      * The composition of the gallery as enumerated by GalleryType.
      */
    virtual ReturnStatus
    finalizeEnrollment(
    	const std::string &configDir,
        const std::string &enrollmentDir,
        const std::string &edbName,
        const std::string &edbManifestName,
        GalleryType galleryType) = 0;

    /**
     * @brief This function will be called once prior to one or more calls to
     * identifyTemplate().  The function might set static internal variables
     * and read the enrollment gallery into memory
     * so that the enrollment database is available to the subsequent
     * identification searches.
     *
     * This function will be called from a single process/thread.
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data files.
     * @param[in] enrollmentDir
     * The read-only top-level directory in which enrollment data was placed.
     */
    virtual ReturnStatus
    initializeIdentification(
        const std::string &configDir,
        const std::string &enrollmentDir) = 0;

    /**
     * @brief This function searches an identification template against the
     * enrollment set, and outputs a vector containing candidateListLength
     * Candidates.
     *
     * @details Each candidate shall be populated by the implementation
     * and added to candidateList.  Note that candidateList will be an empty
     * vector when passed into this function.  The candidates shall appear in
     * descending order of similarity score - i.e. most similar entries appear
     * first.
     *
     * @param[in] idTemplate
     * A template from createTemplate().  If the value returned by that
     * function was non-successful, the contents of idTemplate will not be
     * used, and this function will not be called.
     * @param[in] candidateListLength
     * The number of candidates the search should return.
     * @param[out] candidateList
     * Each candidate shall be populated by the implementation.  The candidates
     * shall appear in descending order of similarity score - i.e. most similar
     * entries appear first.
     * @param[out] decision
     * A best guess at whether there is a mate within the enrollment database.
     * If there was a mate found, this value should be set to true, Otherwise, false.
     * Many such decisions allow a single point to be plotted alongside a DET curve.
     */
    virtual ReturnStatus
    identifyTemplate(
        const std::vector<uint8_t> &idTemplate,
        const uint32_t candidateListLength,
        std::vector<Candidate> &candidateList,
        bool &decision) = 0;

    /**
     * @brief This function inserts a template with an associated id
     * into an existing finalized gallery.
     *
     * @details Invocation of this function will always be preceded by
     * a call to initializeIdentification(), which will provide the location
     * of the gallery.  One or more calls to
     * identifyTemplate may be made after calling this function.
     *
     * This function will be called from a single process/thread.
     *
     * @param[in] templ
     * A template created via createTemplate(TemplateRole=Enrollment_1N)
     * @param[in] id
     * An identifier associated with the enrollment template
     */
    virtual ReturnStatus
    galleryInsertID(
        const std::vector<uint8_t> &templ,
        const std::string &id) = 0;


    /**
     * @brief This function deletes an existing template with an associated id
     * from a finalized gallery.
     *
     * @details Invocation of this function will always be preceded by
     * a call to initializeIdentification(), which will provide the location
     * of the finalized gallery.  One or more calls to
     * identifyTemplate may be made after calling this function.
     *
     * This function will be called from a single process/thread.
     *
     * @param[in] id
     * The identifier of the template to be removed from the gallery
     */
    virtual ReturnStatus
    galleryDeleteID(
        const std::string &id) = 0;

    /**
     * @brief
     * Factory method to return a managed pointer to the IdentInterface object.
     * @details
     * This function is implemented by the submitted library and must return
     * a managed pointer to the IdentInterface object.
     *
     * @note
     * A possible implementation might be:
     * return (std::make_shared<Implementation>());
     */
    static std::shared_ptr<IdentInterface>
    getImplementation();
};
}

#endif /* FRVT1N_H_ */
