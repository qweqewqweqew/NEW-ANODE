// DlgImageWindow.cpp : 实现文件
//

#include "pch.h"
#include "MfcTestQhxcAlg.h"
#include "DlgImageWindow.h"
#include "afxdialogex.h"


// CDlgImageWindow 对话框

IMPLEMENT_DYNAMIC(CDlgImageWindow, CDialogEx)

	CDlgImageWindow::CDlgImageWindow(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgImageWindow::IDD, pParent)
{

	m_hWindow.Clear();

	m_bDynamic=false;
	m_fScale=1.0;
	m_bCross=false;
	m_iGrayValue=0;  
	m_iCountRButtonUp=0;

	m_bDisableMenu=0;

	
}

CDlgImageWindow::~CDlgImageWindow()
{

}

void CDlgImageWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgImageWindow, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()

	//ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgImageWindow 消息处理程序


BOOL CDlgImageWindow::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_picDraw.SubclassDlgItem(IDS_VIEW,this);

	SetTimer(1,2000,false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgImageWindow::InitHWindow()
{
	//布置控件
	CRect rcDlg;
	GetClientRect(&rcDlg);

	GetDlgItem(IDS_VIEW)->MoveWindow(rcDlg.left,rcDlg.top,rcDlg.Width(),rcDlg.Height()/*-iHeightStatius*/-2);

	if (m_hWindow>0)
	{
		HalconCpp::CloseWindow(m_hWindow);
		m_hWindow.Clear();
	}

	CRect	tmpRect;
	GetClientRect(&tmpRect);
	OpenWindow(tmpRect.top,tmpRect.left,tmpRect.Width(),tmpRect.Height(),(Hlong)(GetDlgItem(IDS_VIEW)->GetSafeHwnd()),"visible","",&m_hWindow);
	SetDraw(m_hWindow,"margin");
	SetColored(m_hWindow,3);  
	SetLineWidth(m_hWindow,2);
	SetSystem("tsp_width",5000);
	SetSystem("tsp_height",4000);
	HalconCpp::SetPart(m_hWindow,0,0,1944,2592);


}


void CDlgImageWindow::ClearWindow()
{
	if (m_hWindow.Length()==0)
	{
		return;
	}


	HalconCpp::ClearWindow(m_hWindow);
}

void CDlgImageWindow::ClearObj()
{
	m_object.Clear();
}

void CDlgImageWindow::ClearImage()
{
	m_image.Clear();
	
}


void CDlgImageWindow::DispImage(HObject image ,bool bClearEindow ,bool bTans)
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}

	if (bTans)
	{
		m_image=image;
	}

	if (bClearEindow)
	{
		HalconCpp::ClearWindow(m_hWindow);
	}

	
	if (!image.IsInitialized() || image.CountObj() == 0)
	{
		return;
	}
	HalconCpp::DispObj(image,m_hWindow);

}


void CDlgImageWindow::SetLineWidthX(int iWidth)
{

	try
	{
		SetLineWidth(m_hWindow, iWidth);
	}
	catch (HalconCpp::HException& ex)
	{

	}

}


void CDlgImageWindow::DispObj(HObject obj,CString sFormat,  bool	bClearEindow /*, bool bTans*/)
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}

	HTuple hClor=HTuple(sFormat);

	if (bClearEindow)
	{
		HalconCpp::ClearWindow(m_hWindow);
	}

	if (!obj.IsInitialized() || obj.CountObj() == 0)
	{
		return;
	}

	//if (bTans)
	//{
	//	if (!m_object.IsInitialized() )
	//	{
	//		GenEmptyObj(&m_object);
	//	}
	//	
	//	ConcatObj(m_object, obj, &m_object);
	//}

	SetLineWidth(m_hWindow,2);

	SetColor(m_hWindow,HTuple(sFormat));

	HalconCpp::DispObj(obj,m_hWindow);

}


void CDlgImageWindow::SetFont(int Size, CString Font , bool bBold , bool bSlant )
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}
	HTuple hBold, hSlant;
	bBold ? hBold = "true" : hBold = "false";
	bSlant ? hSlant = "true" : hSlant = "false";
	set_display_font(m_hWindow, Size,HTuple( Font), hBold, hSlant);

}

void CDlgImageWindow::DispText(CString sText,CString sFormat,int cx,int cy,bool	bClearEindow)
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}

	if (bClearEindow)
	{
		HalconCpp::ClearWindow(m_hWindow);
	}

	disp_message(m_hWindow, HTuple(sText), "image", cy, cx, HTuple(sFormat), "false");
	
}


