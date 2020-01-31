#pragma once
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#include "mxnet/c_predict_api.h"
#include "mxnet-cpp/MxNetCpp.h"
#include "comm_lib.hpp"
#include "buffer_file.hpp"

using namespace mxnet::cpp;

//compare input features with labeled features, get classification result with minimal distance and class index
struct class_info
{
	double min_distance;
	int index;
};

class_info classify(const cv::Mat& img, const  cv::Mat& cmp)
{
	int rows = cmp.rows;
	cv::Mat broad;
	cv::repeat(img, rows, 1, broad);

	broad = broad - cmp;
	cv::pow(broad,2,broad);
	cv::reduce(broad, broad, 1, cv::REDUCE_SUM);

	double dis;
	cv::Point point;
	cv::minMaxLoc(broad, &dis, 0, &point, 0);

	return class_info{dis, point.y};
}


class Mxnet_extract 
{
public:
	~Mxnet_extract()
	{
		if(pred_feature)
		    MXPredFree(pred_feature);
	}
	int LoadModel(const std::string & fname, std::vector<char>& buf)
	{
		std::ifstream fs(fname, std::ios::binary | std::ios::in);

		if (!fs.good())
		{
			std::cerr << fname << " does not exist" << std::endl;
			return -1;
		}

		fs.seekg(0, std::ios::end);
		int fsize = fs.tellg();

		fs.seekg(0, std::ios::beg);
		buf.resize(fsize);
		fs.read(buf.data(), fsize);

		fs.close();

		return 0;

	}

	int LoadExtractModule(const std::string& param_file, const std::string& json_file,
		int batch, int channel, int input_h, int input_w)
	{

		std::vector<char> param_buffer;
		std::vector<char> json_buffer;

		if (LoadModel(param_file, param_buffer)<0)
			return -1;

		if (LoadModel(json_file, json_buffer)<0)
			return -1;

		int device_type = 1;
		int dev_id = 0;
		mx_uint  num_input_nodes = 1;
		const char * input_keys[1];
		const mx_uint input_shape_indptr[] = { 0, 4 };
		const mx_uint input_shape_data[] = {
			static_cast<mx_uint>(batch),
			static_cast<mx_uint>(channel),
			static_cast<mx_uint>(input_h),
			static_cast<mx_uint>(input_w)
		};

		input_keys[0] = "data";

		int ret = MXPredCreate(json_buffer.data(),
			param_buffer.data(),
			param_buffer.size(),
			device_type,
			dev_id,
			num_input_nodes,
			input_keys,
			input_shape_indptr,
			input_shape_data,
			&pred_feature
		);
		
		return ret;
	}


	cv::Mat extractFeature(const cv::Mat& img)
	{

		int width = img.cols;
		int height = img.rows;

        cv::Mat img_rgb(height, width, CV_32FC3);
		img.convertTo(img_rgb, CV_32FC3);
		cv::cvtColor(img_rgb, img_rgb, cv::COLOR_BGR2RGB);

		std::vector<float> input(3 * height * width);
		std::vector<cv::Mat> input_channels;
		
		set_input_buffer(input_channels, input.data(), height, width);
		cv::split(img_rgb, input_channels);

		MXPredSetInput(pred_feature, "data", input.data(), input.size());
		MXPredForward(pred_feature);

		mx_uint *shape = NULL;
		mx_uint shape_len = 0;

		MXPredGetOutputShape(pred_feature, 0, &shape, &shape_len);

		int feature_size = 1;
		for (unsigned int i = 0;i<shape_len;i++)
			feature_size *= shape[i];
		std::vector<float> feature(feature_size);

		MXPredGetOutput(pred_feature, 0, feature.data(), feature_size);
		
		cv::Mat output = cv::Mat(feature, true).reshape(1, 1);
		cv::normalize(output, output);
	
		return output;
	}

private:
	PredictorHandle pred_feature;
};

void recognition(MxNetMtcnn& mtcnn, Mxnet_extract& extract, cv::Mat& img, const cv::Mat& data, const std::vector<std::string> labels)
{

	cv::Mat src(5, 2, CV_32FC1, norm_face);
	
	std::vector<face_box> face_info;

	//detect with mtcnn, get detect reuslt with bounding box and landmark point
	mtcnn.Detect(img, face_info);

	for (int i = 0; i < face_info.size(); ++i)
	{
		NDArray::WaitAll();
			
	    face_box face = face_info[i];

		float v2[5][2] =
			{ { face.landmark.x[0] , face.landmark.y[0] },
			{ face.landmark.x[1] , face.landmark.y[1] },
			{ face.landmark.x[2] , face.landmark.y[2] },
			{ face.landmark.x[3] , face.landmark.y[3] },
			{ face.landmark.x[4] , face.landmark.y[4] } };

		cv::Mat dst(5, 2, CV_32FC1, v2);

		//do similar transformation according normal face
		cv::Mat m = similarTransform(dst, src);

		cv::Mat aligned(112, 112, CV_32FC3);
		cv::Size size(112, 112);

		//get aligned face with transformed matrix and resize to 112*112
		cv::Mat transfer = m(cv::Rect(0, 0, 3, 2));		
		cv::warpAffine(img, aligned, transfer, size, 1, 0, 0);
	
		//extract feature from aligned face and do classification with labels 
		cv::Mat output = extract.extractFeature(aligned);
        class_info result = classify(output, data);

		//draw landmark points
		for (int j = 0; j < 5; j++)
		{
			cv::Point p(face.landmark.x[j], face.landmark.y[j]);
			cv::circle(img, p, 2, cv::Scalar(0, 0, 255), -1);
		}

		//draw bound ing box
		cv::Point pt1(face.x0, face.y0);
		cv::Point pt2(face.x1, face.y1);
		cv::rectangle(img, pt1, pt2, cv::Scalar(0, 255, 0), 2);

		cv::Point pt3(face.x0, face.y0 - 10);
		if(result.min_distance < 1.05)
		//show classification name
		cv::putText(img, labels[result.index], pt3, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0));
	}
		
}
