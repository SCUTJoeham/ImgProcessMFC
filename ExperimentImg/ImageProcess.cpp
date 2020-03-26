#include "stdafx.h"
#include "ImageProcess.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

const double PI = acos(-1);

const int fourier_factor = 14;

const float cubic_a = -0.5; //BiCubic基函数

bool ImageProcess::GetValue(int p[], int size, int &value)
{
	//数组中间的值
	int zxy = p[(size - 1) / 2];
	//用于记录原数组的下标
	int *a = new int[size];
	int index = 0;
	for (int i = 0; i<size; ++i)
		a[index++] = i;

	for (int i = 0; i<size - 1; i++)
		for (int j = i + 1; j<size; j++)
			if (p[i]>p[j]) {
				int tempA = a[i];
				a[i] = a[j];
				a[j] = tempA;
				int temp = p[i];
				p[i] = p[j];
				p[j] = temp;

			}
	int zmax = p[size - 1];
	int zmin = p[0];
	int zmed = p[(size - 1) / 2];

	if (zmax>zmed&&zmin<zmed) {
		if (zxy>zmin&&zxy<zmax)
			value = (size - 1) / 2;
		else
			value = a[(size - 1) / 2];
		delete[]a;
		return true;
	}
	else {
		delete[]a;
		return false;
	}

}

void ImageProcess::getW(float w[4], float p) {
	int P = (int)p;//取整数部分
	float stemp_p[4];
	stemp_p[0] = 1 + (p - P);
	stemp_p[1] = p - P;
	stemp_p[2] = 1 - (p - P);
	stemp_p[3] = 2 - (p - P);

	w[0] = cubic_a * abs(stemp_p[0] * stemp_p[0] * stemp_p[0]) - 5 * cubic_a*stemp_p[0] * stemp_p[0] + 8 * cubic_a*abs(stemp_p[0]) - 4 * cubic_a;
	w[1] = (cubic_a + 2)*abs(stemp_p[1] * stemp_p[1] * stemp_p[1]) - (cubic_a + 3)*stemp_p[1] * stemp_p[1] + 1;
	w[2] = (cubic_a + 2)*abs(stemp_p[2] * stemp_p[2] * stemp_p[2]) - (cubic_a + 3)*stemp_p[2] * stemp_p[2] + 1;
	w[3] = cubic_a * abs(stemp_p[3] * stemp_p[3] * stemp_p[3]) - 5 * cubic_a*stemp_p[3] * stemp_p[3] + 8 * cubic_a*abs(stemp_p[3]) - 4 * cubic_a;
}

double ImageProcess::genGaussNoise(double mu, double sigma)
{
	//定义最小值
	double epsilon = std::numeric_limits<double>::min();
	double z0 = 0, z1 = 0;
	bool flag = false;
	flag = !flag;
	if (!flag)
		return z1 * sigma + mu;
	double u1, u2;
	do
	{
		u1 = rand()*(1.0 / RAND_MAX);
		u2 = rand()*(1.0 / RAND_MAX);
	} while (u1 <= epsilon);
	z0 = sqrt(-2.0*log(u1))*cos(2 * CV_PI*u2);
	z1 = sqrt(-2.0*log(u1))*sin(2 * CV_PI*u2);
	return z0 * sigma + mu;
}

void ImageProcess::generate_gauss_mask(int kernel_size, double sigma, double * gauss_mask) {
	int height = kernel_size;
	int width = kernel_size;
	int center_height = (height - 1) / 2;
	int center_width = (width - 1) / 2;
	double sum = 0.0;
	double x, y;
	for (int i = 0; i < height; ++i) {
		y = pow(i - center_height, 2);
		for (int j = 0; j < width; ++j) {
			x = pow(j - center_width, 2);
			double g = exp(-(x + y) / (2 * pow(sigma, 2)));
			gauss_mask[i*kernel_size + j] = g;
			sum += g;
		}
	}
	int kernel_square_size = kernel_size * kernel_size;
	for (int i = 0; i < kernel_square_size; i++) {
		gauss_mask[i] /= sum;
	}
}

void ImageProcess::generate_color_mask(double sigma, double *color_mask) {
	for (int i = 0; i < 256; ++i) color_mask[i] = exp(-(i*i) / (2 * pow(sigma, 2)));
}