void CDlgImageWindow::SetPart(int iLeft,int iTop, int iRight, int iBotton)
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}

	HalconCpp::SetPart(m_hWindow,iTop,iLeft,iBotton,iRight);
}




// Procedures 
// Chapter: Graphics / Text
// Short Description: This procedure writes a text message. 
void CDlgImageWindow::disp_message(HTuple hv_WindowHandle, HTuple hv_String, HTuple hv_CoordSystem,
	HTuple hv_Row, HTuple hv_Column, HTuple hv_Color, HTuple hv_Box)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_GenParamName, hv_GenParamValue;

	//This procedure displays text in a graphics window.
	//
	//Input parameters:
	//WindowHandle: The WindowHandle of the graphics window, where
	//   the message should be displayed
	//String: A tuple of strings containing the text message to be displayed
	//CoordSystem: If set to 'window', the text position is given
	//   with respect to the window coordinate system.
	//   If set to 'image', image coordinates are used.
	//   (This may be useful in zoomed images.)
	//Row: The row coordinate of the desired text position
	//   A tuple of values is allowed to display text at different
	//   positions.
	//Column: The column coordinate of the desired text position
	//   A tuple of values is allowed to display text at different
	//   positions.
	//Color: defines the color of the text as string.
	//   If set to [], '' or 'auto' the currently set color is used.
	//   If a tuple of strings is passed, the colors are used cyclically...
	//   - if |Row| == |Column| == 1: for each new textline
	//   = else for each text position.
	//Box: If Box[0] is set to 'true', the text is written within an orange box.
	//     If set to' false', no box is displayed.
	//     If set to a color string (e.g. 'white', '#FF00CC', etc.),
	//       the text is written in a box of that color.
	//     An optional second value for Box (Box[1]) controls if a shadow is displayed:
	//       'true' -> display a shadow in a default color
	//       'false' -> display no shadow
	//       otherwise -> use given string as color string for the shadow color
	//
	//It is possible to display multiple text strings in a single call.
	//In this case, some restrictions apply:
	//- Multiple text positions can be defined by specifying a tuple
	//  with multiple Row and/or Column coordinates, i.e.:
	//  - |Row| == n, |Column| == n
	//  - |Row| == n, |Column| == 1
	//  - |Row| == 1, |Column| == n
	//- If |Row| == |Column| == 1,
	//  each element of String is display in a new textline.
	//- If multiple positions or specified, the number of Strings
	//  must match the number of positions, i.e.:
	//  - Either |String| == n (each string is displayed at the
	//                          corresponding position),
	//  - or     |String| == 1 (The string is displayed n times).
	//
	//
	//Convert the parameters for disp_text.
	if (0 != (HTuple(int(hv_Row == HTuple())).TupleOr(int(hv_Column == HTuple()))))
	{
		return;
	}
	if (0 != (int(hv_Row == -1)))
	{
		hv_Row = 12;
	}
	if (0 != (int(hv_Column == -1)))
	{
		hv_Column = 12;
	}
	//
	//Convert the parameter Box to generic parameters.
	hv_GenParamName = HTuple();
	hv_GenParamValue = HTuple();
	if (0 != (int((hv_Box.TupleLength()) > 0)))
	{
		if (0 != (int(HTuple(hv_Box[0]) == HTuple("false"))))
		{
			//Display no box
			hv_GenParamName = hv_GenParamName.TupleConcat("box");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (int(HTuple(hv_Box[0]) != HTuple("true"))))
		{
			//Set a color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("box_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[0]));
		}
	}
	if (0 != (int((hv_Box.TupleLength()) > 1)))
	{
		if (0 != (int(HTuple(hv_Box[1]) == HTuple("false"))))
		{
			//Display no shadow.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow");
			hv_GenParamValue = hv_GenParamValue.TupleConcat("false");
		}
		else if (0 != (int(HTuple(hv_Box[1]) != HTuple("true"))))
		{
			//Set a shadow color other than the default.
			hv_GenParamName = hv_GenParamName.TupleConcat("shadow_color");
			hv_GenParamValue = hv_GenParamValue.TupleConcat(HTuple(hv_Box[1]));
		}
	}
	//Restore default CoordSystem behavior.
	if (0 != (int(hv_CoordSystem != HTuple("window"))))
	{
		hv_CoordSystem = "image";
	}
	//
	if (0 != (int(hv_Color == HTuple(""))))
	{
		//disp_text does not accept an empty string for Color.
		hv_Color = HTuple();
	}
	//
	HalconCpp::DispText(hv_WindowHandle, hv_String, hv_CoordSystem, hv_Row, hv_Column, hv_Color,
		hv_GenParamName, hv_GenParamValue);
	return;
}

