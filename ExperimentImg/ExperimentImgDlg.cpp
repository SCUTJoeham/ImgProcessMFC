
// ExperimentImgDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ExperimentImg.h"
#include "ExperimentImgDlg.h"
#include "afxdialogex.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//extern "C" void add_host(int *host_a, int *host_b, int *host_c); //interface for kernel function
extern "C" void MedianFilter_host(int *pixel, int Width, int Height);
extern "C" void Resize_host(const cv::Mat &src, cv::Mat &dst, const cv::Size &s);
extern "C" void Rotate_host(const cv::Mat &src, cv::Mat &dst, const cv::Size &dst_s, double r_angel);
extern "C" void FourierTrans_host(const cv::Mat &src, cv::Mat &dst, const cv::Size &s);

static int flag1 = 0, flag2 = 0;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CExperimentImgDlg 对话框



CExperimentImgDlg::CExperimentImgDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EXPERIMENTIMG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//加载对话框的时候初始化
	m_pImgSrc = NULL;
//	m_pImgCpy = NULL;
	m_nThreadNum = 1;
	m_pThreadParam = new ThreadParam[MAX_THREAD];
	srand(time(0));
}

void CExperimentImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_EDIT_INFO, mEditInfo);
	DDX_Control(pDX, IDC_PICTURE, mPictureControl);
	DDX_Control(pDX, IDC_PICTURE2, mPictureControl2);
	DDX_Control(pDX, IDC_CHECK_100, m_CheckCirculation);
	DDX_Control(pDX, IDC_EDIT1, mParamInfo);
	DDX_Control(pDX, IDC_EDIT2, mScale);
	DDX_Control(pDX, IDC_EDIT3, mRotation);
	DDX_Control(pDX, IDC_EDIT4, mMu);
	DDX_Control(pDX, IDC_EDIT5, mSigma);
	DDX_Control(pDX, IDC_EDIT6, mKernelSize);
}

BEGIN_MESSAGE_MAP(CExperimentImgDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CExperimentImgDlg::OnBnClickedButtonOpen)
//	ON_EN_CHANGE(IDC_EDIT1, &CExperimentImgDlg::OnEnChangeEdit1)
//	ON_EN_CHANGE(IDC_EDIT_INFO, &CExperimentImgDlg::OnEnChangeEditInfo)
ON_CBN_SELCHANGE(IDC_COMBO_FUNCTION, &CExperimentImgDlg::OnCbnSelchangeComboFunction)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_THREADNUM, &CExperimentImgDlg::OnNMCustomdrawSliderThreadnum)
ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CExperimentImgDlg::OnBnClickedButtonProcess)
ON_MESSAGE(WM_NOISE, &CExperimentImgDlg::OnNoiseThreadMsgReceived)
ON_MESSAGE(WM_MEDIAN_FILTER, &CExperimentImgDlg::OnMedianFilterThreadMsgReceived)
ON_MESSAGE(WM_RESIZE, &CExperimentImgDlg::OnResizeThreadMsgReceived)
ON_MESSAGE(WM_ROTATE, &CExperimentImgDlg::OnRotateThreadMsgReceived)
ON_MESSAGE(WM_FOURIER_TRANS, &CExperimentImgDlg::OnFourierTransThreadMsgReceived)
ON_MESSAGE(WM_GAUSSIAN_NOISE, &CExperimentImgDlg::OnGaussianNoiseThreadMsgReceived)
ON_MESSAGE(WM_SMOOTH_FILTER, &CExperimentImgDlg::OnSmoothFilterThreadMsgReceived)
ON_MESSAGE(WM_GAUSSIAN_FILTER, &CExperimentImgDlg::OnGaussianFilterThreadMsgReceived)
ON_MESSAGE(WM_WIENER_FILTER, &CExperimentImgDlg::OnWienerFilterThreadMsgReceived)
ON_MESSAGE(WM_BILATERAL_FILTER, &CExperimentImgDlg::OnBilateralFilterThreadMsgReceived)
END_MESSAGE_MAP()


// CExperimentImgDlg 消息处理程序