UINT ImageProcess::medianFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);

	byte* pRealData = (byte*)param->src->GetBits();
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int *pixel = new int[maxLength];//存储每个像素点的灰度
	int *pixelR = new int[maxLength];
	int *pixelB = new int[maxLength];
	int *pixelG = new int[maxLength];
	int index = 0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int Sxy = 1;
		int med = 0;
		int state = 0;
		int x = i % maxWidth;
		int y = i / maxWidth;
		while (Sxy <= maxSpan)
		{
			index = 0;
			for (int tmpY = y - Sxy; tmpY <= y + Sxy && tmpY <maxHeight; tmpY++)
			{
				if (tmpY < 0) continue;
				for (int tmpX = x - Sxy; tmpX <= x + Sxy && tmpX<maxWidth; tmpX++)
				{
					if (tmpX < 0) continue;
					if (bitCount == 1)
					{
						pixel[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixelR[index++] = pixel[index];

					}
					else
					{
						pixelR[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 2);
						pixelG[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 1);
						pixelB[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);

					}
				}

			}
			if (index <= 0)
				break;
			if ((state = GetValue(pixel, index, med)) == 1)
				break;

			Sxy++;
		};

		if (state)
		{
			if (bitCount == 1)
			{
				*(pRealData + pit*y + x*bitCount) = pixelR[med];

			}
			else
			{
				*(pRealData + pit*y + x*bitCount + 2) = pixelR[med];
				*(pRealData + pit*y + x*bitCount + 1) = pixelG[med];
				*(pRealData + pit*y + x*bitCount) = pixelB[med];

			}
		}

	}



	delete[]pixel;
	delete[]pixelR;
	delete[]pixelG;
	delete[]pixelB;

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEDIAN_FILTER, 1, NULL);
	return 0;
}

UINT ImageProcess::addNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		if ((rand() % 1000) * 0.001 < NOISE)
		{
			int value = 0;
			if (rand() % 1000 < 500)
			{
				value = 0;
			}
			else
			{
				value = 255;
			}
			if (bitCount == 1)
			{
				*(pRealData + pit * y + x * bitCount) = value;
			}
			else
			{
				*(pRealData + pit * y + x * bitCount) = value;
				*(pRealData + pit * y + x * bitCount + 1) = value;
				*(pRealData + pit * y + x * bitCount + 2) = value;
			}
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::resize(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxSrcWidth = param->src_cv.cols;
	int maxSrcHeight = param->src_cv.rows;
	int maxDstWidth = param->dst_cv.cols;
	int maxDstHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxDstWidth;
		int i = p / maxDstWidth;

		//放大后的图像的像素位置相对于源图像的位置
		float x = i * (maxSrcHeight*1.0 / maxDstHeight);
		float y = j * (maxSrcWidth*1.0 / maxDstWidth);

		//行列方向的加权系数
		float w_x[4], w_y[4];
		getW(w_x, x);
		getW(w_y, y);

		cv::Vec3f temp = { 0, 0, 0 };

		for (int s = 0; s <= 3; s++) {
			for (int t = 0; t <= 3; t++) {
				int temp_x = int(x) + s - 1;
				int temp_y = int(y) + t - 1;
				if (temp_x > -1 && temp_y > -1 && temp_x < maxSrcHeight && temp_y < maxSrcWidth) {
					temp = temp + (cv::Vec3f)(param->src_cv.at<cv::Vec3b>(temp_x, temp_y)) * w_x[s] * w_y[t];
				}
			}
		}
		param->dst_cv.at<cv::Vec3b>(i, j) = (cv::Vec3b)temp;
	}

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_RESIZE, 1, NULL);
	return 0;
	
}