// Chapter: Graphics / Text
// Short Description: Set font independent of OS 
void CDlgImageWindow::set_display_font(HTuple hv_WindowHandle, HTuple hv_Size, HTuple hv_Font, HTuple hv_Bold,
	HTuple hv_Slant)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_OS, hv_Fonts, hv_Style, hv_Exception;
	HTuple  hv_AvailableFonts, hv_Fdx, hv_Indices;

	//This procedure sets the text font of the current window with
	//the specified attributes.
	//
	//Input parameters:
	//WindowHandle: The graphics window for which the font will be set
	//Size: The font size. If Size=-1, the default of 16 is used.
	//Bold: If set to 'true', a bold font is used
	//Slant: If set to 'true', a slanted font is used
	//
	GetSystem("operating_system", &hv_OS);
	if (0 != (HTuple(int(hv_Size == HTuple())).TupleOr(int(hv_Size == -1))))
	{
		hv_Size = 16;
	}
	if (0 != (int((hv_OS.TupleSubstr(0, 2)) == HTuple("Win"))))
	{
		//Restore previous behaviour
		hv_Size = (1.13677*hv_Size).TupleInt();
	}
	else
	{
		hv_Size = hv_Size.TupleInt();
	}
	if (0 != (int(hv_Font == HTuple("Courier"))))
	{
		hv_Fonts.Clear();
		hv_Fonts[0] = "Courier";
		hv_Fonts[1] = "Courier 10 Pitch";
		hv_Fonts[2] = "Courier New";
		hv_Fonts[3] = "CourierNew";
		hv_Fonts[4] = "Liberation Mono";
	}
	else if (0 != (int(hv_Font == HTuple("mono"))))
	{
		hv_Fonts.Clear();
		hv_Fonts[0] = "Consolas";
		hv_Fonts[1] = "Menlo";
		hv_Fonts[2] = "Courier";
		hv_Fonts[3] = "Courier 10 Pitch";
		hv_Fonts[4] = "FreeMono";
		hv_Fonts[5] = "Liberation Mono";
	}
	else if (0 != (int(hv_Font == HTuple("sans"))))
	{
		hv_Fonts.Clear();
		hv_Fonts[0] = "Luxi Sans";
		hv_Fonts[1] = "DejaVu Sans";
		hv_Fonts[2] = "FreeSans";
		hv_Fonts[3] = "Arial";
		hv_Fonts[4] = "Liberation Sans";
	}
	else if (0 != (int(hv_Font == HTuple("serif"))))
	{
		hv_Fonts.Clear();
		hv_Fonts[0] = "Times New Roman";
		hv_Fonts[1] = "Luxi Serif";
		hv_Fonts[2] = "DejaVu Serif";
		hv_Fonts[3] = "FreeSerif";
		hv_Fonts[4] = "Utopia";
		hv_Fonts[5] = "Liberation Serif";
	}
	else
	{
		hv_Fonts = hv_Font;
	}
	hv_Style = "";
	if (0 != (int(hv_Bold == HTuple("true"))))
	{
		hv_Style += HTuple("Bold");
	}
	else if (0 != (int(hv_Bold != HTuple("false"))))
	{
		hv_Exception = "Wrong value of control parameter Bold";
		throw HException(hv_Exception);
	}
	if (0 != (int(hv_Slant == HTuple("true"))))
	{
		hv_Style += HTuple("Italic");
	}
	else if (0 != (int(hv_Slant != HTuple("false"))))
	{
		hv_Exception = "Wrong value of control parameter Slant";
		throw HException(hv_Exception);
	}
	if (0 != (int(hv_Style == HTuple(""))))
	{
		hv_Style = "Normal";
	}
	QueryFont(hv_WindowHandle, &hv_AvailableFonts);
	hv_Font = "";
	{
		HTuple end_val48 = (hv_Fonts.TupleLength()) - 1;
		HTuple step_val48 = 1;
		for (hv_Fdx = 0; hv_Fdx.Continue(end_val48, step_val48); hv_Fdx += step_val48)
		{
			hv_Indices = hv_AvailableFonts.TupleFind(HTuple(hv_Fonts[hv_Fdx]));
			if (0 != (int((hv_Indices.TupleLength())>0)))
			{
				if (0 != (int(HTuple(hv_Indices[0]) >= 0)))
				{
					hv_Font = HTuple(hv_Fonts[hv_Fdx]);
					break;
				}
			}
		}
	}
	if (0 != (int(hv_Font == HTuple(""))))
	{
		throw HException("Wrong value of control parameter Font");
	}
	hv_Font = (((hv_Font + "-") + hv_Style) + "-") + hv_Size;
	HalconCpp::SetFont(hv_WindowHandle, hv_Font);
	return;
}