BOOL CExperimentImgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
//	mEditInfo.SetWindowTextW(CString("File Path"));
	CComboBox * cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	cmb_function->AddString(_T("椒盐噪声"));
	cmb_function->AddString(_T("中值滤波"));
	cmb_function->AddString(_T("图像缩放"));
	cmb_function->AddString(_T("图像旋转"));
	cmb_function->AddString(_T("傅里叶变换"));
	cmb_function->AddString(_T("高斯噪声"));
	cmb_function->AddString(_T("平滑线性滤波"));
	cmb_function->AddString(_T("高斯滤波"));
	cmb_function->AddString(_T("维纳滤波"));
	cmb_function->AddString(_T("双边滤波"));
	cmb_function->SetCurSel(0);

	CComboBox * cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	cmb_thread->InsertString(0, _T("WIN多线程"));
	cmb_thread->InsertString(1, _T("OpenMP"));
	cmb_thread->InsertString(2, _T("CUDA"));
	cmb_thread->SetCurSel(0);

	CSliderCtrl * slider = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM));
	slider->SetRange(1, MAX_THREAD, TRUE);
	slider->SetPos(MAX_THREAD);

	AfxBeginThread((AFX_THREADPROC)&CExperimentImgDlg::Update, this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CExperimentImgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CExperimentImgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		if (m_pImgSrc != NULL)
		{
			int height;
			int width;
			CRect rect;
			CRect rect1;
			height = m_pImgSrc->GetHeight();
			width = m_pImgSrc->GetWidth();

			mPictureControl.GetClientRect(&rect);
			CDC *pDC = mPictureControl.GetDC();
			SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

			if (width <= rect.Width() && height <= rect.Width())
			{
				int tx = (int)(rect.Width() - width) / 2;
				int ty = (int)(rect.Height() - height) / 2;
				rect1 = CRect(tx, ty, tx + width, ty + height);
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			else
			{
				float xScale = (float)rect.Width() / (float)width;
				float yScale = (float)rect.Height() / (float)height;
				float ScaleIndex = (xScale <= yScale ? xScale : yScale);
				int tx = (int)(rect.Width() - (int)width * ScaleIndex) / 2;
				int ty = (int)(rect.Height() - (int)height * ScaleIndex) / 2;
				rect1 = CRect(tx, ty, tx + (int)width*ScaleIndex, ty + (int)height*ScaleIndex);
				m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
			}
			ReleaseDC(pDC);
		}
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CExperimentImgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT CExperimentImgDlg::Update(void* p)
{
	while (1)
	{
		if (flag2)
		{
			CExperimentImgDlg* dlg = (CExperimentImgDlg*)p;
			if (dlg->m_pImgSrc != NULL)
			{
				int height;
				int width;
				CRect rect;
				CRect rect1;
				height = dlg->m_pImgSrc->GetHeight();
				width = dlg->m_pImgSrc->GetWidth();

				dlg->mPictureControl2.GetClientRect(&rect);
				CDC *pDC = dlg->mPictureControl2.GetDC();
				SetStretchBltMode(pDC->m_hDC, STRETCH_HALFTONE);

				if (width <= rect.Width() && height <= rect.Width())
				{
					int tx = (int)(rect.Width() - width) / 2;
					int ty = (int)(rect.Height() - height) / 2;
					rect1 = CRect(tx, ty, tx + width, ty + height);
					CRect rect2(rect.left + 1, rect.top + 1, rect.Width() - 1, rect.Height() - 1);
					pDC->FillSolidRect(&rect2, RGB(240, 240, 240));
					flag2 = 0;
					dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
				}
				else
				{
					float xScale = (float)rect.Width() / (float)width;
					float yScale = (float)rect.Height() / (float)height;
					float ScaleIndex = (xScale <= yScale ? xScale : yScale);
					int tx = (int)(rect.Width() - (int)width * ScaleIndex) / 2;
					int ty = (int)(rect.Height() - (int)height * ScaleIndex) / 2;
					rect1 = CRect(tx, ty, tx + (int)width*ScaleIndex, ty + (int)height*ScaleIndex);
					CRect rect2(rect.left + 1, rect.top + 1, rect.Width() - 1, rect.Height() - 1);
					pDC->FillSolidRect(&rect2, RGB(240, 240, 240));
					flag2 = 0;
					dlg->m_pImgSrc->StretchBlt(pDC->m_hDC, rect1, SRCCOPY);
				}
				dlg->ReleaseDC(pDC);
			}
			flag2 = 0;
		}
	}
	return 0;
}

void CExperimentImgDlg::ThreadDraw(DrawPara *p)
{
	CRect rect;
	GetClientRect(&rect);
	CDC     memDC;             // 用于缓冲绘图的内存画笔  
	CBitmap memBitmap;         // 用于缓冲绘图的内存画布
	memDC.CreateCompatibleDC(p->pDC);  // 创建与原画笔兼容的画笔
	memBitmap.CreateCompatibleBitmap(p->pDC, p->width, p->height);  // 创建与原位图兼容的内存画布
	memDC.SelectObject(&memBitmap);      // 创建画笔与画布的关联
	memDC.FillSolidRect(rect, p->pDC->GetBkColor());
	p->pDC->SetStretchBltMode(HALFTONE);
	// 将pImgSrc的内容缩放画到内存画布中
	p->pImgSrc->StretchBlt(memDC.m_hDC, 0, 0, p->width, p->height);

	// 将已画好的画布复制到真正的缓冲区中
	p->pDC->BitBlt(p->oriX, p->oriY, p->width, p->height, &memDC, 0, 0, SRCCOPY);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}

void CExperimentImgDlg::ImageCopy(CImage* pImgSrc, CImage* pImgDrt)
{
	int MaxColors = pImgSrc->GetMaxColorTableEntries();
	RGBQUAD* ColorTab;
	ColorTab = new RGBQUAD[MaxColors];

	CDC *pDCsrc, *pDCdrc;
	if (!pImgDrt->IsNull())
	{
		pImgDrt->Destroy();
	}
	pImgDrt->Create(pImgSrc->GetWidth(), pImgSrc->GetHeight(), pImgSrc->GetBPP(), 0);

	if (pImgSrc->IsIndexed())
	{
		pImgSrc->GetColorTable(0, MaxColors, ColorTab);
		pImgDrt->SetColorTable(0, MaxColors, ColorTab);
	}

	pDCsrc = CDC::FromHandle(pImgSrc->GetDC());
	pDCdrc = CDC::FromHandle(pImgDrt->GetDC());
	pDCdrc->BitBlt(0, 0, pImgSrc->GetWidth(), pImgSrc->GetHeight(), pDCsrc, 0, 0, SRCCOPY);
	pImgSrc->ReleaseDC();
	pImgDrt->ReleaseDC();
	delete ColorTab;

}

void CExperimentImgDlg::MatToCImage(cv::Mat& mat, CImage& cimage)
{
	if (0 == mat.total())
	{
		return;
	}


	int nChannels = mat.channels();
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = mat.cols;
	int nHeight = mat.rows;


	//重建cimage
	cimage.Destroy();
	cimage.Create(nWidth, nHeight, 8 * nChannels);


	//拷贝数据


	uchar* pucRow;                                    //指向数据区的行指针
	uchar* pucImage = (uchar*)cimage.GetBits();        //指向数据区的指针
	int nStep = cimage.GetPitch();                    //每行的字节数,注意这个返回值有正有负


	if (1 == nChannels)                                //对于单通道的图像需要初始化调色板
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		cimage.GetColorTable(0, nMaxColors, rgbquadColorTable);
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;
			rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
			rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
		}
		cimage.SetColorTable(0, nMaxColors, rgbquadColorTable);
		delete[]rgbquadColorTable;
	}


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				*(pucImage + nRow * nStep + nCol) = pucRow[nCol];
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					*(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
				}
			}
		}
	}
}

