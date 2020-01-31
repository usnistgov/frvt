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

#include "customimplfrvt11.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;
using namespace FRVT;
using namespace FRVT_11;

CustomImplFRVT11::CustomImplFRVT11() {}

CustomImplFRVT11::~CustomImplFRVT11() {}

void
CvMatToTemplate(const cv::Mat& mat, std::vector<uint8_t> &templ)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>((const uint8_t*)mat.data);
    int dataSize = sizeof(float) * mat.rows;
    templ.resize(dataSize);
    memcpy(templ.data(), bytes, dataSize);
}

cv::Mat
AveragePoolOnTemplates(const std::vector<std::vector<float>>& templates)
{
    cv::Mat output_features = cv::Mat::zeros(512, 1, CV_32F);
    for (const std::vector<float>& f : templates) {
        for (int i = 0; i < f.size(); ++i) {
            output_features.at<float>(i, 0) += f[i];
        }
    }

    output_features /= templates.size();
    output_features /= cv::norm(output_features);

    return output_features;
}

ReturnStatus
CustomImplFRVT11::initialize(const std::string &configDir)
{
    putenv("OMP_NUM_THREADS=1");
    cv::setNumThreads(0);

    mFaceDetector = std::make_shared<MxNetMtcnn>();
    mExtractor = std::make_shared<Mxnet_extract>();

    std::string mtcnn_model = configDir + "/mtcnn_model";
    mFaceDetector->LoadModule(mtcnn_model);


    std::string feature_model = configDir + "/model-r100-ii";
    mExtractor->LoadExtractModule(feature_model + "/model-0000.params",
        feature_model + "/model-symbol.json", 1, 3, 112, 112);

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
CustomImplFRVT11::createTemplate(
        const Multiface &faces,
        TemplateRole role,
        std::vector<uint8_t> &templ,
        std::vector<EyePair> &eyeCoordinates)
{
    int dim  = 512;
    std::vector<std::vector<float>> templates;
    cv::Mat src(5, 2, CV_32FC1, norm_face);

    for(const Image &face: faces) {
        cv::Mat image(face.height, face.width, CV_8UC3, face.data.get());
        std::vector<face_box> face_info;
        mFaceDetector->Detect(image, face_info);

        if (face_info.size() == 0) continue;

        face_box& box = face_info[0];

        eyeCoordinates.push_back(EyePair(true, true,
        box.landmark.x[0],box.landmark.y[0],
        box.landmark.x[1],box.landmark.y[1]));

        float v2[5][2] =
        { { box.landmark.x[0] , box.landmark.y[0] },
            { box.landmark.x[1] , box.landmark.y[1] },
            { box.landmark.x[2] , box.landmark.y[2] },
            { box.landmark.x[3] , box.landmark.y[3] },
            { box.landmark.x[4] , box.landmark.y[4] } };

        cv::Mat dst(5, 2, CV_32FC1, v2);

        cv::Mat m = similarTransform(dst, src);

        cv::Mat aligned(112, 112,CV_32FC3);
        cv::Size size(112, 112);

        cv::Mat transfer = m(cv::Rect(0, 0, 3, 2));

        cv::warpAffine(image, aligned, transfer, size, 1, 0, 0);

        cv::Mat output = mExtractor->extractFeature(aligned);
        std::vector<float>out_vec(output.begin<float>(), output.end<float>());

        templates.push_back(std::vector<float>(out_vec.begin(), out_vec.end()));
    }
    /* Note: example code, potentially not portable across machines. */
    // std::vector<float> fv = {1.0, 2.0, 8.88, 765.88989};
    if (templates.size() > 0) {
        cv::Mat output_features = AveragePoolOnTemplates(templates);
        CvMatToTemplate(output_features, templ);
    } else {

    }

    return ReturnStatus(ReturnCode::Success);
}

ReturnStatus
CustomImplFRVT11::matchTemplates(
        const std::vector<uint8_t> &verifTemplate,
        const std::vector<uint8_t> &enrollTemplate,
        double &similarity)
{
    int dim  = 512;
    if (verifTemplate.size() == 0 || enrollTemplate.size() == 0) {
        similarity = 0;
    }
    else {
        cv::Mat f1(dim, 1, CV_32F, (float *)verifTemplate.data());
        cv::Mat f2(dim, 1, CV_32F, (float *)enrollTemplate.data());

        double ab = f1.dot(f2);
        similarity = ab / (
            cv::norm(f1, cv::NORM_L2) * cv::norm(f1, cv::NORM_L2)
        );
    }
    return ReturnStatus(ReturnCode::Success);
}

std::shared_ptr<Interface>
Interface::getImplementation()
{
    return std::make_shared<CustomImplFRVT11>();
}