// Chapter: XLD / Creation
// Short Description: Creates an arrow shaped XLD contour.
void CDlgImageWindow::gen_arrow_contour_xld (HObject *Arrow, HTuple Row1, HTuple Column1, 
	HTuple Row2, HTuple Column2, HTuple HeadLength, HTuple HeadWidth)
{
	using namespace HalconCpp;

	// Local iconic variables 
	HObject  TempArrow;


	// Local control variables 
	HTuple  Length, ZeroLengthIndices, DR, DC, HalfHeadWidth;
	HTuple  RowP1, ColP1, RowP2, ColP2, Index;

	//This procedure generates arrow shaped XLD contours,
	//pointing from (Row1, Column1) to (Row2, Column2).
	//If starting and end point are identical, a contour consisting
	//of a single point is returned.
	//
	//input parameteres:
	//Row1, Column1: Coordinates of the arrows' starting points
	//Row2, Column2: Coordinates of the arrows' end points
	//HeadLength, HeadWidth: Size of the arrow heads in pixels
	//
	//output parameter:
	//Arrow: The resulting XLD contour
	//
	//The input tuples Row1, Column1, Row2, and Column2 have to be of
	//the same length.
	//HeadLength and HeadWidth either have to be of the same length as
	//Row1, Column1, Row2, and Column2 or have to be a single element.
	//If one of the above restrictions is violated, an error will occur.
	//
	//
	//Init
	GenEmptyObj(&(*Arrow));
	//
	//Calculate the arrow length
	DistancePp(Row1, Column1, Row2, Column2, &Length);
	//
	//Mark arrows with identical start and end point
	//(set Length to -1 to avoid division-by-zero exception)
	ZeroLengthIndices = Length.TupleFind(0);
	if (0 != (ZeroLengthIndices!=-1))
	{
		Length[ZeroLengthIndices] = -1;
	}
	//
	//Calculate auxiliary variables.
	DR = (1.0*(Row2-Row1))/Length;
	DC = (1.0*(Column2-Column1))/Length;
	HalfHeadWidth = HeadWidth/2.0;
	//
	//Calculate end points of the arrow head.
	RowP1 = (Row1+((Length-HeadLength)*DR))+(HalfHeadWidth*DC);
	ColP1 = (Column1+((Length-HeadLength)*DC))-(HalfHeadWidth*DR);
	RowP2 = (Row1+((Length-HeadLength)*DR))-(HalfHeadWidth*DC);
	ColP2 = (Column1+((Length-HeadLength)*DC))+(HalfHeadWidth*DR);
	//
	//Finally create output XLD contour for each input point pair
	for (Index=0; Index<=(Length.Length())-1; Index+=1)
	{
		if (0 != (HTuple(Length[Index])==-1))
		{
			//Create_ single points for arrows with identical start and end point
			GenContourPolygonXld(&TempArrow, HTuple(Row1[Index]), HTuple(Column1[Index]));
		}
		else
		{
			//Create arrow contour
			GenContourPolygonXld(&TempArrow, ((((HTuple(Row1[Index]).TupleConcat(HTuple(Row2[Index]))).TupleConcat(HTuple(RowP1[Index]))).TupleConcat(HTuple(Row2[Index]))).TupleConcat(HTuple(RowP2[Index]))).TupleConcat(HTuple(Row2[Index])),
				((((HTuple(Column1[Index]).TupleConcat(HTuple(Column2[Index]))).TupleConcat(HTuple(ColP1[Index]))).TupleConcat(HTuple(Column2[Index]))).TupleConcat(HTuple(ColP2[Index]))).TupleConcat(HTuple(Column2[Index])));
		}
		ConcatObj((*Arrow), TempArrow, &(*Arrow));
	}
	return;
}