void CExperimentImgDlg::CImageToMat(CImage& cimage, cv::Mat& mat)
{
	if (true == cimage.IsNull())
	{
		return;
	}


	int nChannels = cimage.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cimage.GetWidth();
	int nHeight = cimage.GetHeight();


	//重建mat
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}


	//拷贝数据


	uchar* pucRow;                                    //指向数据区的行指针
	uchar* pucImage = (uchar*)cimage.GetBits();        //指向数据区的指针
	int nStep = cimage.GetPitch();                    //每行的字节数,注意这个返回值有正有负


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
}

void CExperimentImgDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR szFilter[] = _T("JPEG(*jpg)|*.jpg|*.bmp|*.png|TIFF(*.tif)|*.tif|All Files （*.*）|*.*||");
	CString filePath("");
	
	CFileDialog fileOpenDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);
	if (fileOpenDialog.DoModal() == IDOK)
	{
		VERIFY(filePath = fileOpenDialog.GetPathName());
		CString strFilePath(filePath);
	//	mParamInfo.SetWindowTextW(strFilePath);	//在文本框中显示图像路径
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(strFilePath+"\r\n");

		if (m_pImgSrc != NULL)
		{
			m_pImgSrc->Destroy();
			delete m_pImgSrc;
		}
		m_pImgSrc = new CImage();
		m_pImgSrc->Load(strFilePath);

		std::string filepath;
		filepath = CT2A(strFilePath.GetString());
		if (!ImgSrc_cv.empty())
			ImgSrc_cv.release();
		ImgSrc_cv = cv::imread(filepath);

		this->Invalidate();

	}
}

void CExperimentImgDlg::OnCbnSelchangeComboFunction()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CExperimentImgDlg::OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CSliderCtrl *slider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_THREADNUM);
	CString text("");
	m_nThreadNum = slider->GetPos();
	text.Format(_T("%d"), m_nThreadNum);
	GetDlgItem(IDC_STATIC_THREADNUM)->SetWindowText(text);
	*pResult = 0;
}

void CExperimentImgDlg::OnBnClickedButtonProcess()
{
	// TODO: 在此添加控件通知处理程序代码
	CComboBox* cmb_function = ((CComboBox*)GetDlgItem(IDC_COMBO_FUNCTION));
	int func = cmb_function->GetCurSel();
	switch (func)
	{
	case 0:  //椒盐噪声
		AddNoise();
		break;
	case 1: //自适应中值滤波
		MedianFilter();
		break;
	case 2: //图像缩放
		Resize();
		break;
	case 3: //图像旋转
		Rotate();
		break;
	case 4: //傅里叶变换
		FourierTrans();
		break;
	case 5: //高斯噪声
		GaussianNoise();
		break;
	case 6: //平滑滤波
		SmoothFilter();
		break;
	case 7: //高斯滤波
		GaussianFilter();
		break;
	case 8: //维纳滤波
		WienerFilter();
		break;
	case 9: //双边滤波
		BilateralFilter();
		break;
	default:
		break;
	}
}

void CExperimentImgDlg::AddNoise()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1:100;
	startTime = clock();

	switch (thread)
	{
	case 0://win多线程
	{
		AddNoise_WIN();
	}
	break;

	case 1://openmp
	{
		AddNoise_OMP();
	}
	break;

	case 2://cuda
	{
		CString info(_T("此功能未实现CUDA方法！"));
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(info + "\r\n");
	}
		break;
	}
}

