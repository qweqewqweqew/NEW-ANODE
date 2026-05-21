
// MfcTestLzdmAlgDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MfcTestQhxcAlg.h"
#include "MfcTestQhxcAlgDlg.h"
#include "afxdialogex.h"
#include "CUnloadPlateA.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CMfcTestLzdmAlgDlg 对话框



CMfcTestqhxcAlgDlg::CMfcTestqhxcAlgDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCTESTLZDMALG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMfcTestqhxcAlgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMfcTestqhxcAlgDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMfcTestqhxcAlgDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMfcTestqhxcAlgDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMfcTestqhxcAlgDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON5, &CMfcTestqhxcAlgDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CMfcTestqhxcAlgDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON4, &CMfcTestqhxcAlgDlg::OnBnClickedButton4)
	//ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON7, &CMfcTestqhxcAlgDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CMfcTestqhxcAlgDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CMfcTestqhxcAlgDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CMfcTestqhxcAlgDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CMfcTestqhxcAlgDlg::OnBnClickedButton11)
	ON_EN_CHANGE(IDC_EDIT2, &CMfcTestqhxcAlgDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON12, &CMfcTestqhxcAlgDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CMfcTestqhxcAlgDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CMfcTestqhxcAlgDlg::OnBnClickedButton14)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT5, &CMfcTestqhxcAlgDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_BUTTON15, &CMfcTestqhxcAlgDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &CMfcTestqhxcAlgDlg::OnBnClickedButton16)
	ON_BN_CLICKED(IDC_BUTTON17, &CMfcTestqhxcAlgDlg::OnBnClickedButton17)
END_MESSAGE_MAP()


// CMfcTestLzdmAlgDlg 消息处理程序

BOOL CMfcTestqhxcAlgDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//HTuple hv_Exception;
	//try
	//{
	//	HObject img;
	//	ReadImage(&img ,"");
	//}
	//catch (HException& ex)
	//{

	//	std::string errorCode = ex.ProcName();
	//	std::string errorMessage = ex.ErrorMessage();
	//	std::string ErrMsg= ex.ProcName();

	//

	//	double ddd = 0;
	//}


		//////////////////////////////////////////////////////////////////
	// 初始化算法环境 --只初始化一次
	SetHcppInterfaceStringEncodingIsUtf8(false);//halcon19以上 有中文路径的需要设置
	SetSystem("clip_region", "false");//设定算法环境--边界超出依然有效

	InitUI();

 	SetTimer(1, 100, FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMfcTestqhxcAlgDlg::OnPaint()
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
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMfcTestqhxcAlgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void  CMfcTestqhxcAlgDlg::DoEvents()
{
	MSG msg;
	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{

		if (!AfxGetThread()->PumpMessage())
			return;
	}
}

void CMfcTestqhxcAlgDlg::DelayT(DWORD time)
{
	if (time <= 0)
	{
		return;
	}

	DWORD t = ::GetTickCount();

	while (::GetTickCount() - t < time)
	{
		DoEvents();

	}

}






void CMfcTestqhxcAlgDlg::InitUI()
{

	InitView();

	//信息提示栏
	CRect rc;
	GetDlgItem(IDS_ROI_LIST)->GetWindowRect(&rc);
	ScreenToClient(rc);
	m_pDlgInfoList = NULL;
	m_pDlgInfoList = new CDlgInfoList();
	m_pDlgInfoList->Create(IDD_DLGINFOLIST, this);
	m_pDlgInfoList->ShowWindow(1);
	::SetWindowPos(m_pDlgInfoList->m_hWnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW/*SWP_NOSIZE*/);



	SetDlgItemText(IDC_EDIT1, "1201");
	SetDlgItemText(IDC_EDIT2, "350");
	SetDlgItemText(IDC_EDIT3, "400");


	return;
}


void CMfcTestqhxcAlgDlg::InitView()
{
	//图像编辑栏


	//视窗数量
	int iNumView = 3;


	//清除窗口
	for (int i = 0; i < m_pListDlgImageWindow.size(); i++)
	{
		m_pListDlgImageWindow[i]->DestroyWindow();
		delete m_pListDlgImageWindow[i];
		m_pListDlgImageWindow[i] = NULL;
	}
	m_pListDlgImageWindow.clear();


	//生成窗口

	for (int i = 0; i < iNumView; i++)
	{
		CDlgImageWindow* pAClass;
		pAClass = new CDlgImageWindow;
		pAClass->Create(IDD_DLGIMAGEWINDOW, this);

		CRect rsView;
		if (i==0)
		{
	
			GetDlgItem(IDS_ROI_IMG)->GetWindowRect(&rsView);
			ScreenToClient(rsView);
		}
		else if (i == 1)
		{

			GetDlgItem(IDS_ROI_IMG22)->GetWindowRect(&rsView);
			ScreenToClient(rsView);
		}

		else 
		{
			GetDlgItem(IDS_ROI_IMG33)->GetWindowRect(&rsView);
			ScreenToClient(rsView);
		}




		pAClass->MoveWindow(rsView);
		pAClass->ShowWindow(1);
		pAClass->InitHWindow();
		m_pListDlgImageWindow.push_back(pAClass);
	}



	return;
}


bool CMfcTestqhxcAlgDlg::RegionSegmentByMatrix(CRect Rect1, int nRow, int nCol, CRect* pRect2)
{
	//首先判断一些异常情况，长宽设置异常，矩阵设置异常
	int nWidth = Rect1.Width();
	int nHeight = Rect1.Height();

	if ((nWidth < 2) || (nHeight < 2))
	{
		return false;
	}
	if ((nRow < 1) || (nCol < 1))
	{
		return false;
	}

	int nSegWidth = nWidth / nCol;
	int nSegHeight = nHeight / nRow;

	CRect* rect = new CRect[nRow * nCol];
	for (int i = 0; i < nRow; i++)
	{
		for (int j = 0; j < nCol; j++)
		{
			rect[i * nCol + j].left = Rect1.left + j * nSegWidth;
			rect[i * nCol + j].right = Rect1.left + (j + 1) * nSegWidth;
			rect[i * nCol + j].top = Rect1.top + i * nSegHeight;
			rect[i * nCol + j].bottom = Rect1.top + (i + 1) * nSegHeight;
			pRect2[i * nCol + j] = rect[i * nCol + j];
		}
	}
	delete[] rect;
	return true;
}





void CMfcTestqhxcAlgDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	CFileDialog fileDialog(TRUE, _T("ply"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("PNG Files (*.ly)|*.ply||"));

	// 弹出文件对话框
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}



	CString strText;
	GetDlgItemText(IDC_EDIT1, strText);
	int width = std::stoi(strText.GetString());

	CString sInfo;

	// 获取用户选择的文件路径
	CString filePath = fileDialog.GetPathName();
	std::string  strFullPath = std::string(CT2A(filePath));


	Double3DPointVec data;
	CHVisionAdvX lHVisionAdvXObj;
	if (lHVisionAdvXObj.ReadLidarDevDataX(data, width, strFullPath.c_str())!=0)
	{
		sInfo.Format(_T("雷达：读取图像数据失败--%s "), strFullPath.c_str());
		LogX(sInfo);

		return;
	}


	//数据深拷贝到类成员
	m_DataLidarDev.clear();
	for (int i = 0; i < data.size(); i++)
	{
		std::vector<Point3D> Single3DPointVec;
		for (int j = 0; j < data[i].size(); j++)
		{
			Single3DPointVec.push_back(data[i][j].DeepCopy());
		}
		m_DataLidarDev.push_back(Single3DPointVec);
	}


	sInfo.Format(_T("雷达：读取图像完成--%s "), strFullPath.c_str());
	LogX(sInfo);

}