void CDlgImageWindow::GetWindowImage(HObject &image)
{
	if (m_hWindow.Length() == 0)
	{
		return;
	}

	try
	{
		if (!m_image.IsInitialized() || m_image.CountObj() == 0)
		{
			return;
		}

		image= m_image ;

	}
	catch (...)
	{

		return ;
	}

}



BOOL CDlgImageWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{


	// TODO: 视窗缩放
	CRect	WndRect;
	Hlong	ImagePtX,ImagePtY;
	Hlong	Row0_1,Col0_1,Row1_1,Col1_1;
	Hlong   Width,Height;
	double	Scale=0.1;
	double  MaxScale=80;

	HObject l_image;
	HTuple l_hWindow;
	//HObject l_object;
	//根据鼠标在视窗位置，选择对应视窗句柄和图像进行操作
	GetWindowRect(&WndRect);

	l_image=m_image;
	l_hWindow=m_hWindow;
	//l_object = m_object;

	if (!l_image.IsInitialized() ||l_image.CountObj() == 0)
	{

		return CDialog::OnMouseWheel(nFlags, zDelta, pt);
	}

	if (l_hWindow.Length()==0)
	{

		return CDialog::OnMouseWheel(nFlags, zDelta, pt);
	}


	//判断鼠标位置是否在picture控件区域
	if (!WndRect.PtInRect(pt))                      
	{

		return CDialog::OnMouseWheel(nFlags, zDelta, pt);
	}

	//缩放中心选择当前鼠标点
	double ScalePt0_x,ScalePt0_y;
	ScalePt0_x=pt.x;
	ScalePt0_y=pt.y;

	HTuple hWidth, hHeight;
	GetImageSize(l_image,&hWidth,&hHeight);
	Width = hWidth[0].I();
	Height= hHeight[0].I();

	//获取之前保存的SetPart变量
	HTuple hImageCol0,hImageRow0,hImageCol1,hImageRow1;
	GetPart(l_hWindow,	&hImageRow0,&hImageCol0,&hImageRow1,&hImageCol1);

	Hlong l_ImageCol0, l_ImageRow0, l_ImageCol1, l_ImageRow1;
	l_ImageCol0=hImageCol0[0].D();
	l_ImageRow0 = hImageRow0[0].D();
	l_ImageCol1 = hImageCol1[0].D();
	l_ImageRow1 = hImageRow1[0].D();

	//向上滑动滚轮，图像缩小。以当前鼠标的坐标为支点进行缩小或放大
	if (zDelta>0)
	{   

		//把当前鼠标的坐标由窗口坐标转化为图像坐标
		ImagePtX=l_ImageCol0+(ScalePt0_x-WndRect.left)/(WndRect.Width()-1.0)*(l_ImageCol1-l_ImageCol0);
		ImagePtY=l_ImageRow0+(ScalePt0_y-WndRect.top)/(WndRect.Height()-1.0)*(l_ImageRow1-l_ImageRow0);
		//重新计算缩小后的图像区域
		Row0_1=ImagePtY-1/(1-Scale)*(ImagePtY-l_ImageRow0);
		Row1_1=ImagePtY-1/(1-Scale)*(ImagePtY-l_ImageRow1);
		Col0_1=ImagePtX-1/(1-Scale)*(ImagePtX-l_ImageCol0);
		Col1_1=ImagePtX-1/(1-Scale)*(ImagePtX-l_ImageCol1);
		//限定缩小范围
		if ( fabs((double)(Col1_1)-(double)(Col0_1))/Width<=4)
		{
			//设置在图形窗口中显示局部图像
			l_ImageRow0=Row0_1;
			l_ImageCol0=Col0_1;
			l_ImageRow1=Row1_1;
			l_ImageCol1=Col1_1;
			HalconCpp::SetPart(l_hWindow,l_ImageRow0,l_ImageCol0,l_ImageRow1,l_ImageCol1);

			//中心偏移修正
			Hlong x1,y1;
			x1=l_ImageCol0+(ScalePt0_x-WndRect.left)/(WndRect.Width()-1.0)*(l_ImageCol1-l_ImageCol0);
			y1=l_ImageRow0+(ScalePt0_y-WndRect.top)/(WndRect.Height()-1.0)*(l_ImageRow1-l_ImageRow0);
			int dx,dy;
			dx=x1-ImagePtX;
			dy=y1-ImagePtY;

			l_ImageRow0=l_ImageRow0-dy;
			l_ImageCol0=l_ImageCol0-dx;
			l_ImageRow1=l_ImageRow1-dy;
			l_ImageCol1=l_ImageCol1-dx;
			HalconCpp::SetPart(l_hWindow,l_ImageRow0,l_ImageCol0,l_ImageRow1,l_ImageCol1);

			m_fScale=Width/fabs((double)Col1_1-(double)Col0_1);
		}


	}

	if (zDelta<0)
	{ 
		//把当前鼠标的坐标由窗口坐标转化为图像坐标
		ImagePtX=l_ImageCol0+(ScalePt0_x-WndRect.left)/(WndRect.Width()-1.0)*(l_ImageCol1-l_ImageCol0);
		ImagePtY=l_ImageRow0+(ScalePt0_y-WndRect.top)/(WndRect.Height()-1.0)*(l_ImageRow1-l_ImageRow0);

		//重新计算放大后的图像区域
		Row0_1=ImagePtY-1/(1+Scale)*(ImagePtY-l_ImageRow0);
		Row1_1=ImagePtY-1/(1+Scale)*(ImagePtY-l_ImageRow1);
		Col0_1=ImagePtX-1/(1+Scale)*(ImagePtX-l_ImageCol0);
		Col1_1=ImagePtX-1/(1+Scale)*(ImagePtX-l_ImageCol1);
		//限定放大范围
		if (Width/fabs((double)Col1_1-(double)Col0_1)<=MaxScale)
		{
			//设置在图形窗口中显示局部图像
			l_ImageRow0=Row0_1;
			l_ImageCol0=Col0_1;
			l_ImageRow1=Row1_1;
			l_ImageCol1=Col1_1;
			HalconCpp::SetPart(l_hWindow,l_ImageRow0,l_ImageCol0,l_ImageRow1,l_ImageCol1);

			//中心偏移修正
			Hlong x1,y1;
			x1=l_ImageCol0+(ScalePt0_x-WndRect.left)/(WndRect.Width()-1.0)*(l_ImageCol1-l_ImageCol0);
			y1=l_ImageRow0+(ScalePt0_y-WndRect.top)/(WndRect.Height()-1.0)*(l_ImageRow1-l_ImageRow0);
			int dx,dy;
			dx=x1-ImagePtX;
			dy=y1-ImagePtY;

			l_ImageRow0=l_ImageRow0-dy;
			l_ImageCol0=l_ImageCol0-dx;
			l_ImageRow1=l_ImageRow1-dy;
			l_ImageCol1=l_ImageCol1-dx;
			HalconCpp::SetPart(l_hWindow,l_ImageRow0,l_ImageCol0,l_ImageRow1,l_ImageCol1);

			//缩放倍率显示和位置
			m_fScale=Width/fabs((double)Col1_1-(double)Col0_1);
		}

	}

	HalconCpp::SetColored(m_hWindow, 12);

	HalconCpp::ClearWindow(l_hWindow);

	if (l_image.IsInitialized() && l_image.CountObj() > 0)
	{
		HalconCpp::DispObj(l_image, l_hWindow);
	}
	

	//if (l_object.IsInitialized() && l_object.CountObj() > 0)
	//{
	//	HalconCpp::DispObj(l_object, l_hWindow);
	//}
	

	DisplayCrossCenterLine();


	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CDlgImageWindow::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码


}