void CExperimentImgDlg::AddNoise_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::addNoise, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::AddNoise_OMP()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

	#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].src = m_pImgSrc;
		ImageProcess::addNoise(&m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::MedianFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
	{
		MedianFilter_WIN();
	}

	break;

	case 1://openmp
	{
		MedianFilter_OMP();
	}

	break;

	case 2://cuda
	{
		byte* pRealData = (byte*)m_pImgSrc->GetBits();
		int pit = m_pImgSrc->GetPitch();	//line offset 
		int bitCount = m_pImgSrc->GetBPP() / 8;
		int height = m_pImgSrc->GetHeight();
		int width = m_pImgSrc->GetWidth();
		int length = height * width;
		int *pixel = (int*)malloc(length * sizeof(int));
		int *pixelR = (int*)malloc(length * sizeof(int));
		int *pixelG = (int*)malloc(length * sizeof(int));
		int *pixelB = (int*)malloc(length * sizeof(int));
		int *pixelIndex = (int*)malloc(length * sizeof(int));
		int index = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (bitCount == 1)
				{
					pixel[index] = *(pRealData + pit * y + x * bitCount);
					index++;
				}
				else
				{
					pixelR[index] = *(pRealData + pit * y + x * bitCount + 2);
					pixelG[index] = *(pRealData + pit * y + x * bitCount + 1);
					pixelB[index] = *(pRealData + pit * y + x * bitCount);
					pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);
				}
			}
		}
		if (bitCount == 1)
		{
			MedianFilter_host(pixel, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit * y + x * bitCount) = pixel[index];
					index++;
				}
			}
		}
		else
		{
			MedianFilter_host(pixelR, width, height);
			MedianFilter_host(pixelG, width, height);
			MedianFilter_host(pixelB, width, height);
			index = 0;
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					*(pRealData + pit * y + x * bitCount + 2) = pixelR[index];
					*(pRealData + pit * y + x * bitCount + 1) = pixelG[index];
					*(pRealData + pit * y + x * bitCount) = pixelB[index];
					index++;
				}
			}
		}

		clock_t endTime = clock();
		flag2 = 1;
		CString timeStr;
		timeStr.Format(_T("中值滤波(CUDA): 处理%d次,耗时: %dms"), circulation, endTime - startTime);
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(timeStr + "\r\n");
		//AfxMessageBox(_T("finish!"));
	}
		break;
	}
}

void CExperimentImgDlg::MedianFilter_WIN()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;
	int h = m_pImgSrc->GetHeight() / m_nThreadNum;
	int w = m_pImgSrc->GetWidth() / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::medianFilter, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::MedianFilter_OMP()
{
	int subLength = m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() / m_nThreadNum;

#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : m_pImgSrc->GetWidth() * m_pImgSrc->GetHeight() - 1;
		m_pThreadParam[i].maxSpan = MAX_SPAN;
		m_pThreadParam[i].src = m_pImgSrc;
		ImageProcess::medianFilter(&m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::Resize()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString f_s = NULL;
	mScale.GetWindowTextW(f_s);
	std::string fs = "";
	fs = CT2A(f_s.GetString());

	float fac;
	fs.empty() ? (fac = 1.0) : (fac = std::stof(fs));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
		Resize_WIN(fac);
	break;

	case 1://openmp
		Resize_OMP(fac);
	break;

	case 2://cuda
		Resize_CUDA(fac);
	break;
	}

}

void CExperimentImgDlg::Resize_WIN(float f)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv.create(int(ImgSrc_cv.rows * f), int(ImgSrc_cv.cols * f), ImgSrc_cv.type());

	int subLength = ImgDst_cv.cols * ImgDst_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgDst_cv.cols * ImgDst_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::resize, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::Resize_OMP(float f)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv.create(int(ImgSrc_cv.rows * f), int(ImgSrc_cv.cols * f), ImgSrc_cv.type());

	int subLength = ImgDst_cv.cols * ImgDst_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgDst_cv.cols * ImgDst_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		ImageProcess::resize(&m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::Resize_CUDA(float f)
{
	
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv.create(int(ImgSrc_cv.rows * f), int(ImgSrc_cv.cols * f), ImgSrc_cv.type());

	if(ImgSrc_cv.channels()==3)
		Resize_host(ImgSrc_cv, ImgDst_cv, cv::Size(ImgDst_cv.cols, ImgDst_cv.rows));

	m_pImgCpy = new CImage();
	MatToCImage(ImgDst_cv, *m_pImgCpy);
	if (m_pImgSrc != NULL)
		m_pImgSrc->Destroy();
	m_pImgSrc = m_pImgCpy;
	m_pImgCpy = NULL;

	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	tempProcessCount++;
	if (tempProcessCount < circulation)
	{
		switch (thread)
		{
		case 0://win多线程
			Resize_WIN(f);
			break;

		case 1://openmp
			Resize_OMP(f);
			break;

		case 2://cuda
			Resize_CUDA(f);
			break;
		}
	}
	else
	{
		tempProcessCount = 0;
		//CTime endTime = CTime::GetTickCount();
		clock_t endTime = clock();

		ImgSrc_cv.release();
		ImgSrc_cv = ImgDst_cv.clone();
		flag2 = 1; //图像发生变化

		CString timeStr;
		timeStr.Format(_T("图像缩放(CUDA): 处理%d次,耗时: %dms"), circulation, endTime - startTime);
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(timeStr + "\r\n");
		//AfxMessageBox(timeStr);
	}
}

void CExperimentImgDlg::Rotate()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString angel_s = NULL;
	mRotation.GetWindowTextW(angel_s);
	std::string angels = "";
	angels = CT2A(angel_s.GetString());

	double angel;
	angels.empty() ? (angel = 0) : (angel = std::stod(angels));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
		Rotate_WIN(angel);
		break;

	case 1://openmp
		Rotate_OMP(angel);
		break;

	case 2://cuda
		Rotate_CUDA(angel);
		break;
	}
}