UINT ImageProcess::rotate(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->dst_cv.cols;
	int maxHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	double rotate_radian = param->r_angel * PI / 180;

	for (int p = startIndex; p <= endIndex; ++p)
	{
		
		int i = p / maxWidth;
		int j = p % maxWidth;
		float x = (int)((i - param->dst_cv.rows / 2)*cos(rotate_radian) - (j - param->dst_cv.cols / 2)*sin(rotate_radian) + 0.5);
		float y = (int)((i - param->dst_cv.rows / 2)*sin(rotate_radian) + (j - param->dst_cv.cols / 2)*cos(rotate_radian) + 0.5);
		x += param->src_cv.rows / 2;
		y += param->src_cv.cols / 2;

		float w_x[4], w_y[4];
		getW(w_x, x);
		getW(w_y, y);

		cv::Vec3f temp = { 0, 0, 0 };

		for (int s = 0; s <= 3; s++) {
			for (int t = 0; t <= 3; t++) {
				int temp_x = int(x) + s - 1;
				int temp_y = int(y) + t - 1;
				if (temp_x > -1 && temp_y > -1 && temp_x < param->src_cv.rows && temp_y < param->src_cv.cols) {
					temp = temp + (cv::Vec3f)(param->src_cv.at<cv::Vec3b>(temp_x, temp_y)) * w_x[s] * w_y[t];
				}
			}
		}

		if (x >= param->src_cv.rows || y >= param->src_cv.cols || x <= 0 || y <= 0) {
			param->dst_cv.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
		}
		else {
			param->dst_cv.at<cv::Vec3b>(i, j) = (cv::Vec3b)temp;
		}
	}

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_ROTATE, 1, NULL);
	return 0;

}

UINT ImageProcess::fourierTrans(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxSrcWidth = param->src_cv.cols;
	int maxSrcHeight = param->src_cv.rows;
	int maxDstWidth = param->dst_cv.cols;
	int maxDstHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	for (int p = startIndex; p <= endIndex; p++)
	{
		int j = p % maxDstWidth;
		int i = p / maxDstWidth;

		double real = 0.0, imag = 0.0;

		for (int x = 0; x < maxSrcHeight; x++)
		{
			for (int y = 0; y < maxSrcWidth; y++)
			{
				cv::Vec3b pixel = param->src_cv.at<cv::Vec3b>(x, y);
				double gray = 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
				if ((x + y) & 1) // centralize
					gray = -gray;
				double A = 2 * PI * ((double)i * (double)x / (double)maxSrcHeight + (double)j * (double)y / (double)maxSrcWidth);
				real += gray * cos(A);
				imag -= gray * sin(A);
			}
		}
		double mag = sqrt(real * real + imag * imag);
		mag = fourier_factor * log(mag + 1);
		mag < 0 ? mag = 0 : (mag > 255 ? mag = 255 : mag = mag);
		for (int channel = 0; channel < 3; channel++)
			param->dst_cv.at<cv::Vec3b>(i, j)[channel] = mag;
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_FOURIER_TRANS, 1, NULL);
	return 0;

}

UINT ImageProcess::gaussianNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxHeight = param->dst_cv.rows;
	int maxWidth = param->dst_cv.cols;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int y = i % maxWidth;
		int x = i / maxWidth;

		int temp_r = param->dst_cv.at<cv::Vec3b>(x, y)[2] + genGaussNoise(param->MU, param->SIGMA) * 32;
		param->dst_cv.at<cv::Vec3b>(x, y)[2] = temp_r < 0 ? 0 : (temp_r > 255 ? 255 : temp_r);

		int temp_g = param->dst_cv.at<cv::Vec3b>(x, y)[1] + genGaussNoise(param->MU, param->SIGMA) * 32;
		param->dst_cv.at<cv::Vec3b>(x, y)[1] = temp_g < 0 ? 0 : (temp_g > 255 ? 255 : temp_g);

		int temp_b = param->dst_cv.at<cv::Vec3b>(x, y)[0] + genGaussNoise(param->MU, param->SIGMA) * 32;
		param->dst_cv.at<cv::Vec3b>(x, y)[0] = temp_b < 0 ? 0 : (temp_b > 255 ? 255 : temp_b);

	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_GAUSSIAN_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::smoothFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->dst_cv.cols;
	int maxHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	int channel_num = param->dst_cv.channels();

	int kernel_size = param->kernel_size;
	int kernel_half_size = kernel_size / 2;
	int kernel_square_size = kernel_size * kernel_size;

	CV_Assert(kernel_size % 2 == 1);

	int *kernel = new int[kernel_size * kernel_size];
	for (int i = 0; i < kernel_size * kernel_size; i++)
		kernel[i] = 1;

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxWidth;
		int i = p / maxWidth;
		cv::Vec3f kernel_sum = { 0, 0, 0 };
		for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
			for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++) {
				if (x > -1 && y > -1 && x < maxHeight && y < maxWidth) {
					kernel_sum += (cv::Vec3f)param->src_cv.at<cv::Vec3b>(x, y);
				}
			}

		}
		param->dst_cv.at<cv::Vec3b>(i, j) = (cv::Vec3b)(kernel_sum / kernel_square_size);

	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_SMOOTH_FILTER, 1, NULL);
	return 0;

}