//显示十字线
void CDlgImageWindow::DisplayCrossCenterLine(void)
{

	HTuple l_hWindow=m_hWindow;
	HObject l_hImage=m_image;

	if (l_hWindow.Length()==0)
	{
		return;
	}

	if (!l_hImage.IsInitialized() ||l_hImage.CountObj() == 0)
	{
		return;
	}

	if (!m_bCross)
	{
		return;
	}

	Hlong l_ImageCol0,l_ImageRow0,l_ImageCol1,l_ImageRow1;


	HTuple hWidth,hHeight;
	GetImageSize(l_hImage,&hWidth,&hHeight);
	Hlong iWidth, iHeight;
	iWidth = hWidth[0].I();
	iHeight = hHeight[0].I();

	//Hlong Col0,Row0,Col1,Row1;
	//GetPart(l_hWindow,	&Row0,&Col0,&Row1,&Col1);

	//以图像中心显示十字线
	//l_ImageRow0=(Row0<0 )? 0:Row0;
	//l_ImageCol0=(Col0<0 )? 0:Col0;
	//l_ImageRow1=(Row1>iHeight)?  iHeight:Row1;
	//l_ImageCol1=(Col1>iWidth)?  iWidth:Col1;


	//以相机视野中心显示十字线
	l_ImageRow0=0;
	l_ImageCol0=0;
	l_ImageRow1=iHeight;
	l_ImageCol1=iWidth;


	//显示十字
	SetColor(l_hWindow,"red");
	SetLineWidth(l_hWindow,2);
	//SetLineStyle(l_hWindow, (HTuple(5).Append(3)));
	DispLine(l_hWindow,(l_ImageRow0+l_ImageRow1)/2,0,(l_ImageRow0+l_ImageRow1)/2,l_ImageCol1);
	DispLine(l_hWindow,0,(l_ImageCol0+l_ImageCol1)/2,l_ImageRow1,(l_ImageCol0+l_ImageCol1)/2);
	//SetColor(l_hWindow,g_hColor);
	SetLineWidth(l_hWindow,2);
	SetLineStyle(l_hWindow, HTuple());

}