void CExperimentImgDlg::Rotate_WIN(double ang)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	int dh = sqrt(ImgSrc_cv.rows*ImgSrc_cv.rows + ImgSrc_cv.cols*ImgSrc_cv.cols);
	ImgDst_cv.create(dh, dh, ImgSrc_cv.type());

	int subLength = ImgDst_cv.cols * ImgDst_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgDst_cv.cols * ImgDst_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].r_angel = ang;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::rotate, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::Rotate_OMP(double ang)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	int dh = sqrt(ImgSrc_cv.rows*ImgSrc_cv.rows + ImgSrc_cv.cols*ImgSrc_cv.cols);
	ImgDst_cv.create(dh, dh, ImgSrc_cv.type());

	int subLength = ImgDst_cv.cols * ImgDst_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgDst_cv.cols * ImgDst_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].r_angel = ang;
		ImageProcess::rotate(&m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::Rotate_CUDA(double ang)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	int dh = sqrt(ImgSrc_cv.rows*ImgSrc_cv.rows + ImgSrc_cv.cols*ImgSrc_cv.cols);
	ImgDst_cv.create(dh, dh, ImgSrc_cv.type());

	if (ImgSrc_cv.channels() == 3)
		Rotate_host(ImgSrc_cv, ImgDst_cv, cv::Size(ImgDst_cv.cols, ImgDst_cv.rows), ang);

	m_pImgCpy = new CImage();
	MatToCImage(ImgDst_cv, *m_pImgCpy);
	if (m_pImgSrc != NULL)
		m_pImgSrc->Destroy();
	m_pImgSrc = m_pImgCpy;
	m_pImgCpy = NULL;

	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	tempProcessCount++;
	if (tempProcessCount < circulation)
	{
		switch (thread)
		{
		case 0://win多线程
			Rotate_WIN(ang);
			break;

		case 1://openmp
			Rotate_OMP(ang);
			break;

		case 2://cuda
			Rotate_CUDA(ang);
			break;
		}
	}
	else
	{
		tempProcessCount = 0;
		//CTime endTime = CTime::GetTickCount();
		clock_t endTime = clock();

		ImgSrc_cv.release();
		ImgSrc_cv = ImgDst_cv.clone();
		flag2 = 1; //图像发生变化

		CString timeStr;
		timeStr.Format(_T("图像旋转(CUDA): 处理%d次,耗时: %dms"), circulation, endTime - startTime);
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(timeStr + "\r\n");
		//AfxMessageBox(timeStr);
	}
}

void CExperimentImgDlg::FourierTrans()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
		FourierTrans_WIN();
		break;

	case 1://openmp
		FourierTrans_OMP();
		break;

	case 2://cuda
		FourierTrans_CUDA();
		break;
	}
}

void CExperimentImgDlg::FourierTrans_WIN()
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::fourierTrans, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::FourierTrans_OMP()
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		ImageProcess::fourierTrans(&m_pThreadParam[i]);
	}

}

void CExperimentImgDlg::FourierTrans_CUDA()
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	if (ImgSrc_cv.channels() == 3)
		FourierTrans_host(ImgSrc_cv, ImgDst_cv, cv::Size(ImgDst_cv.cols, ImgDst_cv.rows));

	m_pImgCpy = new CImage();
	MatToCImage(ImgDst_cv, *m_pImgCpy);
	if (m_pImgSrc != NULL)
		m_pImgSrc->Destroy();
	m_pImgSrc = m_pImgCpy;
	m_pImgCpy = NULL;

	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 1;

	tempProcessCount++;
	if (tempProcessCount < circulation)
	{
		switch (thread)
		{
		case 0://win多线程
			FourierTrans_WIN();
			break;

		case 1://openmp
			FourierTrans_OMP();
			break;

		case 2://cuda
			FourierTrans_CUDA();
			break;
		}
	}
	else
	{
		tempProcessCount = 0;
		//CTime endTime = CTime::GetTickCount();
		clock_t endTime = clock();

		flag2 = 1; //图像发生变化

		CString timeStr;
		timeStr.Format(_T("傅里叶变换(CUDA): 处理%d次,耗时: %dms"), circulation, endTime - startTime);
		mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
		mParamInfo.ReplaceSel(timeStr + "\r\n");
		//AfxMessageBox(timeStr);
	}
}

