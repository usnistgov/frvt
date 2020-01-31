#include<opencv2/opencv.hpp>


//Normal face landmark for 112x112,
float norm_face[5][2] = {
	{ 30.2946f + 8.0, 51.6963f },
	{ 65.5318f + 8.0, 51.5014f },
	{ 48.0252f + 8.0, 71.7366f },
	{ 33.5493f + 8.0, 92.3655f },
	{ 62.7299f + 8.0, 92.2041f } };

//Normal face landmark for 112x96,
float norm_face_2[5][2] = {
	{ 30.2946f , 51.6963f },
	{ 65.5318f, 51.5014f },
	{ 48.0252f, 71.7366f },
	{ 33.5493f, 92.3655f },
	{ 62.7299f, 92.2041f } };

cv::Mat meanAxis0(const cv::Mat &src)
{
	int num = src.rows;
	int dim = src.cols;

	cv::Mat output(1, dim, CV_32F);
	for (int i = 0; i < dim; i++)
	{
		float sum = 0;
		for (int j = 0; j < num; j++)
		{
			sum += src.at<float>(j, i);
		}
		output.at<float>(0, i) = sum / num;
	}

	return output;
}

cv::Mat elementwiseMinus(const cv::Mat &A, const cv::Mat &B)
{
	cv::Mat output(A.rows, A.cols, A.type());

	assert(B.cols == A.cols);
	if (B.cols == A.cols)
	{
		for (int i = 0; i < A.rows; i++)
		{
			for (int j = 0; j < B.cols; j++)
			{
				output.at<float>(i, j) = A.at<float>(i, j) - B.at<float>(0, j);
			}
		}
	}
	return output;
}

cv::Mat varAxis0(const cv::Mat &src)
{
cv::Mat temp_ = elementwiseMinus(src, meanAxis0(src));
	cv::multiply(temp_, temp_, temp_);
	return meanAxis0(temp_);

}

int MatrixRank(cv::Mat M)
{
	cv::Mat w, u, vt;
	cv::SVD::compute(M, w, u, vt);
	cv::Mat1b nonZeroSingularValues = w > 0.0001;
	int rank = cv::countNonZero(nonZeroSingularValues);
	return rank;

}

//    References
//    ----------
//    .. [1] "Least-squares estimation of transformation parameters between two
//    point patterns", Shinji Umeyama, PAMI 1991, DOI: 10.1109/34.88573
//
//    """
//
//    Anthor:Jack Yu
cv::Mat similarTransform(cv::Mat src, cv::Mat dst) {
	int num = src.rows;
	int dim = src.cols;
	cv::Mat src_mean = meanAxis0(src);
	cv::Mat dst_mean = meanAxis0(dst);
	cv::Mat src_demean = elementwiseMinus(src, src_mean);
	cv::Mat dst_demean = elementwiseMinus(dst, dst_mean);
	cv::Mat A = (dst_demean.t() * src_demean) / static_cast<float>(num);
	cv::Mat d(dim, 1, CV_32F);
	d.setTo(1.0f);
	if (cv::determinant(A) < 0) {
		d.at<float>(dim - 1, 0) = -1;

	}
	cv::Mat T = cv::Mat::eye(dim + 1, dim + 1, CV_32F);
	cv::Mat U, S, V;
	cv::SVD::compute(A, S, U, V);

	// the SVD function in opencv differ from scipy .


	int rank = MatrixRank(A);
	if (rank == 0) {
		assert(rank == 0);

	}
	else if (rank == dim - 1) {
		if (cv::determinant(U) * cv::determinant(V) > 0) {
			T.rowRange(0, dim).colRange(0, dim) = U * V;
		}
		else {
			//            s = d[dim - 1]
			//            d[dim - 1] = -1
			//            T[:dim, :dim] = np.dot(U, np.dot(np.diag(d), V))
			//            d[dim - 1] = s
			int s = d.at<float>(dim - 1, 0) = -1;
			d.at<float>(dim - 1, 0) = -1;

			T.rowRange(0, dim).colRange(0, dim) = U * V;
			cv::Mat diag_ = cv::Mat::diag(d);
			cv::Mat twp = diag_*V; //np.dot(np.diag(d), V.T)
			cv::Mat B = cv::Mat::zeros(3, 3, CV_8UC1);
			cv::Mat C = B.diag(0);
			T.rowRange(0, dim).colRange(0, dim) = U* twp;
			d.at<float>(dim - 1, 0) = s;
		}
	}
	else {
		cv::Mat diag_ = cv::Mat::diag(d);
		cv::Mat twp = diag_*V.t(); //np.dot(np.diag(d), V.T)
		cv::Mat res = U* twp; // U
		T.rowRange(0, dim).colRange(0, dim) = -U.t()* twp;
	}
	cv::Mat var_ = varAxis0(src_demean);
	float val = cv::sum(var_).val[0];
	cv::Mat res;
	cv::multiply(d, S, res);
	float scale = 1.0 / val*cv::sum(res).val[0];
	T.rowRange(0, dim).colRange(0, dim) = -T.rowRange(0, dim).colRange(0, dim).t();
	cv::Mat  temp1 = T.rowRange(0, dim).colRange(0, dim); // T[:dim, :dim]
	cv::Mat  temp2 = src_mean.t(); //src_mean.T
	cv::Mat  temp3 = temp1*temp2; // np.dot(T[:dim, :dim], src_mean.T)
	cv::Mat temp4 = scale*temp3;
	T.rowRange(0, dim).colRange(dim, dim + 1) = -(temp4 - dst_mean.t());
	T.rowRange(0, dim).colRange(0, dim) *= scale;
	return T;
}
