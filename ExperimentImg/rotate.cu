#ifndef  __ROTATE_CU_
#define  __ROTATE_CU_

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <opencv2\opencv.hpp>
#include <time.h>
#include <iostream>

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}

__global__ void Rotate(uchar* _src_dev, uchar * _dst_dev, int _src_step, int _dst_step,
	int _src_rows, int _src_cols, int _dst_rows, int _dst_cols, double rr)
{
	int i = blockIdx.x;
	int j = blockIdx.y;

	double x = (int)((i - _dst_rows / 2)*cos(rr) - (j - _dst_cols / 2)*sin(rr) + 0.5);
	double y = (int)((i - _dst_rows / 2)*sin(rr) + (j - _dst_cols / 2)*cos(rr) + 0.5);

	x += _src_rows / 2;
	y += _src_cols / 2;

	int px = (int)x;
	int py = (int)y;
	double w_x[4], w_y[4];

	const double cubic_a = -0.5;
	double stemp_x[4], stemp_y[4];
	stemp_x[0] = 1 + (x - px); stemp_x[1] = x - px; stemp_x[2] = 1 - (x - px); stemp_x[3] = 2 - (x - px);
	w_x[0] = cubic_a * abs(stemp_x[0] * stemp_x[0] * stemp_x[0]) - 5 * cubic_a*stemp_x[0] * stemp_x[0] + 8 * cubic_a*abs(stemp_x[0]) - 4 * cubic_a;
	w_x[1] = (cubic_a + 2)*abs(stemp_x[1] * stemp_x[1] * stemp_x[1]) - (cubic_a + 3)*stemp_x[1] * stemp_x[1] + 1;
	w_x[2] = (cubic_a + 2)*abs(stemp_x[2] * stemp_x[2] * stemp_x[2]) - (cubic_a + 3)*stemp_x[2] * stemp_x[2] + 1;
	w_x[3] = cubic_a * abs(stemp_x[3] * stemp_x[3] * stemp_x[3]) - 5 * cubic_a*stemp_x[3] * stemp_x[3] + 8 * cubic_a*abs(stemp_x[3]) - 4 * cubic_a;

	stemp_y[0] = 1 + (y - py); stemp_y[1] = y - py; stemp_y[2] = 1 - (y - py); stemp_y[3] = 2 - (y - py);
	w_y[0] = cubic_a * abs(stemp_y[0] * stemp_y[0] * stemp_y[0]) - 5 * cubic_a*stemp_y[0] * stemp_y[0] + 8 * cubic_a*abs(stemp_y[0]) - 4 * cubic_a;
	w_y[1] = (cubic_a + 2)*abs(stemp_y[1] * stemp_y[1] * stemp_y[1]) - (cubic_a + 3)*stemp_y[1] * stemp_y[1] + 1;
	w_y[2] = (cubic_a + 2)*abs(stemp_y[2] * stemp_y[2] * stemp_y[2]) - (cubic_a + 3)*stemp_y[2] * stemp_y[2] + 1;
	w_y[3] = cubic_a * abs(stemp_y[3] * stemp_y[3] * stemp_y[3]) - 5 * cubic_a*stemp_y[3] * stemp_y[3] + 8 * cubic_a*abs(stemp_y[3]) - 4 * cubic_a;

	double tmp_r = 0.0, tmp_g = 0.0, tmp_b = 0.0;
	for (int s = 0; s <= 3; s++) {
		for (int t = 0; t <= 3; t++) {
			int temp_x = px + s - 1;
			int temp_y = py + t - 1;
			if (temp_x > -1 && temp_y > -1 && temp_x < _src_rows && temp_y < _src_cols) {
				tmp_r += *(_src_dev + temp_x * _src_step + 3 * temp_y + 2) * w_x[s] * w_y[t];
				tmp_g += *(_src_dev + temp_x * _src_step + 3 * temp_y + 1) * w_x[s] * w_y[t];
				tmp_b += *(_src_dev + temp_x * _src_step + 3 * temp_y) * w_x[s] * w_y[t];
			}
		}
	}
	if (x >= _src_rows || y >= _src_cols || x <= 0 || y <= 0)
	{
		*(_dst_dev + i * _dst_step + 3 * j + 2) = 0;
		*(_dst_dev + i * _dst_step + 3 * j + 1) = 0;
		*(_dst_dev + i * _dst_step + 3 * j) = 0;
	}
	else {
		*(_dst_dev + i * _dst_step + 3 * j + 2) = (uchar)tmp_r;
		*(_dst_dev + i * _dst_step + 3 * j + 1) = (uchar)tmp_g;
		*(_dst_dev + i * _dst_step + 3 * j) = (uchar)tmp_b;
	}
	
}

extern "C" void Rotate_host(const cv::Mat &src, cv::Mat &dst, const cv::Size &dst_s, double r_angel)
{
	uchar *src_data = src.data;
	int srcWidth = src.cols;
	int srcHeight = src.rows;
	int src_step = src.step;
	int dst_step = dst.step;
	uchar *src_dev, *dst_dev;

	const double PI = acos(-1);
	double rotate_radian = r_angel * PI / 180;

	checkCudaErrors(cudaMalloc((void**)&src_dev, 3 * srcWidth * srcHeight * sizeof(uchar)));
	checkCudaErrors(cudaMalloc((void**)&dst_dev, 3 * dst_s.width * dst_s.height * sizeof(uchar)));

	checkCudaErrors(cudaMemcpy(src_dev, src_data, 3 * srcWidth * srcHeight * sizeof(uchar), cudaMemcpyHostToDevice));

	dim3 grid(dst_s.height, dst_s.width);

	Rotate << < grid, 1 >> > (src_dev, dst_dev, src_step, dst_step, srcHeight, srcWidth, dst_s.height, dst_s.width, rotate_radian);

	checkCudaErrors(cudaMemcpy(dst.data, dst_dev, 3 * dst_s.width * dst_s.height * sizeof(uchar), cudaMemcpyDeviceToHost));

	cudaFree(src_dev);
	cudaFree(dst_dev);

}

#endif // ! __RESIZE_KERNEL_CU_