void CExperimentImgDlg::GaussianNoise()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString mu_s = NULL , sigma_s = NULL;
	mMu.GetWindowTextW(mu_s);
	mSigma.GetWindowTextW(sigma_s);
	std::string mus = "", sigmas = "";
	mus = CT2A(mu_s.GetString());
	sigmas = CT2A(sigma_s.GetString());

	double mu, sigma;
	mus.empty() ? (mu = 0) : (mu = std::stod(mus));
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));
	
	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
		GaussianNoise_WIN(mu, sigma);
		break;

	case 1://openmp
		GaussianNoise_OMP(mu, sigma);
		break;

	case 2://cuda
		GaussianNoise_CUDA(mu, sigma);
		break;
	}
	
}

void CExperimentImgDlg::GaussianNoise_WIN(double m, double s)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].MU = m;
		m_pThreadParam[i].SIGMA = s;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::gaussianNoise, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::GaussianNoise_OMP(double m, double s)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].MU = m;
		m_pThreadParam[i].SIGMA = s;
		ImageProcess::gaussianNoise(&m_pThreadParam[i]);
	}

}

void CExperimentImgDlg::GaussianNoise_CUDA(double m, double s)
{
	CString info(_T("此功能未实现CUDA方法！"));
	mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
	mParamInfo.ReplaceSel(info + "\r\n");
}

void CExperimentImgDlg::SmoothFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString k_s = NULL;
	mKernelSize.GetWindowTextW(k_s);
	std::string ks = "";
	ks = CT2A(k_s.GetString());
	
	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
	{
		SmoothFilter_WIN(ker);
	}

	break;

	case 1://openmp
	{
		SmoothFilter_OMP(ker);
	}

	break;

	case 2://cuda
		SmoothFilter_CUDA(ker);
		break;
	}
}

void CExperimentImgDlg::SmoothFilter_WIN(int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;  
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::smoothFilter, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::SmoothFilter_OMP(int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();

	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k; 
		ImageProcess::smoothFilter(&m_pThreadParam[i]);
	}

}

void CExperimentImgDlg::SmoothFilter_CUDA(int k)
{
	CString info(_T("此功能未实现CUDA方法！"));
	mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
	mParamInfo.ReplaceSel(info + "\r\n");
}

void CExperimentImgDlg::GaussianFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString sigma_s = NULL, k_s = NULL;
	mSigma.GetWindowTextW(sigma_s);
	mKernelSize.GetWindowTextW(k_s);
	std::string sigmas = "", ks = "";
	sigmas = CT2A(sigma_s.GetString());
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));
	double sigma;
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
	{
		GaussianFilter_WIN(sigma, ker);
	}

	break;

	case 1://openmp
	{
		GaussianFilter_OMP(sigma, ker);
	}

	break;

	case 2://cuda
		GaussianFilter_CUDA(sigma, ker);
		break;
	}
}

void CExperimentImgDlg::GaussianFilter_WIN(double s, int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	if (gm != NULL)
	{
		delete gm; gm = NULL;
	}
	gm = new double[k * k];
	ImageProcess::generate_gauss_mask(k, s, gm);

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k; 
		m_pThreadParam[i].gauss_mask = gm;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::gaussianFilter, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::GaussianFilter_OMP(double s, int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	if (gm != NULL)
	{
		delete gm; gm = NULL;
	}
	gm = new double[k * k];
	ImageProcess::generate_gauss_mask(k, s, gm);

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;
		m_pThreadParam[i].gauss_mask = gm;
		ImageProcess::gaussianFilter(&m_pThreadParam[i]);
	}

}

void CExperimentImgDlg::GaussianFilter_CUDA(double s, int k)
{
	CString info(_T("此功能未实现CUDA方法！"));
	mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
	mParamInfo.ReplaceSel(info + "\r\n");
}

void CExperimentImgDlg::WienerFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString k_s = NULL;
	mKernelSize.GetWindowTextW(k_s);
	std::string ks = "";
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
	{
		WienerFilter_WIN(ker);
	}

	break;

	case 1://openmp
	{
		WienerFilter_OMP(ker);
	}

	break;

	case 2://cuda
		WienerFilter_CUDA(ker);
		break;
	}
}

void CExperimentImgDlg::WienerFilter_WIN(int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::wienerFilter, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::WienerFilter_OMP(int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;
		ImageProcess::wienerFilter(&m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::WienerFilter_CUDA(int k)
{
	CString info(_T("此功能未实现CUDA方法！"));
	mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
	mParamInfo.ReplaceSel(info + "\r\n");
}

void CExperimentImgDlg::BilateralFilter()
{
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString sigma_s = NULL, k_s = NULL;
	mSigma.GetWindowTextW(sigma_s);
	mKernelSize.GetWindowTextW(k_s);
	std::string sigmas = "", ks = "";
	sigmas = CT2A(sigma_s.GetString());
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));
	double sigma;
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));

	startTime = clock();
	switch (thread)
	{
	case 0://win多线程
	{
		BilateralFilter_WIN(sigma, ker);
	}

	break;

	case 1://openmp
	{
		BilateralFilter_OMP(sigma, ker);
	}

	break;

	case 2://cuda
		BilateralFilter_CUDA(sigma, ker);
		break;
	}
}

