// DlgInfoList.cpp : 实现文件
//

#include "pch.h"
#include "MfcTestQhxcAlg.h"
#include "DlgInfoList.h"
#include "afxdialogex.h"


// CDlgInfoList 对话框

IMPLEMENT_DYNAMIC(CDlgInfoList, CDialogEx)

CDlgInfoList::CDlgInfoList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgInfoList::IDD, pParent)
{
	m_iCoutMax=200;

	InitializeCriticalSection(&m_csListInfo);
}

CDlgInfoList::~CDlgInfoList()
{
	DeleteCriticalSection(&m_csListInfo);
}

void CDlgInfoList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInfoList, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDB_LIST_CLEAR, &CDlgInfoList::OnBnClickedListClear)
	ON_LBN_SELCHANGE(IDC_LIST, &CDlgInfoList::OnLbnSelchangeList)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDlgInfoList 消息处理程序


HBRUSH CDlgInfoList::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	pDC->SetBkMode(TRANSPARENT); //设置字体背景为透明
	hbr = (HBRUSH)::GetStockObject(WHITE_BRUSH);  // 设置背景色
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CDlgInfoList::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


BOOL CDlgInfoList::OnInitDialog()
{
	CDialogEx::OnInitDialog();




	// 滚动提示栏
	m_ListBox.SubclassDlgItem(IDC_LIST,this);
	m_ListBox.SetHorizontalExtent(2000);	


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL CDlgInfoList::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg -> message == WM_KEYDOWN)
	{
		if(pMsg -> wParam == VK_ESCAPE)
			return TRUE;
		if(pMsg -> wParam == VK_RETURN)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgInfoList::OnBnClickedListClear()
{
	m_ListBox.ResetContent();
}



void CDlgInfoList::DisplayInfo(CString str)
{
	EnterCriticalSection(&m_csListInfo);

	CListBox *myListBox=&m_ListBox;

	int	count;

	SYSTEMTIME st;
	GetLocalTime(&st);

	CString l_sCurTime;
	l_sCurTime.Format(_T("[%02d:%02d:%02d:%3d]"),st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	//l_sCurTime.Format(_T("[%02d:%02d:%02d]"),st.wHour,st.wMinute,st.wSecond);
	
	CString l_sInfo=l_sCurTime+str;
	myListBox->AddString(l_sInfo);

	count=myListBox->GetCount();
	if (count>m_iCoutMax)
	{
		myListBox->DeleteString(0);
	}
	count=myListBox->GetCount();
	myListBox->SetCurSel(count-1);

	LeaveCriticalSection(&m_csListInfo);

}


void CDlgInfoList::SetCountMax(UINT n)
{
	if (n>1)
	{
		m_iCoutMax=n;
	}

}

//////////////////////////////////////////////////////////////////////////////
//功能：字符串按指定分隔符进行分割
/////////////////////////////////////////////////////////////////////////////
void  CDlgInfoList::ParseCString(CString source, CStringArray& dest, char division)
{

	dest.RemoveAll();
	int i;
	for (i = 0; i < source.GetLength(); i ++)
	{
		if (source.GetAt(i) == division)
		{
			dest.Add(source.Left(i)); //remove left
			for (int j = 0; j < (dest.GetSize() - 1); j ++)
			{
				dest[dest.GetSize()-1] = dest[dest.GetSize()-1].Right(dest[dest.GetSize()-1].GetLength()-dest[j].GetLength()-1);  //remove right
			}
		}
	}

	//The last string
	dest.Add(source.Left(i));
	for (int j = 0; j < (dest.GetSize() - 1); j ++)
	{
		dest[dest.GetSize()-1] = dest[dest.GetSize()-1].Right(dest[dest.GetSize()-1].GetLength()-dest[j].GetLength()-1);
	}

}



void CDlgInfoList::OnLbnSelchangeList()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CDlgInfoList::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

    //布置控件

	if (cx <= 0 || cy <= 0)
		return;
	
	CRect rcDlg;
	GetClientRect(&rcDlg);

	const int BUTTON_HEIGHT = 20;

	CWnd* pList = GetDlgItem(IDC_LIST);
	CWnd* pBtn = GetDlgItem(IDB_LIST_CLEAR);

	if (pList)
	{
		pList->MoveWindow(rcDlg.left, rcDlg.top, rcDlg.Width() + 2, rcDlg.Height() - 20);
	}

	if (pBtn)
	{
		pBtn->MoveWindow(rcDlg.left, rcDlg.bottom - 20, rcDlg.Width() + 2, 20);
	}
}