void CMfcTestqhxcAlgDlg::OnBnClickedButton10()
{
	// TODO: 在此添加控件通知处理程序代码


	CString sInfo;


	s_LidarTrans m_sLidarTrans;
	m_sLidarTrans.Y_Z_Change = true;
	m_sLidarTrans.X_Y_Change = false;
	m_sLidarTrans.X_mirro = false;
	m_sLidarTrans.Y_mirro = false;
	m_sLidarTrans.Map_Rotate = false;


	s_Lidar3d LidarT;

	CHVisionAdvX lHVisionAdvXObj;
	if (lHVisionAdvXObj.TransLidarDevData2Lidar3ds(m_DataLidarDev, m_sLidarTrans, LidarT) != 0)
	{
		sInfo.Format(_T("雷达：原始数据转换失败. "));
		LogX(sInfo);

		return;
	}

	sInfo.Format(_T("雷达：原始数据转换成功. "));
	LogX(sInfo);

	mLidarT = LidarT.DeepCopy();

	int width = mLidarT.GetMapWidth();
	int height = mLidarT.GetMapHeight();
	if (width>0&& height>0)
	{
		m_pListDlgImageWindow[0]->SetPart(0, 0, width - 1, height - 1);
	}
	


	m_pListDlgImageWindow[0]->ClearWindow();
	m_pListDlgImageWindow[0]->DispImage(LidarT.Z.Clone());

}





void CMfcTestqhxcAlgDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sInfo;

	CUnloadPlateA lProObj;


	m_sPreProcessLidar3dPara.fRoiXMin = -10000;
	m_sPreProcessLidar3dPara.fRoiXMax = 10000+1500;
	m_sPreProcessLidar3dPara.fRoiYMin = -2000;
	m_sPreProcessLidar3dPara.fRoiYMax = 2000;
	m_sPreProcessLidar3dPara.fRoiZMin = 3500;
	m_sPreProcessLidar3dPara.fRoiZMax = 6000+1000;

	m_sPreProcessLidar3dPara.fThSeg0Dis = 200;
	m_sPreProcessLidar3dPara.iThSeg0NumPtsMin = 20;
	m_sPreProcessLidar3dPara.iThSeg0NumPtsMax = 999999;
	m_sPreProcessLidar3dPara.fThSeg0DiameterMin = 200;
	m_sPreProcessLidar3dPara.fThSeg0DiameterMax = 999999;



	m_sPreProcessLidar3dPara.ProcessModel = 0;
	m_sPreProcessLidar3dPara.fProSetResolutionX = 20;
    m_sPreProcessLidar3dPara.fProSetResolutionY = 10;


	s_PreProcessLidar3dRts sPreProcessRts;
	s_Rtnf sRts =lProObj.OnPreProcess(mLidarT, m_sPreProcessLidar3dPara, sPreProcessRts);
	
	if (sRts.iCode != 0)
	{

		sInfo.Format(_T("雷达：预处理图像失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);

		return;
	}

	sInfo.Format(_T("雷达：预处理图像成功--%s "), sRts.strInfo.c_str());
	LogX(sInfo);

	m_sPreProcessLidar3dRts = sPreProcessRts.DeepCopy();

	int width = sPreProcessRts.sImage3dPro.GetMapWidth();
	int height = sPreProcessRts.sImage3dPro.GetMapHeight();
	if (width > 0 && height > 0)
	{
		m_pListDlgImageWindow[0]->SetPart(0, 0, width - 1, height - 1);
	}

	m_pListDlgImageWindow[0]->ClearWindow();
	m_pListDlgImageWindow[0]->DispImage(sPreProcessRts.sImage3dPro.Z.Clone());


}

void CMfcTestqhxcAlgDlg::LogX(CString str)
{
         m_pDlgInfoList->DisplayInfo(str);

		 m_Log.Log(str);

}


void CMfcTestqhxcAlgDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CString sInfo;






	m_sPreADPlateAPara.fThNZAbsVal = 0.8;  //提取平面法线参数

	m_sPreADPlateAPara.fThSegCbDis = 80; //铜跺分割距离
	m_sPreADPlateAPara.fThCbSelLMin = 3000;//选取铜跺的长宽参数 点数参数
	m_sPreADPlateAPara.fThCbSelLMax = 18000+4000;
	m_sPreADPlateAPara.fThCbSelWMin = 800;
	m_sPreADPlateAPara.fThCbSelWMax = 5000;
	m_sPreADPlateAPara.fThCbSelPtsMin = 2000;
	m_sPreADPlateAPara.fThCbSelPtsMax = 180000;
	m_sPreADPlateAPara.fThCbSelZMin = 4900/*+1600*/;
	m_sPreADPlateAPara.fThCbSelZMax = 5600/*+1600*/;


	m_sPreADPlateAPara.bDefectSpaceObj = true;//是否检测空间异物 
	m_sPreADPlateAPara.fDefectSpaceThZMin = 900;  //提取范围限制参数
	m_sPreADPlateAPara.fDefectSpaceThZMax = 3000;
	m_sPreADPlateAPara.fDefectSpaceThInnerLen = 150; //内缩长度 单侧内缩距离
	m_sPreADPlateAPara.fDefectSpaceThInnerWidth = 300; //内缩宽度 
	m_sPreADPlateAPara.fDefectSpaceThLen = 500.00;  //判定参数
	m_sPreADPlateAPara.fDefectSpaceThNumPts = 10;


	m_sPreADPlateAPara.bDefectLbObj = true;  //是否检测栏板
	m_sPreADPlateAPara.fDefectLbThZMin = 300;  //提取Z范围限制参数  相对于车板底面
	m_sPreADPlateAPara.fDefectLbThZMax = 1600;
	m_sPreADPlateAPara.fDefectLbThick = 300;
	m_sPreADPlateAPara.fDefectLbThInnerLen = 200;  //内缩长度
	m_sPreADPlateAPara.fDefectLbThInnerWidth = 100;//内缩宽度
	m_sPreADPlateAPara.fDefectLbThLen = 2000.00;   //判定参数-栏板长度
	m_sPreADPlateAPara.fDefectLbThWidth = 1000.00;  //判定参数-栏板宽度
	m_sPreADPlateAPara.fDefectLbThNumPts = 100;   //判定参数-栏板点数


	m_sPreADPlateAPara.fThSegTbDis = 60;   //铜板分割参数
	m_sPreADPlateAPara.fThTbSelLMin = 800;
	m_sPreADPlateAPara.fThTbSelLMax = 1500;
	m_sPreADPlateAPara.fThTbSelWMin = 800;
	m_sPreADPlateAPara.fThTbSelWMax = 1500;
	m_sPreADPlateAPara.fThTbSelPtsMin = 3500-800-1700;
	m_sPreADPlateAPara.fThTbSelPtsMax = 17000;
	m_sPreADPlateAPara.fThTbSelZMin = 4500/*+1400*/;
	m_sPreADPlateAPara.fThTbSelZMax = 5050/*+1400*/;
	m_sPreADPlateAPara.fJgTbCloseLenMin = 2000; //判断铜跺接触最小长度
	m_sPreADPlateAPara.fJgTbClosePtsMin = 4500; //判断铜跺接触最小点数


	CString strText;
	GetDlgItemText(IDC_EDIT2, strText);
	int ThSafeDisTbNear = std::stoi(strText.GetString());

	CString strText2;
	GetDlgItemText(IDC_EDIT3, strText2);
	int ThSafeDisToCbOuter = std::stoi(strText.GetString());

	m_sPreADPlateAPara.fThSafeDisTbNear = /*350*/ThSafeDisTbNear;  //铜跺之间的安全间距
	m_sPreADPlateAPara.fThSafeDisToCbOuter = /*400*/ThSafeDisToCbOuter; //铜跺和车板外边界的安全间距

	m_sPreADPlateAPara.fSampleTb = 80;  //铜跺下采样参数



	s_PreADPlateARtsPara sPreADPlateARtsPara;
	CUnloadPlateA lCUnloadPlateAObj;
	s_Rtnf sRts = lCUnloadPlateAObj.OnPreAD(m_sPreProcessLidar3dRts.sImage3dPro, m_sPreADPlateAPara, sPreADPlateARtsPara);
	//lock(m_LockRts)  //图像数据加锁  如能确保此刻数据不被其他线程访问 可取消加锁及深拷贝
	//{
		m_sPreADPlateARtsPara = sPreADPlateARtsPara.DeepCopy();
	//}


	if (sRts.iCode != 0)
	{
		sInfo.Format(_T("雷达：预定位失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);
		return;
	}

	HObject XYZ;
	Compose3(m_sPreProcessLidar3dRts.sImage3dPro.X, m_sPreProcessLidar3dRts.sImage3dPro.Y, m_sPreProcessLidar3dRts.sImage3dPro.Z, &XYZ);


    m_pListDlgImageWindow[0]->ClearWindow();
	//m_pListDlgImageWindow[0]->DispImage(m_sPreProcessLidar3dRts.sImage3dPro.Z.Clone());
	m_pListDlgImageWindow[0]->DispImage(XYZ.Clone());
	
	m_pListDlgImageWindow[0]->ClearObj();
	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionTbsPlus.Clone(),"green");
	

	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionDefectLbRts.Clone(), "white");
	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionDefectSpaceRts.Clone(), "red");
	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionDefectCloseRtsTb.Clone(), "yellow");
	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionDefectTbNearRts.Clone(), "blue");
	m_pListDlgImageWindow[0]->DispObj(sPreADPlateARtsPara.RegionDefectOuterRts.Clone(), "cyan");

	
	m_pListDlgImageWindow[0]->SetFont(12, "mono", true, false);
	for (size_t i = 0; i < sPreADPlateARtsPara.TbUpRtsList.size(); i++)
	{
		CString str;
		str.Format(_T("%d"), (i + 1));
		m_pListDlgImageWindow[0]->DispText(str, "green", (int)sPreADPlateARtsPara.TbUpRtsList[i].fCenterCol, (int)sPreADPlateARtsPara.TbUpRtsList[i].fCenterRow, false);
	}

		 


	sInfo.Format(_T("雷达：缺陷检测 耗时=%d ms "), (int)(sPreADPlateARtsPara.time * 1000));
	LogX(sInfo);


	sInfo.Format(_T("雷达：铜跺数量=%d  "), sPreADPlateARtsPara.TbUpRtsList.size());
	LogX(sInfo);


	sInfo.Format(_T("雷达：综合判定=%s  "), sPreADPlateARtsPara.bTJG ? _T("OK") : _T("NG"));
	LogX(sInfo);



	s_PreADPlateARtsPara As = sPreADPlateARtsPara;



	if (As.bExistDefectSpace)
	{
		sInfo.Format(_T("雷达：存在空间异物 数量=%d  "), As.fDefectSpaceRtsListLSel.size());
		LogX(sInfo);

		for (size_t i = 0; i < As.fDefectSpaceRtsListLSel.size(); i++)
		{
			sInfo.Format(_T("雷达：空间异物 长度=%d   高度=%d  "),(int) As.fDefectSpaceRtsListLSel[i], (int) As.fDefectSpaceRtsListZSel[i]);
			LogX(sInfo);
		}

	}

	if (As.bExistDefectLb)
	{
		sInfo.Format(_T("雷达：存在栏板."));
		LogX(sInfo);
	}


	if (As.bExitTbClose)
	{
		sInfo.Format(_T("雷达：铜跺位置接触."));
		LogX(sInfo);
	}

	if (As.bExitSafeDisTb)
	{
		sInfo.Format(_T("雷达：铜跺之间安全间距不够."));
		LogX(sInfo);
	}


	if (As.bExitSafeDisToCbOuter)
	{
		sInfo.Format(_T("雷达：铜跺到车板边缘安全间距不够."));
		LogX(sInfo);
	}

	for (size_t i = 0; i < As.TbUpRtsList.size(); i++)
	{
		sInfo.Format(_T("雷达：雷达转换坐标系下-铜跺[%d] 中心XYZ=[%.1f  ,%.1f  ,%.1f]  "), i , As.TbUpRtsList[i].fCenterX, As.TbUpRtsList[i].fCenterY,  As.TbUpRtsList[i].fCenterZ);
		LogX(sInfo);
	}


}