void CExperimentImgDlg::BilateralFilter_WIN(double s, int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	if (gm != NULL){ delete gm; gm = NULL; }
	gm = new double[k * k];
	if (cm != NULL) { delete cm; cm = NULL; }
	cm = new double[256];

	ImageProcess::generate_gauss_mask(k, s, gm);
	ImageProcess::generate_color_mask(s, cm);

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;
		m_pThreadParam[i].gauss_mask = gm;
		m_pThreadParam[i].color_mask = cm;
		AfxBeginThread((AFX_THREADPROC)&ImageProcess::bilateralFilter, &m_pThreadParam[i]);
	}
}

void CExperimentImgDlg::BilateralFilter_OMP(double s, int k)
{
	if (!ImgDst_cv.empty())
		ImgDst_cv.release();
	ImgDst_cv = ImgSrc_cv.clone();

	if (gm != NULL) { delete gm; gm = NULL; }
	gm = new double[k * k];
	if (cm != NULL) { delete cm; cm = NULL; }
	cm = new double[256];

	ImageProcess::generate_gauss_mask(k, s, gm);
	ImageProcess::generate_color_mask(s, cm);

	int subLength = ImgSrc_cv.cols * ImgSrc_cv.rows / m_nThreadNum;
#pragma omp parallel for num_threads(m_nThreadNum)
	for (int i = 0; i < m_nThreadNum; ++i)
	{
		m_pThreadParam[i].startIndex = i * subLength;
		m_pThreadParam[i].endIndex = i != m_nThreadNum - 1 ?
			(i + 1) * subLength - 1 : ImgSrc_cv.cols * ImgSrc_cv.rows - 1;
		m_pThreadParam[i].src_cv = ImgSrc_cv;
		m_pThreadParam[i].dst_cv = ImgDst_cv;
		m_pThreadParam[i].kernel_size = k;
		m_pThreadParam[i].gauss_mask = gm;
		m_pThreadParam[i].color_mask = cm;
		ImageProcess::bilateralFilter(&m_pThreadParam[i]);
	}

}

void CExperimentImgDlg::BilateralFilter_CUDA(double s, int k)
{
	CString info(_T("此功能未实现CUDA方法！"));
	mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
	mParamInfo.ReplaceSel(info + "\r\n");
}