BOOL CDlgImageWindow::PreTranslateMessage(MSG* pMsg)
{


	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg -> message == WM_KEYDOWN)
	{
		if(pMsg -> wParam == VK_ESCAPE)
			return TRUE;
		if(pMsg -> wParam == VK_RETURN)
			return TRUE;
	}


	CPoint pt;
	GetCursorPos(&pt);

	HTuple l_hWindow;
	HObject l_image;
	CRect	WndRect;


	//判断图像窗口选择图像
	GetDlgItem(IDS_VIEW)->GetWindowRect(&WndRect);
	if (!WndRect.PtInRect(pt))
	{
		return CDialogEx::PreTranslateMessage(pMsg);
	}



	//右键弹出菜单
	if((pMsg -> message == WM_RBUTTONUP)&&(m_bDisableMenu==0))     
	{

		m_iCountRButtonUp=m_iCountRButtonUp+1;

		if (m_iCountRButtonUp==2)
		{
			m_iCountRButtonUp=0;
		}

		if ((m_iCountRButtonUp==1))
		{
			//CMenu   menu;  
			//VERIFY(menu.LoadMenu(IDM_PIC));  
			//CMenu*   pPopup   =   menu.GetSubMenu(0);  
			//ASSERT(pPopup   !=   NULL);  

			//l_hWindow=m_hWindow;
			//m_bDynamic?pPopup->ModifyMenu(IDB_DYNAMIC,MF_POPUP,IDB_DYNAMIC,"关闭动态")
			//	:pPopup->ModifyMenu(IDB_DYNAMIC,MF_POPUP,IDB_DYNAMIC,"开启动态");


			//m_bCross?pPopup->ModifyMenu(IDB_CROSS,MF_POPUP,IDB_CROSS,"关闭十字线")
			//	:pPopup->ModifyMenu(IDB_CROSS,MF_POPUP,IDB_CROSS,"显示十字线");

			//CPoint   point;  
			//GetCursorPos(&point);  
			//pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,   point.y,   this);  


		}

	}


	//鼠标移动显示坐标，倍率，像素
	if ((pMsg->message == WM_MOUSEMOVE))
	{
		//保持窗口焦点不丢失
		GetDlgItem(IDS_VIEW)->SetFocus();

		//选择对应的窗口变量
		l_hWindow = m_hWindow;
		l_image = m_image;



		HTuple hImageCol0, hImageRow0, hImageCol1, hImageRow1;
		GetPart(l_hWindow, &hImageRow0, &hImageCol0, &hImageRow1, &hImageCol1);

		Hlong l_ImageCol0, l_ImageRow0, l_ImageCol1, l_ImageRow1;
		l_ImageCol0 = hImageCol0[0].D();
		l_ImageRow0 = hImageRow0[0].D();
		l_ImageCol1 = hImageCol1[0].D();
		l_ImageRow1 = hImageRow1[0].D();

		HTuple x, y;
		if (l_image.IsInitialized() && l_image.CountObj() > 0)
		{
			//鼠标位置
			x = 1.0 * (pt.x - WndRect.left) * (1.0 * (l_ImageCol1 - l_ImageCol0) / WndRect.Width()) + l_ImageCol0;
			y = 1.0 * (pt.y - WndRect.top) * (1.0 * (l_ImageRow1 - l_ImageRow0) / WndRect.Height()) + l_ImageRow0;
			x = x.TupleRound();
			y = y.TupleRound();

			//获取灰度值
			HTuple GrayVal;

			HTuple hWidth, hHeight;
			GetImageSize(l_image, &hWidth, &hHeight);
			Hlong iWidth, iHeight;
			iWidth = hWidth[0].I();
			iHeight = hHeight[0].I();


			if ((0 <= x[0].I()) && (x[0].I() < iWidth) && (y[0].I() >= 0) && (y[0].I() < iHeight))
			{

				//显示
				CString str;
				try
				{
					if ((0 <= x[0].I()) && (x[0].I() < iWidth) && (y[0].I() >= 0) && (y[0].I() < iHeight))
					{

						HTuple hChannels;
						CountChannels(l_image, &hChannels);
						if (hChannels.Length() > 0 && hChannels[0].I() == 3)
						{
							HObject hImageRed, hImageGreen, hImageBlue;
							Decompose3(l_image, &hImageRed, &hImageGreen, &hImageBlue);


							HTuple gray1, gray2, gray3;
							GetGrayval(hImageRed, y, x, &gray1);
							GetGrayval(hImageGreen, y, x, &gray2);
							GetGrayval(hImageBlue, y, x, &gray3);

							try
							{
								str.Format(_T("x=%d,y=%d,rgb=%d,%d,%d"), x[0].I(), y[0].I(), gray1[0].I(), gray2[0].I(), gray3[0].I());
							}
							catch (...)
							{
								str.Format(_T("x=%d,y=%d,XYZ=%d,%d,%d"), x[0].I(), y[0].I(),(int)  gray1[0].D(), (int)gray2[0].D(), (int)gray3[0].D());
							}

							

						}
						else if (hChannels.Length() > 0 && hChannels[0].I() == 2)
						{
							HObject hImage1, hImage2;
							Decompose2(l_image, &hImage1, &hImage2);


							HTuple gray1, gray2;
							GetGrayval(hImage1, y, x, &gray1);
							GetGrayval(hImage2, y, x, &gray2);
							str.Format(_T("x=%d,y=%d,Gray==%d,Z=%.1f"), x[0].I(), y[0].I(), gray1[0].I(), gray2[0].D());

						}
						else if (hChannels.Length() > 0 && hChannels[0].I() == 1)
						{
							HTuple gray;
							GetGrayval(l_image, y, x, &gray);

							str.Format(_T("x=%d,y=%d,gray=%.3f,scale=%.2f"), x[0].I(), y[0].I(), gray[0].D(), m_fScale);
						}
						else if (hChannels.Length() > 0 && hChannels[0].I() == 4) //临时使用  定义成gray-xyz
						{


							HObject hImage1, hImage2, hImage3, hImage4;
							Decompose4(l_image, &hImage1, &hImage2, &hImage3, &hImage4);


							HTuple gray1, gray2, gray3, gray4;
							GetGrayval(hImage1, y, x, &gray1);
							GetGrayval(hImage2, y, x, &gray2);
							GetGrayval(hImage3, y, x, &gray3);
							GetGrayval(hImage4, y, x, &gray4);


							str.Format(_T("x=%d,y=%d,gray=%d,XYZ=%.1f, %.1f, %.1f"), x[0].I(), y[0].I(), gray1[0].I(), gray2[0].D(), gray3[0].D(), gray4[0].D());

						}





					}



				}
				catch (...)
				{
					m_iGrayValue = 0;
				}




				// 画图  
				CDC* pDC = m_picDraw.GetDC();
				CRect rectPicture;
				m_picDraw.GetClientRect(&rectPicture);
				//背景
				CBrush newBrush;
				CBrush* pOldBrush;
				CRect rectBK = CRect(rectPicture.left, rectPicture.bottom - 22, rectPicture.left + 320, rectPicture.bottom);
				newBrush.CreateSolidBrush(RGB(240, 240, 240));
				pOldBrush = pDC->SelectObject(&newBrush);
				pDC->Rectangle(rectBK);
				pDC->SelectObject(pOldBrush);
				newBrush.DeleteObject();

				//文字
				CFont  newFont;
				CFont* pOldFont;
				newFont.CreatePointFont(90, _T("隶书"));
				pOldFont = pDC->SelectObject(&newFont);
				pDC->SetBkMode(TRANSPARENT);
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->TextOut(rectPicture.left + 5, rectPicture.bottom - 15, str);
				pDC->SelectObject(pOldFont);
				newFont.DeleteObject();

				ReleaseDC(pDC);
			}



		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}







//void CDlgImageWindow::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//	if (m_bDisableMenu)
//	{
//		m_bDisableMenu=0;
//	}
//
//	CDialogEx::OnTimer(nIDEvent);
//}


