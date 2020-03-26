#include <opencv2\opencv.hpp>

#pragma once
#pragma once
#define NOISE 0.2
struct ThreadParam
{
	CImage * src;
	CImage * dst;
	cv::Mat src_cv;
	cv::Mat dst_cv;
	int startIndex;
	int endIndex;
	int maxSpan;//为模板中心到边缘的距离
	double MU;
	double SIGMA;
	double r_angel;
	int kernel_size;
	double gm_sigma;
	double *gauss_mask;
	double *color_mask;
};

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT addNoise(LPVOID param);
	static UINT resize(LPVOID param);
	static UINT rotate(LPVOID p);
	static UINT fourierTrans(LPVOID p);
	static UINT gaussianNoise(LPVOID p);
	static UINT smoothFilter(LPVOID p);
	static UINT gaussianFilter(LPVOID p);
	static UINT wienerFilter(LPVOID p);
	static UINT bilateralFilter(LPVOID p);

	static bool GetValue(int p[], int size, int &value);
	static void getW(float w_x[4], float x);
	static double genGaussNoise(double mu, double sigma);
	static void generate_gauss_mask(int kernel_size, double sigma, double *gauss_mask);
	static void generate_color_mask(double sigma, double *color_mask);
};