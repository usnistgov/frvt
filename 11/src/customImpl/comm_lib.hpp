#pragma once
#define NMS_UNION 1
#define NMS_MIN  2

struct scale_window
{
	int h;
	int w;
	float scale;
};

void nms_boxes(std::vector<face_box>& input, float threshold, int type, std::vector<face_box>&output);

void regress_boxes(std::vector<face_box>& rects);

void square_boxes(std::vector<face_box>& rects);

void padding(int img_h, int img_w, std::vector<face_box>& rects);

void process_boxes(std::vector<face_box>& input, int img_h, int img_w, std::vector<face_box>& rects);

void generate_bounding_box(const float * confidence_data, int confidence_size,
	const float * reg_data, float scale, float threshold,
	int feature_h, int feature_w, std::vector<face_box>&  output, bool transposed);


void set_input_buffer(std::vector<cv::Mat>& input_channels,
	float* input_data, const int height, const int width);


void  cal_pyramid_list(int height, int width, int min_size, float factor, std::vector<scale_window>& list);

void cal_landmark(std::vector<face_box>& box_list);

void set_box_bound(std::vector<face_box>& box_list, int img_h, int img_w);

int make_round(float num);

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);