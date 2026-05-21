#pragma once

#include "halconcpp.h"
#include <vector>
using namespace HalconCpp;

#include <vector>
using namespace std;


// CDlgImageWindow 对话框

class CDlgImageWindow : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgImageWindow)

public:
	CDlgImageWindow(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgImageWindow();

// 对话框数据
	enum { IDD = IDD_DLGIMAGEWINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	//afx_msg void OnTimer(UINT_PTR nIDEvent);
	void disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box);
	void set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold,HTuple hv_Slant);
	
	void gen_arrow_contour_xld (HObject *Arrow, HTuple Row1, HTuple Column1, HTuple Row2, HTuple Column2, HTuple HeadLength, HTuple HeadWidth);

	HTuple m_hWindow;
	HObject m_image;
	HObject m_object;
	float  m_fScale;
	bool m_bCross;
	int m_iGrayValue;  
	int  m_iCountRButtonUp;
	bool m_bEditing;
	bool m_bDisableMenu;
	bool m_bDynamic;

	CStatic m_picDraw;

	
	
public:
	void InitHWindow();

	void DispImage(HObject image ,bool bClearEindow=FALSE ,bool bTans=TRUE);
	void DispObj(HObject obj,CString sFormat,  bool	bClearEindow=false);
	void DispText(CString sText,CString sFormat,int cx,int cy,bool	bClearEindow=false);
	void SetFont( int Size, CString Font= "mono", bool bBold=false, bool bSlant=false);
	void SetPart(int iLeft = 0, int iTop = 0, int iRight = 2592, int iBotton = 1944);
	


	void DisplayCrossCenterLine(void);
	void ClearWindow();
	void ClearImage();
	void ClearObj();

	void GetWindowImage(HObject &image);

	void SetLineWidthX(int iWidth);


};
