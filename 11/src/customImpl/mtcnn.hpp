#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

struct face_landmark
{
	float x[5];
	float y[5];
};

struct face_box
{
	float x0;
	float y0;
	float x1;
	float y1;

	/* confidence score */
	float score;

	/*regression scale */

	float regress[4];

	/* padding stuff*/
	float px0;
	float py0;
	float px1;
	float py1;

	face_landmark landmark;
};



class Mtcnn {
public:
	Mtcnn(void) {
		min_size_ = 50;
		pnet_threshold_ = 0.6;
		rnet_threshold_ = 0.7;
		onet_threshold_ = 0.9;
		//factor_ = 0.709;
		factor_ = 0.5;

	}

	void SetThreshold(float p, float r, float o)
	{
		pnet_threshold_ = p;
		rnet_threshold_ = r;
		onet_threshold_ = o;
	}

	void SetFactorMinSize(float factor, float min_size)
	{
		factor_ = factor;
		min_size_ = min_size;
	}


	virtual int LoadModule(const std::string& model_dir) = 0;
	virtual void Detect(cv::Mat& img, std::vector<face_box>& face_list) = 0;
	virtual ~Mtcnn(void) {};

protected:

	int min_size_;
	float pnet_threshold_;
	float rnet_threshold_;
	float onet_threshold_;
	float factor_;
};

