
// ExperimentImgDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ImageProcess.h"
#include <opencv2\opencv.hpp>


#define MAX_THREAD 8
#define MAX_SPAN 15
struct DrawPara
{
	CImage* pImgSrc;
	CDC* pDC;
	int oriX;
	int oriY;
	int width;
	int height;
};

// CExperimentImgDlg 对话框
class CExperimentImgDlg : public CDialogEx
{
// 构造
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage* getImage() { return m_pImgSrc; }
	void MedianFilter();
	void AddNoise();
	void AddNoise_WIN();
	void AddNoise_OMP();
	void ThreadDraw(DrawPara *p);
	static UINT Update(void* p);
	void ImageCopy(CImage* pImgSrc, CImage* pImgDrt);
	void MatToCImage(cv::Mat & mat, CImage & cimage);
	void CImageToMat(CImage & cimage, cv::Mat & mat);
	void MedianFilter_WIN();
	void MedianFilter_OMP();
	void Resize();
	void Resize_WIN(float f);
	void Resize_OMP(float f);
	void Resize_CUDA(float f);
	void Rotate();
	void Rotate_WIN(double ang);
	void Rotate_OMP(double ang);
	void Rotate_CUDA(double ang);
	void FourierTrans();
	void FourierTrans_WIN();
	void FourierTrans_OMP();
	void FourierTrans_CUDA();
	void GaussianNoise();
	void GaussianNoise_WIN(double m, double s);
	void GaussianNoise_OMP(double m, double s);
	void GaussianNoise_CUDA(double m, double s);
	void SmoothFilter();
	void SmoothFilter_WIN(int k);
	void SmoothFilter_OMP(int k);
	void SmoothFilter_CUDA(int k);
	void GaussianFilter();
	void GaussianFilter_WIN(double s, int k);
	void GaussianFilter_OMP(double s, int k);
	void GaussianFilter_CUDA(double s, int k);
	void WienerFilter();
	void WienerFilter_WIN(int k);
	void WienerFilter_OMP(int k);
	void WienerFilter_CUDA(int k);
	void BilateralFilter();
	void BilateralFilter_WIN(double s, int k);
	void BilateralFilter_OMP(double s, int k);
	void BilateralFilter_CUDA(double s, int k);
	afx_msg LRESULT OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnResizeThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFourierTransThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGaussianNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSmoothFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGaussianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWienerFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBilateralFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);


// 实现
protected:
	HICON m_hIcon;
	CImage * m_pImgSrc;
	CImage * m_pImgCpy;

	cv::Mat ImgSrc_cv;
	cv::Mat ImgDst_cv;

	double* gm;
	double* cm;

	int m_nThreadNum;
	ThreadParam* m_pThreadParam;
	clock_t startTime;
//	ThreadParam * m_pThreadParam;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	CEdit mEditInfo;
	CStatic mPictureControl;
	CStatic mPictureControl2;
	afx_msg void OnCbnSelchangeComboFunction();
	afx_msg void OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonProcess();
	CButton m_CheckCirculation;

	CEdit mParamInfo;
	CEdit mScale;
	CEdit mRotation;
	CEdit mMu;
	CEdit mSigma;
	CEdit mKernelSize;
};
