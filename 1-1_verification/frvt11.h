/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT11_H_
#define FRVT11_H_

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace FRVT {
/**
 * @brief
 * Struct representing a single image
 */
typedef struct Image {
    /** Labels describing the type of image */
    enum class Label {
        /** Unknown or unassigned. */
        UNKNOWN = 0,
        /** Frontal, ISO/IEC 19794-5:2005 compliant. */
        ISO = 1,
        /** From law enforcement booking processes. Nominally frontal. */
        MUGSHOT = 2,
        /** The image might appear in a news source or magazine.
         * The images are typically well exposed and focused but
         * exhibit pose and illumination variations. */
        PHOTOJOURNALISM = 3,
        /** The image is taken from a child exploitation database.
         * This imagery has highly unconstrained pose and illumination */
        EXPLOITATION = 4,
        /** Unconstrained image, taken by an amateur photographer, exhibiting
         * wide variations in pose, illumination, and resolution.
         */
        WILD = 5
    };

    /** Number of pixels horizontally */
    uint16_t width;
    /** Number of pixels vertically */
    uint16_t height;
    /** Number of bits per pixel. Legal values are 8 and 24. */
    uint8_t depth;
    /** Managed pointer to raster scanned data.
     * Either RGB color or intensity.
     * If image_depth == 24 this points to  3WH bytes  RGBRGBRGB...
     * If image_depth ==  8 this points to  WH bytes  IIIIIII */
    std::shared_ptr<uint8_t> data;
    /** @brief Single description of the image.  */
    Label description;

    Image() :
        width{0},
        height{0},
        depth{24},
        description{Label::UNKNOWN}
        {}

    Image(
        uint16_t width,
        uint16_t height,
        uint8_t depth,
        const std::shared_ptr<uint8_t> &data,
        Label description
        ) :
        width{width},
        height{height},
        depth{depth},
        data{data},
        description{description}
        {}

    /** @brief This function returns the size of the image data. */
    size_t
    size() const { return (width * height * (depth / 8)); }
} Image;

/**
 * @brief
 * Data structure representing a set images of a single person
 *
 * @details
 * The set of faces passed to the template extraction process.
 */
using Multiface = std::vector<Image>;

/** Labels describing the type/role of the template
 * to be generated (provided as input to template generation)
 */
enum class TemplateRole {
    /** Enrollment template */
    Enrollment_11,
    /** Verification template */
    Verification_11
};

/**
 * @brief
 * Return codes for functions specified in this API
 */
enum class ReturnCode {
    /** Success */
    Success = 0,
    /** Error reading configuration files */
    ConfigError,
    /** Elective refusal to process the input */
    RefuseInput,
    /** Involuntary failure to process the image */
    ExtractError,
    /** Cannot parse the input data */
    ParseError,
    /** Elective refusal to produce a template */
    TemplateCreationError,
    /** Either or both of the input templates were result of failed
     * feature extraction */
    VerifTemplateError,
    /** The implementation cannot support the number of input images */
    NumDataError,
    /** Template file is an incorrect format or defective */
    TemplateFormatError,
    /** There was a problem setting or accessing the GPU */
    GPUError,
    /** Vendor-defined failure */
    VendorError
};

/** Output stream operator for a ReturnCode object. */
inline std::ostream&
operator<<(
    std::ostream &s,
    const ReturnCode &rc)
{
    switch (rc) {
    case ReturnCode::Success:
        return (s << "Success");
    case ReturnCode::ConfigError:
        return (s << "Error reading configuration files");
    case ReturnCode::RefuseInput:
        return (s << "Elective refusal to process the input");
    case ReturnCode::ExtractError:
        return (s << "Involuntary failure to process the image");
    case ReturnCode::ParseError:
        return (s << "Cannot parse the input data");
    case ReturnCode::TemplateCreationError:
        return (s << "Elective refusal to produce a template");
    case ReturnCode::VerifTemplateError:
        return (s << "Either/both input templates were result of failed feature extraction");
    case ReturnCode::NumDataError:
        return (s << "Number of input images not supported");
    case ReturnCode::TemplateFormatError:
        return (s << "Template file is an incorrect format or defective");
    case ReturnCode::GPUError:
        return (s << "Problem setting or accessing the GPU");
    case ReturnCode::VendorError:
        return (s << "Vendor-defined error");
    default:
        return (s << "Undefined error");
    }
}

/**
 * @brief
 * A structure to contain information about a failure by the software
 * under test.
 *
 * @details
 * An object of this class allows the software to return some information
 * from a function call. The string within this object can be optionally
 * set to provide more information for debugging etc. The status code
 * will be set by the function to Success on success, or one of the
 * other codes on failure.
 */
typedef struct ReturnStatus {
    /** @brief Return status code */
    ReturnCode code;
    /** @brief Optional information string */
    std::string info;

    ReturnStatus() {}
    /**
     * @brief
     * Create a ReturnStatus object.
     *
     * @param[in] code
     * The return status code; required.
     * @param[in] info
     * The optional information string.
     */
    ReturnStatus(
        const ReturnCode code,
        const std::string &info = ""
        ) :
        code{code},
        info{info}
        {}
} ReturnStatus;

typedef struct EyePair
{
    /** If the left eye coordinates have been computed and
     * assigned successfully, this value should be set to true,
     * otherwise false. */
    bool isLeftAssigned;
    /** If the right eye coordinates have been computed and
     * assigned successfully, this value should be set to true,
     * otherwise false. */
    bool isRightAssigned;
    /** X and Y coordinate of the center of the subject's left eye.  If the
     * eye coordinate is out of range (e.g. x < 0 or x >= width), isLeftAssigned
     * should be set to false. */
    uint16_t xleft;
    uint16_t yleft;
    /** X and Y coordinate of the center of the subject's right eye.  If the
     * eye coordinate is out of range (e.g. x < 0 or x >= width), isRightAssigned
     * should be set to false. */
    uint16_t xright;
    uint16_t yright;

    EyePair() :
        isLeftAssigned{false},
        isRightAssigned{false},
        xleft{0},
        yleft{0},
        xright{0},
        yright{0}
        {}

    EyePair(
        bool isLeftAssigned,
        bool isRightAssigned,
        uint16_t xleft,
        uint16_t yleft,
        uint16_t xright,
        uint16_t yright
        ) :
        isLeftAssigned{isLeftAssigned},
        isRightAssigned{isRightAssigned},
        xleft{xleft},
        yleft{yleft},
        xright{xright},
        yright{yright}
        {}
} EyePair;

/**
 * @brief
 * A structure to contain information about a subject useful for training/
 * model adaptation
 */
typedef struct Attributes {
    enum class Gender {Unknown, Male, Female};
    enum class Race {Unknown, White, Black, East Asian, South Asian, Hispanic};
    enum class EyeGlasses {Unknown, NotWearing, Wearing};
    enum class FacialHair {Unknown, Moustache, Goatee, Beard};
    enum class SkinTone {Unknown, LightPink, LightYellow, MediumPinkBrown, MediumYellowBrown, MediumDarkBrown, DarkBrown};

    /** A subject ID that identifies a person.  Images of the same
     * person will have the same subject ID */
    std::string id;
    double age;
    Gender gender;
    Race race;
    EyeGlasses eyeglasses;
    FacialHair facialhair;
    double height;
    double weight;
    SkinTone skintone;

    Attributes() :
        id{""},
        age{-1.0},
        gender{Gender::Unknown},
        race{Race::Unknown},
        eyeglasses{EyeGlasses::Unknown},
        facialhair{FacialHair::Unknown},
        height{-1.0},
        weight{-1.0},
        skintone{SkinTone::Unknown}
        {}
} Attributes;

/** A pair of face image and associated attributes. */
using faceAttributePair = std::pair<Image, Attributes>;

/**
 * @brief
 * The interface to FRVT 1:1 implementation
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
     */
    virtual ReturnStatus
    createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates) = 0;

    /**
     * @brief This function compares two proprietary templates and outputs a
     * similarity score, which need not satisfy the metric properties. When
     * either or both of the input templates are the result of a failed
     * template generation, the similarity score shall be -1 and the function
     * return value shall be VerifTemplateError.
     *
     * param[in] verifTemplate
     * A verification template from createTemplate(role=Verification_11).
     * The underlying data can be accessed via verifTemplate.data().  The size,
     * in bytes, of the template could be retrieved as verifTemplate.size().
     * param[in] enrollTemplate
     * An enrollment template from createTemplate(role=Enrollment_11).
     * The underlying data can be accessed via enrollTemplate.data().  The size,
     * in bytes, of the template could be retrieved as enrollTemplate.size().
     * param[out] similarity
     * A similarity score resulting from comparison of the templates,
     * on the range [0,DBL_MAX].
     *
     */
    virtual ReturnStatus
    matchTemplates(
        const std::vector<uint8_t> &verifTemplate,
        const std::vector<uint8_t> &enrollTemplate,
        double &similarity) = 0;

    /**
     * @brief This function sets the GPU device number to be used by all
     * subsequent implementation function calls.  gpuNum is a zero-based
     * sequence value of which GPU device to use.  0 would mean the first
     * detected GPU, 1 would be the second GPU, etc.  If the implementation
     * does not use GPUs, then this function call should simply do nothing.
     *
     * @param[in] gpuNum
     * Index number representing which GPU to use
     */
    virtual ReturnStatus
    setGPU(uint8_t gpuNum) = 0;

    /**
     * @brief This function provides the implementation with face images and
     * associated attributes where available.  Attributes include a subject ID
     * (this value is always assigned), and where available, subject data such
     * as age, gender, race, and other information.  Images of the same person
     * will have the same subject ID.  Genuine associations can be created
     * using images with the same subject ID, and imposter associations can be
     * derived using images with different subject IDs.  This function may or
     * may not be called prior to creation of templates or matching.  The
     * implementation’s ability to create or match templates should not be
     * dependent on this function.
     *
     * @param[in] configDir
     * A read-only directory containing any developer-supplied configuration
     * parameters or run-time data.  The name of this directory is assigned by
     * NIST, not hardwired by the provider.  The names of the
     * files in this directory are hardwired in the implementation and are
     * unrestricted.
     * @param[in] trainedConfigDir
     * A directory with read-write permissions where the implementation can
     * store any training output.  The name of this directory is assigned by
     * NIST, not hardwired by the provider.  The names of the files in this
     * directory are hardwired in the implementation and are unrestricted.
     * Important: This directory is what will subsequently be provided to the
     * implementation’s initialize() function as the input configuration
     * directory if this training function is invoked.  Therefore, at a minimum,
     * even if you choose not to implement this function, the original
     * configuration data in configDir must be copied over into this directory.
     * @param[in] faces
     * A vector of face image-subject attribute pairs provided to the
     * implementation for training purposes
     */
    virtual ReturnStatus
    train(
        const std::string &configDir,
        const std::string &trainedConfigDir,
        const std::vector<faceAttributePair> &faces) = 0;

    /**
     * @brief
     * Factory method to return a managed pointer to the Interface object.
     * @details
     * This function is implemented by the submitted library and must return
     * a managed pointer to the Interface object.
     *
     * @note
     * A possible implementation might be:
     * return (std::make_shared<Implementation>());
     */
    static std::shared_ptr<Interface>
    getImplementation();
};
}

#endif /* FRVT11_H_ */
