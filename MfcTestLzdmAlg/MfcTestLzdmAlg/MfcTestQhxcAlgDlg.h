
// MfcTestLzdmAlgDlg.h: 头文件
//

#pragma once

#include "DlgImageWindow.h"
#include "CHVisionAdvX.h"
#include "DlgInfoList.h"

#include "LogX.h"

// CMfcTestLzdmAlgDlg 对话框
class CMfcTestqhxcAlgDlg : public CDialogEx
{
// 构造
public:
	CMfcTestqhxcAlgDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCTESTLZDMALG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton5();

	void InitUI();

	void InitView();

	void LogX(CString str);



private:
	bool RegionSegmentByMatrix(CRect Rect1, int nRow, int nCol, CRect* pRect2);
public:
	
	void DelayT(DWORD time);
	void  DoEvents();

	//属性
	vector<CDlgImageWindow*> m_pListDlgImageWindow;
	CDlgInfoList* m_pDlgInfoList;
	CLogX m_Log;


	
	Double3DPointVec m_DataLidarDev;//雷达原始数据--SDK格式
	s_Lidar3d mLidarT;//雷达原始数据--H格式


	s_PreProcessLidar3dPara m_sPreProcessLidar3dPara;  //雷达数据预处理输入
	s_PreProcessLidar3dRts m_sPreProcessLidar3dRts;//雷达数据预处理结果

	s_PreADPlateAPara m_sPreADPlateAPara;    //初定位参数
	s_PreADPlateARtsPara m_sPreADPlateARtsPara;  //初定位结果参数

	s_CalcPreAlignPara m_sCalcPreAlignPara;    //粗定位计算参数
	s_CalcPreAlignRtsPara m_sCalcPreAlignRtsPara; //粗定位计算结果


	s_Image3dS mImgTOrg[2];    //相机数据--原始
	s_Image3dS mImgTTool[2];    //相机数据--工具坐标系

	s_PreProcess3DSPara m_sPreProcess3DSPara[2];  //相机预处理参数
	s_PreProcess3DSResultPara m_sPreProcess3DSResultPara[2]; //相机预处理结果


	s_AccurateADPlateAPara m_sAccurateADPlateAPara;    //精定位参数
	s_AccurateADPlateARtsPara m_sAccurateADPlateARtsPara;  //精定位结果参数

	s_CalcAccurateAlignPara m_sCalcAccurateAlignPara; //精定位计算输入
	s_CalcAccurateAlignRtsPara m_sCalcAccurateAlignRtsPara; //精定位结果



	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton4();
	//afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton16();
	afx_msg void OnBnClickedButton17();
};
