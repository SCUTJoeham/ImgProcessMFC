
// stdafx.cpp : 只包括标准包含文件的源文件
// ExperimentImg.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"


// 自定义的线程间通信消息
#ifndef WM_MEDIAN_FILTER
#define WM_MEDIAN_FILTER WM_USER+1
#endif

#ifndef WM_NOISE
#define WM_NOISE WM_USER+2
#endif

#ifndef WM_RESIZE
#define WM_RESIZE WM_USER+3
#endif

#ifndef WM_ROTATE
#define WM_ROTATE WM_USER+4
#endif

#ifndef WM_FOURIER_TRANS
#define WM_FOURIER_TRANS WM_USER+5
#endif

#ifndef WM_GAUSSIAN_NOISE
#define WM_GAUSSIAN_NOISE WM_USER+6
#endif

#ifndef WM_SMOOTH_FILTER
#define WM_SMOOTH_FILTER WM_USER+7
#endif

#ifndef WM_GAUSSIAN_FILTER
#define WM_GAUSSIAN_FILTER WM_USER+8
#endif

#ifndef WM_WIENER_FILTER
#define WM_WIENER_FILTER WM_USER+9
#endif

#ifndef WM_BILATERAL_FILTER
#define WM_BILATERAL_FILTER WM_USER+10
#endif