LRESULT CExperimentImgDlg::OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempThreadCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();

	if ((int)wParam == 1)
	{
		// 当所有线程都返回了值1代表全部结束~显示时间
		if (m_nThreadNum == ++tempThreadCount)
		{
			//CTime endTime = CTime::GetTickCount();
			//CString timeStr;
			//timeStr.Format(_T("耗时:%dms"), endTime - startTime);
			tempThreadCount = 0;
			tempProcessCount++;
			if (tempProcessCount < circulation)
			{
				switch (thread)
				{
				case 0://win多线程
				{
					MedianFilter_WIN();
				}

				break;

				case 1://openmp
				{
					MedianFilter_OMP();
				}

				break;

				case 2://cuda
					break;
				}
			}
			else
			{
				tempProcessCount = 0;
				clock_t endTime = clock();

				flag2 = 1;

				CString timeStr;
				timeStr.Format(_T("中值滤波: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
				mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
				mParamInfo.ReplaceSel(timeStr + "\r\n");
				//AfxMessageBox(timeStr);
			}
			// 显示消息窗口
//			AfxMessageBox(timeStr);
		}
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
			{
				AddNoise_WIN();
			}
			break;

			case 1://openmp
			{
				AddNoise_OMP();
			}
			break;

			case 2://cuda
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();
			flag2 = 1;
			CString timeStr;
			timeStr.Format(_T("椒盐噪声: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr+"\r\n");
			//AfxMessageBox(timeStr);
		}
	//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnResizeThreadMsgReceived(WPARAM wParam, LPARAM lParam) 
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString f_s = NULL;
	mScale.GetWindowTextW(f_s);
	std::string fs = "";
	fs = CT2A(f_s.GetString());
	float fac;
	fs.empty() ? (fac = 1.0) : (fac = std::stof(fs));

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
				Resize_WIN(fac);
				break;

			case 1://openmp
				Resize_OMP(fac);
				break;

			case 2://cuda
				Resize_CUDA(fac);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1; //图像发生变化

			CString timeStr;
			timeStr.Format(_T("图像缩放: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnRotateThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString angel_s = NULL;
	mRotation.GetWindowTextW(angel_s);
	std::string angels = "";
	angels = CT2A(angel_s.GetString());

	double angel;
	angels.empty() ? (angel = 0) : (angel = std::stod(angels));

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
				Rotate_WIN(angel);
				break;

			case 1://openmp
				Rotate_OMP(angel);
				break;

			case 2://cuda
				Rotate_CUDA(angel);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1; //图像发生变化

			CString timeStr;
			timeStr.Format(_T("图像旋转: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnFourierTransThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 1;

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
				FourierTrans_WIN();
				break;

			case 1://openmp
				FourierTrans_OMP();
				break;

			case 2://cuda
				FourierTrans_CUDA();
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();
			flag2 = 1; //图像发生变化

			CString timeStr;
			timeStr.Format(_T("傅里叶变换: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnGaussianNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();

	CString mu_s = NULL, sigma_s = NULL;
	mMu.GetWindowTextW(mu_s);
	mSigma.GetWindowTextW(sigma_s);
	std::string mus = "", sigmas = "";
	mus = CT2A(mu_s.GetString());
	sigmas = CT2A(sigma_s.GetString());

	double mu, sigma;
	mus.empty() ? (mu = 0) : (mu = std::stod(mus));
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));

	if ((int)wParam == 1)
		tempCount++;

	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;

		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation) {
			switch (thread)
			{
			case 0://win多线程
				GaussianNoise_WIN(mu, sigma);
				break;

			case 1://openmp
				GaussianNoise_OMP(mu, sigma);
				break;

			case 2://cuda
				GaussianNoise_CUDA(mu, sigma);
				break;
			}
		}
			
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1;

			CString timeStr;
			timeStr.Format(_T("高斯噪声: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnSmoothFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString k_s = NULL;
	mKernelSize.GetWindowTextW(k_s);
	std::string ks = "";
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
			{
				SmoothFilter_WIN(ker);
			}

			break;

			case 1://openmp
			{
				SmoothFilter_OMP(ker);
			}

			break;

			case 2://cuda
				SmoothFilter_CUDA(ker);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1;

			CString timeStr;
			timeStr.Format(_T("平滑线性滤波: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnGaussianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString sigma_s = NULL, k_s = NULL;
	mSigma.GetWindowTextW(sigma_s);
	mKernelSize.GetWindowTextW(k_s);
	std::string sigmas = "", ks = "";
	sigmas = CT2A(sigma_s.GetString());
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));
	double sigma;
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));


	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
			{
				GaussianFilter_WIN(sigma, ker);
			}

			break;

			case 1://openmp
			{
				GaussianFilter_OMP(sigma, ker);
			}

			break;

			case 2://cuda
				GaussianFilter_CUDA(sigma, ker);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			delete gm; gm = NULL;
			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1;

			CString timeStr;
			timeStr.Format(_T("高斯滤波: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnWienerFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0;
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;

	CString k_s = NULL;
	mKernelSize.GetWindowTextW(k_s);
	std::string ks = "";
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
			{
				WienerFilter_WIN(ker);
			}

			break;

			case 1://openmp
			{
				WienerFilter_OMP(ker);
			}

			break;

			case 2://cuda
				WienerFilter_CUDA(ker);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();
			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1;

			CString timeStr;
			timeStr.Format(_T("维纳滤波: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

LRESULT CExperimentImgDlg::OnBilateralFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam)
{
	static int tempCount = 0;
	static int tempProcessCount = 0; 
	CComboBox* cmb_thread = ((CComboBox*)GetDlgItem(IDC_COMBO_THREAD));
	int thread = cmb_thread->GetCurSel();
	CButton* clb_circulation = ((CButton*)GetDlgItem(IDC_CHECK_CIRCULATION));
	int circulation = clb_circulation->GetCheck() == 0 ? 1 : 100;


	CString sigma_s = NULL, k_s = NULL;
	mSigma.GetWindowTextW(sigma_s);
	mKernelSize.GetWindowTextW(k_s);
	std::string sigmas = "", ks = "";
	sigmas = CT2A(sigma_s.GetString());
	ks = CT2A(k_s.GetString());

	int ker;
	ks.empty() ? (ker = 3) : (ker = std::stoi(ks));
	double sigma;
	sigmas.empty() ? (sigma = 1) : (sigma = std::stod(sigmas));

	if ((int)wParam == 1)
		tempCount++;
	if (m_nThreadNum == tempCount)
	{
		m_pImgCpy = new CImage();
		MatToCImage(ImgDst_cv, *m_pImgCpy);
		if (m_pImgSrc != NULL)
			m_pImgSrc->Destroy();
		m_pImgSrc = m_pImgCpy;
		m_pImgCpy = NULL;
		//CTime endTime = CTime::GetTickCount();
		//CString timeStr;
		//timeStr.Format(_T("耗时:%dms", endTime - startTime));
		tempCount = 0;
		tempProcessCount++;
		if (tempProcessCount < circulation)
		{
			switch (thread)
			{
			case 0://win多线程
			{
				BilateralFilter_WIN(sigma, ker);
			}

			break;

			case 1://openmp
			{
				BilateralFilter_OMP(sigma, ker);
			}

			break;

			case 2://cuda
				BilateralFilter_CUDA(sigma, ker);
				break;
			}
		}
		else
		{
			tempProcessCount = 0;
			//CTime endTime = CTime::GetTickCount();
			clock_t endTime = clock();

			delete gm; gm = NULL;
			delete cm; cm = NULL;
			ImgSrc_cv.release();
			ImgSrc_cv = ImgDst_cv.clone();
			flag2 = 1;

			CString timeStr;
			timeStr.Format(_T("双边滤波: 处理%d次,耗时: %dms"), circulation, endTime - startTime);
			mParamInfo.SetSel(mParamInfo.GetWindowTextLength(), mParamInfo.GetWindowTextLength());
			mParamInfo.ReplaceSel(timeStr + "\r\n");
			//AfxMessageBox(timeStr);
		}
		//	AfxMessageBox(timeStr);
	}
	return 0;
}

