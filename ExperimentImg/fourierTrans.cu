#ifndef  __FOURIERTRANS_CU_
#define  __FOURIERTRANS_CU_

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

__global__ void FourierTrans(uchar* _src_dev, uchar * _dst_dev, int _src_step, int _dst_step,
	int _src_rows, int _src_cols, int _dst_rows, int _dst_cols)
{
	int i = blockIdx.x;
	int j = blockIdx.y;

	const double PI = 3.1415926535;
	const int fourier_factor = 14;
	double real = 0.0, imag = 0.0;

	for (int x = 0; x < _src_rows; x++)
	{
		for (int y = 0; y < _src_cols; y++)
		{
			uchar pixel_r = *(_src_dev + x * _src_step + 3 * y + 2);
			uchar pixel_g = *(_src_dev + x * _src_step + 3 * y + 1);
			uchar pixel_b = *(_src_dev + x * _src_step + 3 * y);
			double gray = 0.299 * pixel_r + 0.587 * pixel_g + 0.114 * pixel_b;
			if ((x + y) & 1) // centralize
				gray = -gray;
			double A = 2 * PI * ((double)i * (double)x / (double)_src_rows + (double)j * (double)y / (double)_src_cols);
			real += gray * cos(A);
			imag -= gray * sin(A);
		}
	}
	double mag = sqrt(real * real + imag * imag);
	mag = fourier_factor * log(mag + 1);
	mag < 0 ? mag = 0 : (mag > 255 ? mag = 255 : mag = mag);

	*(_dst_dev + i * _dst_step + 3 * j + 2) = (uchar)mag;
	*(_dst_dev + i * _dst_step + 3 * j + 1) = (uchar)mag;
	*(_dst_dev + i * _dst_step + 3 * j) = (uchar)mag;

}

extern "C" void FourierTrans_host(const cv::Mat &src, cv::Mat &dst, const cv::Size &dst_s)
{
	uchar *src_data = src.data;
	int srcWidth = src.cols;
	int srcHeight = src.rows;
	int src_step = src.step;
	int dst_step = dst.step;
	uchar *src_dev, *dst_dev;


	checkCudaErrors(cudaMalloc((void**)&src_dev, 3 * srcWidth * srcHeight * sizeof(uchar)));
	checkCudaErrors(cudaMalloc((void**)&dst_dev, 3 * dst_s.width * dst_s.height * sizeof(uchar)));

	checkCudaErrors(cudaMemcpy(src_dev, src_data, 3 * srcWidth * srcHeight * sizeof(uchar), cudaMemcpyHostToDevice));

	dim3 grid(dst_s.height, dst_s.width);

	FourierTrans << < grid, 1 >> > (src_dev, dst_dev, src_step, dst_step, srcHeight, srcWidth, dst_s.height, dst_s.width);

	checkCudaErrors(cudaMemcpy(dst.data, dst_dev, 3 * dst_s.width * dst_s.height * sizeof(uchar), cudaMemcpyDeviceToHost));

	cudaFree(src_dev);
	cudaFree(dst_dev);

}

#endif // ! __RESIZE_KERNEL_CU_