void CMfcTestqhxcAlgDlg::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码

	CString sInfo;

	m_sCalcPreAlignRtsPara.Reset();


	//标定结果参数
	m_sCalcPreAlignPara.fSX = 17721;   //天车拍照基准位


	//[1.003, -0.00146136, -0.0156262, 20022.0, 0.00157407, 1.00309, 0.0072255, 844.565, 0.0156153, -0.00724914, 1.00297, 23.8086]
	//[20022.0, 844.565, 23.8086, 359.587, 359.107, 0.0834795, 0]

	m_sCalcPreAlignPara.sPoseLidar2Crane.Reset(); //雷达转天车Pose  需保留7位有效小数  当前不在使用
	m_sCalcPreAlignPara.sPoseLidar2Crane.fTransX = 20022.0;
	m_sCalcPreAlignPara.sPoseLidar2Crane.fTransY = 844.565;
	m_sCalcPreAlignPara.sPoseLidar2Crane.fTransZ = 23.8086;
	m_sCalcPreAlignPara.sPoseLidar2Crane.fRotX = 359.587;
	m_sCalcPreAlignPara.sPoseLidar2Crane.fRotY = 359.107;
	m_sCalcPreAlignPara.sPoseLidar2Crane.fRotZ = 0.0834795;

	m_sCalcPreAlignPara.fLidar2Gnd = 6690; //当前不使用


	//雷达转天车矩阵  需保留7位有效小数！！！！！！！  
	double Mat[12]={ 1.003, -0.00146136, -0.0156262, 20022.0, 0.00157407, 1.00309, 0.0072255, 844.565, 0.0156153, -0.00724914, 1.00297, 23.8086 };
	for (size_t i = 0; i < 12; i++)
	{
		m_sCalcPreAlignPara.MatLidar2Crane[i] = Mat[i];
	}
	
	

	m_sCalcPreAlignPara.fX = 17719;   //当前拍照位 --需要实时写入

	m_sCalcPreAlignPara.fOffsetTbX = 0; //铜跺固定补偿
	m_sCalcPreAlignPara.fOffsetTbY = 0;
	m_sCalcPreAlignPara.fOffsetTbZ = 0;
	m_sCalcPreAlignPara.fOffsetTbDeg = 0;


	m_sCalcPreAlignPara.fCameraOffsetX = 0;  //相机拍照位相对于铜跺偏移量XYZ Deg
	m_sCalcPreAlignPara.fCameraOffsetY = 0;
	m_sCalcPreAlignPara.fCameraOffsetZ = 2500;
	m_sCalcPreAlignPara.fCameraOffsetDeg = 0;

	m_sCalcPreAlignPara.fLimitCameraOffsetZMin = 0; //拍照位限制范围
	m_sCalcPreAlignPara.fLimitCameraOffsetZMax = 7000; //
	m_sCalcPreAlignPara.fLimitCameraOffsetXMin = 0;
	m_sCalcPreAlignPara.fLimitCameraOffsetXMax = 50000;
	m_sCalcPreAlignPara.fLimitCameraOffsetYMin = 0;
	m_sCalcPreAlignPara.fLimitCameraOffsetYMax = 4000;
	m_sCalcPreAlignPara.fLimitCameraOffsetDegMin = 80;
	m_sCalcPreAlignPara.fLimitCameraOffsetDegMax = 105;



	s_CalcPreAlignRtsPara sCalcPreRts;
	CUnloadPlateA lCUnloadPlateAObj;
	s_Rtnf sRts = lCUnloadPlateAObj.CalcPreAlignRts(m_sPreADPlateARtsPara, m_sCalcPreAlignPara, sCalcPreRts);
	//lock(m_LockRts)  //图像数据加锁  如能确保此刻数据不被其他线程访问 可取消加锁及深拷贝
	//{
	m_sCalcPreAlignRtsPara= sCalcPreRts;
	//}


	if (sRts.iCode != 0)
	{
		sInfo.Format(_T("雷达：预定位计算失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);
		return;
	}

	if (sCalcPreRts.bTJG == false)
	{
		sInfo.Format(_T("雷达：预定位计算失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);
		return;
	}



	sInfo.Format(_T("雷达：天车拍照位[%.1f] 基准位=[%.1f ]  "), m_sCalcPreAlignPara.fX, m_sCalcPreAlignPara.fSX);
	LogX(sInfo);



	//打印计算结果--铜跺坐标
	for (size_t i = 0; i < sCalcPreRts.fTbCenterCrane.size(); i++)
	{
		s_PPts As = sCalcPreRts.fTbCenterCrane[i];

		sInfo.Format(_T("雷达：天车坐标系下-铜跺[%d] 中心XYZ=[%.1f  ,%.1f  ,%.1f]  Deg=[%.1f] "), i, As.fX, As.fY, As.fZ,As.fDeg);
		LogX(sInfo);
	}



	//打印计算结果--计算相机拍照坐标
	for (size_t i = 0; i < sCalcPreRts.fCamaraPosCrane.size(); i++)
	{
		s_PPts As = sCalcPreRts.fCamaraPosCrane[i];

		sInfo.Format(_T("雷达：天车坐标系下-相机拍照位[%d] 中心XYZ=[%.1f  ,%.1f  ,%.1f]  Deg=[%.1f] "), i, As.fX, As.fY, As.fZ, As.fDeg);
		LogX(sInfo);
	}


	


}




void CMfcTestqhxcAlgDlg::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码



	//HObject X1;  //图像坐标系--原始图像X-铜板区域
	//HObject Y1;  //图像坐标系--原始图像Y-铜板区域
	//HObject Z1;  //图像坐标系--原始图像Z-铜板区域

	//HObject Z1PZero; //图像调零坐标系--原始图像X-铜板区域
	//HObject X1PZero; //图像调零坐标系--原始图像Y-铜板区域
	//HObject Y1PZero; //图像调零坐标系--原始图像Y-铜板区域

	//HObject ImageSubZ1;  //高度图--相对周边区域
	//HObject ImageZ1ZeroReal; //高度图--相对整个零平面

	//HObject NzRender; //法线渲染图
	//HObject ZTbRender; //铜板区域高度渲染图
	//HObject ZLzRender; //粒子区域高度渲染图

	//CHVisionAdvX lHVisionAdvXObj;

	//std::string szFolderPath = "d:\\rts";
	//std::string sNameRemoveSuffix = "1";

	//lHVisionAdvXObj.WriteDefectPlateBRtsImage(m_sDefectPlateBRtsPara, szFolderPath, sNameRemoveSuffix);



}






void CMfcTestqhxcAlgDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码



	CFileDialog fileDialog(TRUE, _T("png"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("PNG Files (*.png)|*.png||"));

	// 弹出文件对话框
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}


	CString sInfo;


	// 获取用户选择的文件路径
	CString filePath = fileDialog.GetPathName();


	std::string  strFullPath = std::string(CT2A(filePath));

	std::string strFullPathNameRemoveSuffix;

	const std::string suffix = "_IMG_Texture_8Bit.png";
	strFullPathNameRemoveSuffix = strFullPath.substr(0, strFullPath.size() - suffix.size());


	
	m_pListDlgImageWindow[1]->ClearWindow();
	s_Image3dS imgT;
	CHVisionAdvX lHVisionAdvXObj;
	if (lHVisionAdvXObj.ReadImage3DX(imgT, strFullPathNameRemoveSuffix.c_str()))
	{

		sInfo.Format(_T("自动：读取图像失败--%s "), strFullPathNameRemoveSuffix.c_str());
		LogX(sInfo);

		return;
	}

	mImgTOrg[0] = imgT.DeepCopy();
	int width = mImgTOrg[0].GetMapWidth();
	int height = mImgTOrg[0].GetMapHeight();
	if (width > 0 && height > 0)
	{
		m_pListDlgImageWindow[1]->SetPart(0, 0, width - 1, height - 1);
	}


	m_pListDlgImageWindow[1]->DispImage(imgT.Gray.Clone());


	sInfo.Format(_T("自动：读取图像完成--%s "), strFullPathNameRemoveSuffix.c_str());
	LogX(sInfo);




}









void CMfcTestqhxcAlgDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码

	CFileDialog fileDialog(TRUE, _T("png"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("PNG Files (*.png)|*.png||"));

	// 弹出文件对话框
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}


	CString sInfo;


	// 获取用户选择的文件路径
	CString filePath = fileDialog.GetPathName();


	std::string  strFullPath = std::string(CT2A(filePath));

	std::string strFullPathNameRemoveSuffix;

	const std::string suffix = "_IMG_Texture_8Bit.png";
	strFullPathNameRemoveSuffix = strFullPath.substr(0, strFullPath.size() - suffix.size());


	m_pListDlgImageWindow[2]->ClearWindow();

	s_Image3dS imgT;
	CHVisionAdvX lHVisionAdvXObj;
	if (lHVisionAdvXObj.ReadImage3DX(imgT, strFullPathNameRemoveSuffix.c_str()))
	{


		sInfo.Format(_T("自动：读取图像失败--%s "), strFullPathNameRemoveSuffix.c_str());
		LogX(sInfo);

		return;
	}

	mImgTOrg[1] = imgT.DeepCopy();

	int width = mImgTOrg[1].GetMapWidth();
	int height = mImgTOrg[1].GetMapHeight();
	if (width > 0 && height > 0)
	{
		m_pListDlgImageWindow[2]->SetPart(0, 0, width - 1, height - 1);
	}

	m_pListDlgImageWindow[2]->DispImage(imgT.Gray.Clone());


	sInfo.Format(_T("自动：读取图像完成--%s "), strFullPathNameRemoveSuffix.c_str());
	LogX(sInfo);

}


void CMfcTestqhxcAlgDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码

	int iCam = 0;

	CString sInfo;

	CUnloadPlateA lProObj;


	m_sPreProcess3DSPara[iCam].fRoiZMin = -2800;
	m_sPreProcess3DSPara[iCam].fRoiZMax = -1200;
	m_sPreProcess3DSPara[iCam].fRoiXMin = -400;
	m_sPreProcess3DSPara[iCam].fRoiXMax = 1000;
	m_sPreProcess3DSPara[iCam].fRoiYMin = -1000;
	m_sPreProcess3DSPara[iCam].fRoiYMax = 1000;




	m_sPreProcess3DSPara[iCam].fThSeg0Dis  = 20;
	m_sPreProcess3DSPara[iCam].iThSeg0NumPtsMin  = 3;
	m_sPreProcess3DSPara[iCam].iThSeg0NumPtsMax  = 99999999;
	m_sPreProcess3DSPara[iCam].fThSeg0DiameterMin  = 5;
	m_sPreProcess3DSPara[iCam].fThSeg0DiameterMax  = 3000;


	s_PreProcess3DSResultPara sPreProcessRts;
	s_Rtnf sRts = lProObj.OnPreProcess(mImgTTool[iCam], m_sPreProcess3DSPara[iCam], sPreProcessRts);

	if (sRts.iCode != 0)
	{

		sInfo.Format(_T("自动：预处理图像失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);

		return;
	}

	sInfo.Format(_T("自动：预处理图像完成--耗时=%d ms "), (int)(sPreProcessRts.time * 1000));
	LogX(sInfo);

	m_sPreProcess3DSResultPara[iCam] = sPreProcessRts.DeepCopy();

	m_pListDlgImageWindow[iCam+1]->ClearWindow();
	m_pListDlgImageWindow[iCam+1]->DispImage(sPreProcessRts.sImage3dPro.Gray.Clone());


}

void CMfcTestqhxcAlgDlg::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码



	//相机转工具坐标系矩阵  需保留7位有效小数！！！！！！！  



	double Mat1[12] = { 0.0037917, -0.986526, -0.163562, 802.976, -0.990748, -0.0258952, 0.13322, -437.525, -0.13566, 0.161544, -0.977497, 2879.82 -2207};
	 double Mat2[12] = {-0.00692093, 0.992505, 0.122012, -669.506, 0.992222, 0.0219815, -0.122526, 415.139, -0.12429, 0.120215, -0.984937, 2884.28 -2207};





	for (size_t i = 0; i < 12; i++)
	{
		m_sCalcAccurateAlignPara.MatCameraA2Grip[i] = Mat1[i];
		//m_sCalcAccurateAlignPara.MatCameraB2Grip[i] = Mat2[i];
	}


	//***************************************************************************************************
	//变换坐标系

	int iCam = 0;

	HTuple hv_HomMat3D;
	hv_HomMat3D.Clear();
	for (size_t i = 0; i < 12; i++)
	{
		hv_HomMat3D[i] = m_sCalcAccurateAlignPara.MatCameraA2Grip[i];
	}

	HTuple hv_ObjectModel3D, hv_ObjectModel3DT;
	XyzToObjectModel3d(mImgTOrg[iCam].X, mImgTOrg[iCam].Y, mImgTOrg[iCam].Z, &hv_ObjectModel3D);
	AffineTransObjectModel3d(hv_ObjectModel3D, hv_HomMat3D, &hv_ObjectModel3DT);

	HObject ho_X, ho_Y, ho_Z;
	ObjectModel3dToXyz(&ho_X, &ho_Y, &ho_Z, hv_ObjectModel3DT, "from_xyz_map", HTuple(), HTuple());


	mImgTTool[iCam] = mImgTOrg[iCam].DeepCopy();
	if (ho_X.IsInitialized() && ho_X.CountObj() > 0) { mImgTTool[iCam].X = ho_X.Clone(); };
	if (ho_Y.IsInitialized() && ho_Y.CountObj() > 0) { mImgTTool[iCam].Y = ho_Y.Clone(); };
	if (ho_Z.IsInitialized() && ho_Z.CountObj() > 0) { mImgTTool[iCam].Z = ho_Z.Clone(); };


	//显示转换后的数据

	HObject XYZ;
	Compose3(mImgTTool[iCam].X, mImgTTool[iCam].Y, mImgTTool[iCam].Z, &XYZ);

	m_pListDlgImageWindow[iCam + 1]->ClearWindow();
	m_pListDlgImageWindow[iCam + 1]->DispImage(XYZ.Clone());



}



//void CMfcTestLzdmAlgDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	if (1 == nIDEvent)
//	{
//		InitUI();
//
//		KillTimer(1);
//	}
//
//	CDialogEx::OnTimer(nIDEvent);
//}



void CMfcTestqhxcAlgDlg::OnBnClickedButton9()
{
	// TODO: 相机2坐标系变换



	//相机转工具坐标系矩阵  需保留7位有效小数！！！！！！！  
	double Mat1[12] = { 0.0037917, -0.986526, -0.163562, 802.976, -0.990748, -0.0258952, 0.13322, -437.525, -0.13566, 0.161544, -0.977497, 2879.82 - 2207 };
	double Mat2[12] = { -0.00692093, 0.992505, 0.122012, -669.506, 0.992222, 0.0219815, -0.122526, 415.139, -0.12429, 0.120215, -0.984937, 2884.28 - 2207 };



	for (size_t i = 0; i < 12; i++)
	{
		//m_sCalcAccurateAlignPara.MatCameraA2Grip[i] = Mat1[i];
		m_sCalcAccurateAlignPara.MatCameraB2Grip[i] = Mat2[i];
	}


	//***************************************************************************************************
	//变换坐标系

	int iCam = 1;


	HTuple hv_HomMat3D;
	hv_HomMat3D.Clear();
	for (size_t i = 0; i < 12; i++)
	{
		hv_HomMat3D[i] = m_sCalcAccurateAlignPara.MatCameraB2Grip[i];
	}

	HTuple hv_ObjectModel3D, hv_ObjectModel3DT;
	XyzToObjectModel3d(mImgTOrg[iCam].X, mImgTOrg[iCam].Y, mImgTOrg[iCam].Z, &hv_ObjectModel3D);
	AffineTransObjectModel3d(hv_ObjectModel3D, hv_HomMat3D, &hv_ObjectModel3DT);

	HObject ho_X, ho_Y, ho_Z;
	ObjectModel3dToXyz(&ho_X, &ho_Y, &ho_Z, hv_ObjectModel3DT, "from_xyz_map", HTuple(), HTuple());


	mImgTTool[iCam] = mImgTOrg[iCam].DeepCopy();
	if (ho_X.IsInitialized() && ho_X.CountObj() > 0) { mImgTTool[iCam].X = ho_X.Clone(); };
	if (ho_Y.IsInitialized() && ho_Y.CountObj() > 0) { mImgTTool[iCam].Y = ho_Y.Clone(); };
	if (ho_Z.IsInitialized() && ho_Z.CountObj() > 0) { mImgTTool[iCam].Z = ho_Z.Clone(); };


	//显示转换后的数据

	HObject XYZ;
	Compose3(mImgTTool[iCam].X, mImgTTool[iCam].Y, mImgTTool[iCam].Z, &XYZ);

	m_pListDlgImageWindow[iCam+1]->ClearWindow();
	m_pListDlgImageWindow[iCam+1]->DispImage(XYZ.Clone());


	

}






void CMfcTestqhxcAlgDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMfcTestqhxcAlgDlg::OnBnClickedButton12()
{
	int iCam = 1;

	CString sInfo;

	CUnloadPlateA lProObj;


	m_sPreProcess3DSPara[iCam].fRoiZMin = -2800;
	m_sPreProcess3DSPara[iCam].fRoiZMax = -1200;
	m_sPreProcess3DSPara[iCam].fRoiXMin = -1000;
	m_sPreProcess3DSPara[iCam].fRoiXMax = 400;
	m_sPreProcess3DSPara[iCam].fRoiYMin = -1000;
	m_sPreProcess3DSPara[iCam].fRoiYMax = 1000;



	m_sPreProcess3DSPara[iCam].fThSeg0Dis = 20;
	m_sPreProcess3DSPara[iCam].iThSeg0NumPtsMin = 3;
	m_sPreProcess3DSPara[iCam].iThSeg0NumPtsMax = 99999999;
	m_sPreProcess3DSPara[iCam].fThSeg0DiameterMin = 5;
	m_sPreProcess3DSPara[iCam].fThSeg0DiameterMax = 3000;


	s_PreProcess3DSResultPara sPreProcessRts;
	s_Rtnf sRts = lProObj.OnPreProcess(mImgTTool[iCam], m_sPreProcess3DSPara[iCam], sPreProcessRts);

	if (sRts.iCode != 0)
	{

		sInfo.Format(_T("自动：预处理图像失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);

		return;
	}

	sInfo.Format(_T("自动：预处理图像完成--耗时=%d ms "), (int)(sPreProcessRts.time * 1000));
	LogX(sInfo);

	m_sPreProcess3DSResultPara[iCam] = sPreProcessRts.DeepCopy();

	m_pListDlgImageWindow[iCam + 1]->ClearWindow();
	m_pListDlgImageWindow[iCam + 1]->DispImage(sPreProcessRts.sImage3dPro.Gray.Clone());
}


void CMfcTestqhxcAlgDlg::OnBnClickedButton13()
{
	// TODO: 精定位计算

	CString sInfo;

	m_sCalcAccurateAlignRtsPara.Reset();



	 //对位基准 即标定板的位置 工具坐标下  
	// 将标定板的中心严格对准夹爪中心，并让标定板和天车运动方向平行 
	//  故XY = 0 DEG = 0 SZ为测量夹爪下方到标定板的垂直高度
	m_sCalcAccurateAlignPara.fSX = 0;    //天车拍照基准位（标定位）XYZ+DEG 
	m_sCalcAccurateAlignPara.fSY = 0;
	m_sCalcAccurateAlignPara.fSZ = -2100;
	m_sCalcAccurateAlignPara.fSDeg = 90;

	//当前拍照位坐标
	//m_sCalcAccurateAlignPara.fCameraX = 23885;   //天车当前拍照位坐标 需要实时输入 
	//m_sCalcAccurateAlignPara.fCameraY = 1235;
	//m_sCalcAccurateAlignPara.fCameraZ = 2258;
	//m_sCalcAccurateAlignPara.fCameraDeg = 90;


	CString strText;
	GetDlgItemText(IDC_EDIT4, strText);
	m_sCalcAccurateAlignPara.fCameraX = std::stoi(strText.GetString());


	GetDlgItemText(IDC_EDIT5, strText);
	m_sCalcAccurateAlignPara.fCameraY = std::stoi(strText.GetString());

	GetDlgItemText(IDC_EDIT6, strText);
	m_sCalcAccurateAlignPara.fCameraZ = std::stoi(strText.GetString());

	GetDlgItemText(IDC_EDIT7, strText);
	m_sCalcAccurateAlignPara.fCameraDeg = std::stoi(strText.GetString());





	m_sCalcAccurateAlignPara.fCameraSX = 20692;   //天车拍照基准位（标定位）XYZ+DEG 标定时记录的天车坐标 实际不使用
	m_sCalcAccurateAlignPara.fCameraSY = 2432;
	m_sCalcAccurateAlignPara.fCameraSZ = 2207;
	m_sCalcAccurateAlignPara.fCameraSDeg = 90;

	//double  MatCameraA2Grip[12];//相机1转工具坐标系矩阵 
	//double  MatCameraB2Grip[12];//相机2转工具坐标系矩阵 


	m_sCalcAccurateAlignPara.fOffsetTbX = 0.1; //铜跺固定补偿
	m_sCalcAccurateAlignPara.fOffsetTbY = 0.2;
	m_sCalcAccurateAlignPara.fOffsetTbZ = 0.3;
	m_sCalcAccurateAlignPara.fOffsetTbDeg = -0.01;


	m_sCalcAccurateAlignPara.fLimitRefMaxZ = 500; //对位数据最大值  为相对基准数据  每次对位发现超过最大值 直接报警 并结束对位
	m_sCalcAccurateAlignPara.fLimitRefMaxX = 500;
	m_sCalcAccurateAlignPara.fLimitRefMaxY = 500;
	m_sCalcAccurateAlignPara.fLimitRefMaxDeg = 10;


	m_sCalcAccurateAlignPara.fLimitGripZMin = -50; //抓取位限制范围
	m_sCalcAccurateAlignPara.fLimitGripZMax = 7000;
	m_sCalcAccurateAlignPara.fLimitGripXMin = 0;
	m_sCalcAccurateAlignPara.fLimitGripXMax = 50000;
	m_sCalcAccurateAlignPara.fLimitGripYMin = 0;
	m_sCalcAccurateAlignPara.fLimitGripYMax = 4000;
	m_sCalcAccurateAlignPara.fLimitGripDegMin = 60;
	m_sCalcAccurateAlignPara.fLimitGripDegMax = 300;

	m_sCalcAccurateAlignPara.fAccurateX = 20;   //对位精度xyz+deg
	m_sCalcAccurateAlignPara.fAccurateY = 20;
	m_sCalcAccurateAlignPara.fAccurateZ = 20;
	m_sCalcAccurateAlignPara.fAccurateDeg = 1.5;



	s_CalcAccurateAlignRtsPara sCalRts;
	CUnloadPlateA lCUnloadPlateAObj;
	s_Rtnf sRts = lCUnloadPlateAObj.CalcAccurateAlignRts(m_sAccurateADPlateARtsPara, m_sCalcAccurateAlignPara, sCalRts);
	//lock(m_LockRts)  //图像数据加锁  如能确保此刻数据不被其他线程访问 可取消加锁及深拷贝
	//{
	m_sCalcAccurateAlignRtsPara = sCalRts;
	//}


	if (sRts.iCode != 0)
	{
		sInfo.Format(_T("相机：精定位计算失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);
		return;
	}

	sInfo.Format(_T("相机：精定位计算结果 iStatus==%d "), sCalRts.iStatus);
	LogX(sInfo);


	sInfo.Format(_T("相机：相机基准拍照位SX=[%.1f] SY==[%.1f ] SZ=[%.1f] SDeg==[%.1f ]  "), m_sCalcAccurateAlignPara.fSX, m_sCalcAccurateAlignPara.fSY, m_sCalcAccurateAlignPara.fSZ, m_sCalcAccurateAlignPara.fSDeg);
	LogX(sInfo);

	sInfo.Format(_T("相机：相机当前拍照位X=[%.1f] Y==[%.1f ] Z=[%.1f] Deg==[%.1f ]  "), m_sCalcAccurateAlignPara.fCameraX, m_sCalcAccurateAlignPara.fCameraY, m_sCalcAccurateAlignPara.fCameraZ, m_sCalcAccurateAlignPara.fCameraDeg);
	LogX(sInfo);


	sInfo.Format(_T("相机：对位偏差dX=[%.1f] dY==[%.1f ] dZ=[%.1f] dDeg==[%.1f ]  "), sCalRts.dX, sCalRts.dY, sCalRts.dZ, sCalRts.dDeg);
	LogX(sInfo);


	sInfo.Format(_T("相机：下一个拍照位坐标 NextCameraX=[%.1f] NextCameraY=[%.1f ] NextCameraZ=[%.1f] NextCameraDeg=[%.1f ]  "), sCalRts.fNextCameraX, sCalRts.fNextCameraY, sCalRts.fNextCameraZ, sCalRts.fNextCameraDeg);
	LogX(sInfo);


	sInfo.Format(_T("相机：抓取位坐标 fGripX=[%.1f] fGripY==[%.1f ] fGripZ=[%.1f] fGripDeg=[%.1f ]  "), sCalRts.fGripX, sCalRts.fGripY, sCalRts.fGripZ, sCalRts.fGripDeg);
	LogX(sInfo);

	sInfo.Format(_T("相机：铜跺角度（天车旋转坐标系下 注意DegCrane=DegCam+180） fTdEarDeg=[%.1f]   "), sCalRts.fTdEarDeg);
	LogX(sInfo);


	sInfo.Format(_T("相机：铜跺方向  |0:0度方向 1:90度方向  2:180度方向:3：270度方向| iTdEarDir=[%d]   "), sCalRts.iTdEarDir);
	LogX(sInfo);


	sInfo.Format(_T("相机：铜跺尺寸 L=[%.1f] W==[%.1f ] H=[%.1f] "), sCalRts.TdL, sCalRts.TdW, sCalRts.TdH);
	LogX(sInfo);

	
	if (sCalRts.iStatus==1)
	{
		sInfo.Format(_T("相机：对位完成. "));
	}
	else if (sCalRts.iStatus == 2)
	{
		sInfo.Format(_T("相机：对位失败. "));
	}
	else if (sCalRts.iStatus == 5)
	{
		sInfo.Format(_T("相机：继续对位. "));
	}
	else
	{
		sInfo.Format(_T("相机：对位状态异常. "));
	}
}


void CMfcTestqhxcAlgDlg::OnBnClickedButton14()
{
	// TODO: 执行相机定位


	// TODO: 在此添加控件通知处理程序代码
	CString sInfo;

    m_sAccurateADPlateAPara.fThCbNZAbsValMin = 0.95;  //车板法线筛选
	m_sAccurateADPlateAPara.fThCbNZAbsValMax = 1;

	m_sAccurateADPlateAPara.fThCbNXAbsValMin = 0;
	m_sAccurateADPlateAPara.fThCbNXAbsValMax = 0.4;
	m_sAccurateADPlateAPara.fThCbNYAbsValMin = 0;
	m_sAccurateADPlateAPara.fThCbNYAbsValMax = 0.35;

	m_sAccurateADPlateAPara.fThSegCbDis = 20;   //车板分割距离
	m_sAccurateADPlateAPara.fThCbSelLMin = 500;  //选择车板的长宽 点数 
	m_sAccurateADPlateAPara.fThCbSelLMax = 5000;
	m_sAccurateADPlateAPara.fThCbSelWMin = 500;
	m_sAccurateADPlateAPara.fThCbSelWMax = 5000;
	m_sAccurateADPlateAPara.fThCbSelPtsMin = 2000;
	m_sAccurateADPlateAPara.fThCbSelPtsMax = 2000000;
	m_sAccurateADPlateAPara.fThCbSelZMin = -2600;   //选择车板的Z值范围  该值在标定后坐标系下 为车板相对于夹爪底部的距离范围
	m_sAccurateADPlateAPara.fThCbSelZMax = -2000;


	m_sAccurateADPlateAPara.fThTbNZAbsVal = 0.90; //铜跺法线提取
	m_sAccurateADPlateAPara.fThSegTbDis = 10;  //铜跺分割距离
	m_sAccurateADPlateAPara.fThTbSelLMin = 800;    //选择铜跺的长宽 点数 
	m_sAccurateADPlateAPara.fThTbSelLMax = 1500;
	m_sAccurateADPlateAPara.fThTbSelWMin = 800;
	m_sAccurateADPlateAPara.fThTbSelWMax = 1500;
	m_sAccurateADPlateAPara.fThTbSelPtsMin = 50000;
	m_sAccurateADPlateAPara.fThTbSelPtsMax = 1500000;
	m_sAccurateADPlateAPara.fThTbSelZMin = -2100;    //选择铜跺的Z值范围  该值在标定后坐标系下 为铜跺相对于夹爪底部的距离范围 
	m_sAccurateADPlateAPara.fThTbSelZMax = -1600;


	m_sAccurateADPlateAPara.fThSafeDisTdNear = 350;   //铜跺安全距离
	m_sAccurateADPlateAPara.fDefectTdNearSelZRefCb = 100; //检测范围相对于车板高度
	m_sAccurateADPlateAPara.iDefectTdNearRoiWDilation = 201; //检测范围宽度方向扩展像素
	m_sAccurateADPlateAPara.iDefectTdNearRoiLErosion = 401; //检测范围长度方向内缩像素 

	m_sAccurateADPlateAPara.iBmRoiWDilationW = 301; //板面宽度方向ROI在宽度方向扩展
	m_sAccurateADPlateAPara.iBmRoiWErosionL = 65; //板面宽度方向ROI在长度方向内缩
	m_sAccurateADPlateAPara.iBmRoiLDilationL = 101; //板面长度方向ROI在长度方向扩展

	m_sAccurateADPlateAPara.fOffsetZSelTbRelCb = 30; //选择相对于车板高度的铜跺 

	m_sAccurateADPlateAPara.fEdgeWLenUse = 800;  //宽度方向边缘长度使用
	m_sAccurateADPlateAPara.fGndSelEdgeWOffset = 300;  //宽度方向边缘偏离一定距离作为近底面
	m_sAccurateADPlateAPara.fGndSelEdgeWLen = 600;  //近底面的长度方向长度
	m_sAccurateADPlateAPara.fGripUp = 0;  //吊钩底部相对于底板上偏移

	m_sAccurateADPlateAPara.fLimitDegMin = 80;  //限制范围报警-铜跺角度
	m_sAccurateADPlateAPara.fLimitDegMax = 100;

	m_sAccurateADPlateAPara.fLimitTdWMin = 900;  //限制范围报警-铜跺的长宽高
	m_sAccurateADPlateAPara.fLimitTdWMax = 1100.0;
	m_sAccurateADPlateAPara.fLimitTdLMin = 980;
	m_sAccurateADPlateAPara.fLimitTdLMax = 1200;
	m_sAccurateADPlateAPara.fLimitTdHMin = 300;
	m_sAccurateADPlateAPara.fLimitTdHMax = 600;

	m_sAccurateADPlateAPara.fThTbLayerOuter = 100;//层错阈值  定义成下面的板子相对于最上面一块板的外偏移（爪子位置范围内）

	m_sAccurateADPlateAPara.fDefectGripSpaceRoiXOffeset = 10; //检测夹爪区域ROI范围--偏离铜跺外边缘作为X起始
	m_sAccurateADPlateAPara.fDefectGripSpaceRoiXLen = 200; //检测夹爪区域ROI范围--Y方向长度
	m_sAccurateADPlateAPara.fDefectGripSpaceRoiYLen = 600; //检测夹爪区域ROI范围--Y方向长度
	m_sAccurateADPlateAPara.fDefectGripSpaceRoiZOffesetCb = 35; //检测夹爪区域ROI范围--高于底面一定高度的空间数据
	m_sAccurateADPlateAPara.fDefectGripSpaceRoiZLen = 500;//检测夹爪区域ROI范围--Z方向长度
	m_sAccurateADPlateAPara.fDefectGripSpaceThLen = 50.00;  //判定参数--抓取范围内空间异物
	m_sAccurateADPlateAPara.fDefectGripSpaceThNumPts = 10;

	std::vector<s_Image3dS> ImgList;
	ImgList.push_back(m_sPreProcess3DSResultPara[0].sImage3dPro.DeepCopy());
	ImgList.push_back(m_sPreProcess3DSResultPara[1].sImage3dPro.DeepCopy());

	s_AccurateADPlateARtsPara sAccurateADPlateARtsPara;
	CUnloadPlateA lCUnloadPlateAObj;
	s_Rtnf sRts = lCUnloadPlateAObj.OnAccurateAD(ImgList, m_sAccurateADPlateAPara, sAccurateADPlateARtsPara);
	//lock(m_LockRts)  //图像数据加锁  如能确保此刻数据不被其他线程访问 可取消加锁及深拷贝
	//{
	m_sAccurateADPlateARtsPara = sAccurateADPlateARtsPara.DeepCopy();
	//}

   //定位不成功的情况下或者结果综合判定NG 不抓取
	if (sRts.iCode != 0 /*|| sAccurateADPlateARtsPara.bTJG == false*/)
	{
		sInfo.Format(_T("相机：精定位失败--%s "), sRts.strInfo.c_str());
		LogX(sInfo);
	
	}


	//打印一些检测参数
	s_AccurateADPlateARtsPara As = sAccurateADPlateARtsPara;

	sInfo.Format(_T("相机：定位检测 耗时=%d ms "), (int)(As.time * 1000));
	LogX(sInfo);


	if (As.bExitSafeDisTd)
	{
		sInfo.Format(_T("相机：存在铜跺安全距离超限 两侧距离=[%.1f  ,%.1f]  "), As.fDefectTdNearRtsDis1, As.fDefectTdNearRtsDis2);
		LogX(sInfo);
	}

	if (As.bExitLimitSize)
	{
		sInfo.Format(_T("相机：存在角度和尺寸数据超限制."));   //打印一下限制数据
		LogX(sInfo);
	}

	sInfo.Format(_T("相机： 层错值=[%.1f,%.1f ]"), As.fTbLayerOuterDis1, As.fTbLayerOuterDis2);
	LogX(sInfo);
	if (As.bExitTbLayerOuter)
	{
		sInfo.Format(_T("相机：存在层错超限制. "));
		LogX(sInfo);
	}



	if (As.bExistDefectGripSpace)
	{
		sInfo.Format(_T("相机：存在夹爪区域空间干涩."));   //打印一下限制数据
		LogX(sInfo);

		for (size_t i = 0; i < As.iDefectGripSpaceRtsListPtsSel.size(); i++)
		{
			sInfo.Format(_T("相机：干涩点云数量点=[%d] 长度=[%.1f ]"), As.iDefectGripSpaceRtsListPtsSel[i], As.fDefectGripSpaceRtsListLSel[i]);
			LogX(sInfo);
		}

	}

	sInfo.Format(_T("相机：铜跺尺寸 L=[%.1f] W==[%.1f ] H=[%.1f] "), As.TdL, As.TdW, As.TdH);
	LogX(sInfo);


	sInfo.Format(_T("相机：综合判定=%s  "), As.bTJG ? _T("OK") : _T("NG"));
	LogX(sInfo);

	if (As.bTJG ==false)
	{
		return;
	}


	//正常情况 显示和发送数据结果
	HObject XYZ1, XYZ2;
	Compose3(ImgList[0].X, ImgList[0].Y, ImgList[0].Z, &XYZ1);
	Compose3(ImgList[1].X, ImgList[1].Y, ImgList[1].Z, &XYZ2);

	m_pListDlgImageWindow[1]->ClearWindow();
	m_pListDlgImageWindow[1]->DispImage(XYZ1.Clone());

	m_pListDlgImageWindow[1]->ClearObj();
	m_pListDlgImageWindow[1]->DispObj(sAccurateADPlateARtsPara.RegionTDW1.Clone(), "green");
	m_pListDlgImageWindow[1]->DispObj(sAccurateADPlateARtsPara.RegionTDL1.Clone(), "cyan");


	m_pListDlgImageWindow[2]->ClearWindow();
	m_pListDlgImageWindow[2]->DispImage(XYZ2.Clone());

	m_pListDlgImageWindow[2]->ClearObj();
	m_pListDlgImageWindow[2]->DispObj(sAccurateADPlateARtsPara.RegionTDW2.Clone(), "green");
	m_pListDlgImageWindow[2]->DispObj(sAccurateADPlateARtsPara.RegionTDL2.Clone(), "cyan");


	sInfo.Format(_T("相机：铜跺抓取位置 TdX=[%.1f] TdY=[%.1f ] TdZ=[%.1f] TdDeg==[%.1f ]  "), As.TdX, As.TdY, As.TdZ, As.TdDeg);
	LogX(sInfo);

	sInfo.Format(_T("相机：铜跺方向  |0:0度方向 1:90度方向  2:180度方向:3：270度方向| iTdEarDir=[%d]   "), As.TdEarDir);
	LogX(sInfo);





}


void CMfcTestqhxcAlgDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}


void CMfcTestqhxcAlgDlg::OnEnChangeEdit5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CMfcTestqhxcAlgDlg::OnBnClickedButton15()
{
	// TODO: 在此添加控件通知处理程序代码



	CFileDialog fileDialog(TRUE, _T("dat"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("PNG Files (*.ly)|*.dat||"));

	// 弹出文件对话框
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}

	// 获取用户选择的文件路径
	CString filePath = fileDialog.GetPathName();
	std::string  strFullPath = std::string(CT2A(filePath));

	CHVisionAdvX lHVisionAdvXObj;


	CString sInfo;
	Double3DPointVec data;
	std::string err;


	// 加载
	if (!lHVisionAdvXObj.LoadDouble3DPointVecFromXYZMap(strFullPath.c_str(), data, err)) 
	{
		sInfo.Format(_T("雷达：读取图像数据失败--%s "), strFullPath.c_str());
		LogX(sInfo);

		return;
	}


	//数据深拷贝到类成员
	m_DataLidarDev.clear();
	for (int i = 0; i < data.size(); i++)
	{
		std::vector<Point3D> Single3DPointVec;
		for (int j = 0; j < data[i].size(); j++)
		{
			Single3DPointVec.push_back(data[i][j].DeepCopy());
		}
		m_DataLidarDev.push_back(Single3DPointVec);
	}

	sInfo.Format(_T("雷达：读取图像完成--%s "), strFullPath.c_str());
	LogX(sInfo);




}


void CMfcTestqhxcAlgDlg::OnBnClickedButton16()
{
	// TODO: 在此添加控件通知处理程序代码


	// 构造“另存为”对话框
	CFileDialog fileDialog(
		FALSE,                          // FALSE = 保存模式
		_T("dat"),                      // 默认扩展名
		_T("radar_data.dat"),           // 默认文件名
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("DAT Files (*.dat)|*.dat|All Files (*.*)|*.*||")
	);

	// 弹出对话框
	if (fileDialog.DoModal() != IDOK)
	{
		return;
	}

	CString filePath = fileDialog.GetPathName();
	std::string  strFullPath = std::string(CT2A(filePath));




	CHVisionAdvX lHVisionAdvXObj;
	CString sInfo;
	std::string err;

	// 保存
	if (!lHVisionAdvXObj.SaveDouble3DPointVecAsXYZMap(strFullPath, m_DataLidarDev, err))
	{
		sInfo.Format(_T("雷达：保存雷达原始图像数据失败."));
		LogX(sInfo);
	}
	else
	{
		sInfo.Format(_T("雷达：保存雷达原始图像数据完成."));
		LogX(sInfo);
	}

}


void CMfcTestqhxcAlgDlg::OnBnClickedButton17()
{
	// TODO: 在此添加控件通知处理程序代码

	//// 构造“另存为”对话框
	//CFileDialog fileDialog(
	//	FALSE,                          // FALSE = 保存模式
	//	_T("ply"),                      // 默认扩展名
	//	_T("radar_data.ply"),           // 默认文件名
	//	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
	//	_T("DAT Files (*.ply)|*.ply|All Files (*.*)|*.*||")
	//);

	//// 弹出对话框
	//if (fileDialog.DoModal() != IDOK)
	//{
	//	return;
	//}

	//CString filePath = fileDialog.GetPathName();
	//std::string  strFullPath = std::string(CT2A(filePath));




	//CHVisionAdvX lHVisionAdvXObj;
	//CString sInfo;
	//std::string err;

	//// 保存
	//if (!lHVisionAdvXObj.SaveDouble3DPointVecAsPlyWithHalcon(strFullPath, m_DataLidarDev, err))
	//{
	//	sInfo.Format(_T("雷达：保存雷达原始图像数据Ply失败."));
	//	LogX(sInfo);
	//}
	//else
	//{
	//	sInfo.Format(_T("雷达：保存雷达原始图像数据Ply完成."));
	//	LogX(sInfo);
	//}

}