UINT ImageProcess::gaussianFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->dst_cv.cols;
	int maxHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	int channel_num = param->dst_cv.channels();

	int kernel_size = param->kernel_size;
	int kernel_half_size = kernel_size / 2;
	int kernel_square_size = kernel_size * kernel_size;

	double *gauss_mask = param->gauss_mask;

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxWidth;
		int i = p / maxWidth;
		cv::Vec3f kernel_sum = { 0, 0, 0 };
		int gauss_mask_index = 0;
		
		for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
			for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++) {
				if (x > -1 && y > -1 && x < maxHeight && y < maxWidth) {
					kernel_sum += (cv::Vec3f)param->src_cv.at<cv::Vec3b>(x, y) * gauss_mask[gauss_mask_index];
				}
				gauss_mask_index++;
			}
		}
		param->dst_cv.at<cv::Vec3b>(i, j) = (cv::Vec3b)kernel_sum;
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_GAUSSIAN_FILTER, 1, NULL);
	return 0;

}

UINT ImageProcess::wienerFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxDstWidth = param->dst_cv.cols;
	int maxDstHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	int kernel_size = param->kernel_size;
	int kernel_half_size = kernel_size / 2;
	int kernel_square_size = kernel_size * kernel_size;

	int len = endIndex - startIndex + 1;
	double noise[3];
	double **mean = new double *[3];
	double **variance = new double *[3];

	for (int channel = 0; channel < 3; channel++)
	{
		mean[channel] = new double[len];
		variance[channel] = new double[len];
	}

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxDstWidth;
		int i = p / maxDstWidth;
		int offset = (i * maxDstWidth + j - startIndex);

		for (int channel = 0; channel < 3; channel++)
		{
			mean[channel][offset] = 0.0;
			variance[channel][offset] = 0.0;
			for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
				for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++) {
					if (x > -1 && y > -1 && x < maxDstHeight && y < maxDstWidth) {
						mean[channel][offset] += param->src_cv.at<cv::Vec3b>(x, y)[channel];
					}
				}
			}
			mean[channel][offset] /= kernel_square_size;

			for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
				for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++) {
					if (x > -1 && y > -1 && x < maxDstHeight && y < maxDstWidth) {
						variance[channel][offset] += pow(param->src_cv.at<cv::Vec3b>(x, y)[channel] - mean[channel][offset], 2.0);
					}
				}
			}
			variance[channel][offset] /= kernel_square_size;

			noise[channel] += variance[channel][offset];
		}

	}

	for (int ch = 0; ch < 3; ++ch)
		noise[ch] /= len;

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxDstWidth;
		int i = p / maxDstWidth;
		int offset = (i * maxDstWidth + j - startIndex);
		double rgb[3];

		for (int channel = 0; channel < 3; channel++)
		{
			rgb[channel] = param->src_cv.at<cv::Vec3b>(i, j)[channel] - mean[channel][offset];
			double t = variance[channel][offset] - noise[channel];
			if (t < 0.0) t = 0.0;
			variance[channel][offset] = fmax(variance[channel][offset], noise[channel]);
			rgb[channel] = rgb[channel] / variance[channel][offset] * t + mean[channel][offset];
		}
		for (int channel = 0; channel < 3; channel++)
			param->dst_cv.at<cv::Vec3b>(i, j)[channel] = rgb[channel];
	}

	for (int ch = 0; ch < 3; ++ch)
	{
		delete mean[ch];
		delete variance[ch];
	}
	delete[] mean;
	delete[] variance;

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_WIENER_FILTER, 1, NULL);
	return 0;

}

