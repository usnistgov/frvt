/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#ifndef FRVT_STRUCTS_H_
#define FRVT_STRUCTS_H_

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
        ISO,
        /** From law enforcement booking processes. Nominally frontal. */
        MUGSHOT,
        /** The image might appear in a news source or magazine.
         * The images are typically well exposed and focused but
         * exhibit pose and illumination variations. */
        PHOTOJOURNALISM,
        /** The image is taken from a child exploitation database.
         * This imagery has highly unconstrained pose and illumination */
        EXPLOITATION,
        /** Unconstrained image, taken by an amateur photographer, exhibiting
         * wide variations in pose, illumination, and resolution.
         */
        WILD
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
        std::shared_ptr<uint8_t> &data,
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
    /** 1:1 enrollment template */
    Enrollment_11,
    /** 1:1 verification template */
    Verification_11,
    /** 1:N enrollment template */
    Enrollment_1N,
    /** 1:N identification template */
    Search_1N
};

/** Labels describing the composition of the gallery
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
    /** Unable to detect a face in the image */
    FaceDetectionError,
    /** The implementation cannot support the number of input images */
    NumDataError,
    /** Template file is an incorrect format or defective */
    TemplateFormatError,
    /**
     * An operation on the enrollment directory
     * failed (e.g. permission, space)
     */
    EnrollDirError,
    /** Cannot locate the input data - the input files or names seem incorrect */
    InputLocationError,
    /** Memory allocation failed (e.g. out of memory) */
    MemoryError,
	/** Function is not implemented */
	NotImplemented,
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
        return (s << "Either or both of the input templates were result of "
                "failed feature extraction");
    case ReturnCode::FaceDetectionError:
        return (s << "Unable to detect a face in the image");
    case ReturnCode::NumDataError:
        return (s << "Number of input images not supported");
    case ReturnCode::TemplateFormatError:
        return (s << "Template file is an incorrect format or defective");
    case ReturnCode::EnrollDirError:
        return (s << "An operation on the enrollment directory failed");
    case ReturnCode::InputLocationError:
        return (s << "Cannot locate the input data - the input files or names "
                "seem incorrect");
    case ReturnCode::MemoryError:
        return (s << "Memory allocation failed (e.g. out of memory)");
    case ReturnCode::NotImplemented:
        return (s << "Function is not implemented");
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
 * A structure to contain information about a subject useful for training/
 * model adaptation
 */
typedef struct Attributes {
    enum class Gender {Unknown, Male, Female};
    enum class Race {Unknown, White, Black, EastAsian, SouthAsian, Hispanic};
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
}

#endif /* FRVT_STRUCTS_H_ */