UINT ImageProcess::bilateralFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	int maxWidth = param->dst_cv.cols;
	int maxHeight = param->dst_cv.rows;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;

	int channel_num = param->dst_cv.channels();

	int kernel_size = param->kernel_size;
	int kernel_half_size = kernel_size / 2;
	int kernel_square_size = kernel_size * kernel_size;

	double *mask0, *mask1, *mask2;
	mask0 = new double[kernel_square_size];
	mask1 = new double[kernel_square_size];
	mask2 = new double[kernel_square_size];

	for (int p = startIndex; p <= endIndex; ++p)
	{
		int j = p % maxWidth;

		int i = p / maxWidth;

		int mask_index = 0;

		double sum[3] = { 0 };
		int gray_diff[3] = { 0 };
		double space_color_sum[3] = { 0.0 };

		for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
			for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++)
			{
				if (x > -1 && y > -1 && x < maxHeight && y < maxWidth) {
					gray_diff[0] = abs(param->src_cv.at<cv::Vec3b>(x, y)[0] - param->src_cv.at<cv::Vec3b>(i, j)[0]);
					gray_diff[1] = abs(param->src_cv.at<cv::Vec3b>(x, y)[1] - param->src_cv.at<cv::Vec3b>(i, j)[1]);
					gray_diff[2] = abs(param->src_cv.at<cv::Vec3b>(x, y)[2] - param->src_cv.at<cv::Vec3b>(i, j)[2]);
					mask0[mask_index] = param->color_mask[gray_diff[0]] * param->gauss_mask[mask_index];
					mask1[mask_index] = param->color_mask[gray_diff[1]] * param->gauss_mask[mask_index];
					mask2[mask_index] = param->color_mask[gray_diff[2]] * param->gauss_mask[mask_index];
					space_color_sum[0] += mask0[mask_index];
					space_color_sum[1] += mask1[mask_index];
					space_color_sum[2] += mask2[mask_index];
				}
				mask_index++;
			}
		}

		if (channel_num == 1) {
			for (int k = 0; k < kernel_square_size; k++)
				mask0[k] /= space_color_sum[0];
		}
		else {
			for (int k = 0; k < kernel_square_size; k++) {
				mask0[k] /= space_color_sum[0];
				mask1[k] /= space_color_sum[1];
				mask2[k] /= space_color_sum[2];
			}
		}

		mask_index = 0;
		for (int x = i - kernel_half_size; x <= i + kernel_half_size; x++) {
			for (int y = j - kernel_half_size; y <= j + kernel_half_size; y++) {
				if (x > -1 && y > -1 && x < maxHeight && y < maxWidth) {
					if (channel_num == 1) {
						sum[0] = sum[0] + param->src_cv.at<uchar>(x, y) * mask0[mask_index];
					}
					else if (channel_num == 3) {
						cv::Vec3b bgr = param->src_cv.at<cv::Vec3b>(x, y);
						sum[0] += bgr[0] * mask0[mask_index]; // B
						sum[1] += bgr[1] * mask1[mask_index]; // G
						sum[2] += bgr[2] * mask2[mask_index]; // R
					}
				}
				mask_index++;
			}

		}

		for (int k = 0; k < channel_num; k++) {
			if (sum[k] < 0)
				sum[k] = 0;
			else if (sum[k] > 255)
				sum[k] = 255;
		}

		if (channel_num == 1) {
			param->dst_cv.at<uchar>(i, j) = static_cast<uchar>(sum[0]);
		}
		else if (channel_num == 3) {
			cv::Vec3b bgr = { static_cast<uchar>(sum[0]), static_cast<uchar>(sum[1]), static_cast<uchar>(sum[2]) };
			// param->dst.at<cv::Vec3b>(i, j) = bgr;
			param->dst_cv.at<cv::Vec3b>(i, j)[2] = sum[2];
			param->dst_cv.at<cv::Vec3b>(i, j)[1] = sum[1];
			param->dst_cv.at<cv::Vec3b>(i, j)[0] = sum[0];
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_BILATERAL_FILTER, 1, NULL);
	return 0;

}



