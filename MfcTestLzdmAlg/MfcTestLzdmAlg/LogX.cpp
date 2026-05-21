
#include "pch.h"
//#include "StdAfx.h"
#include "LogX.h"

#include <afxmt.h>            // for CCriticalSection
#include <direct.h>           // for _mkdir
#include <io.h>               // for _access
#include <afx.h>
#include <time.h>

CLogX::CLogX(void)
{
	InitializeCriticalSection(&m_csLogX);
}


CLogX::~CLogX(void)
{
	DeleteCriticalSection(&m_csLogX);
}


//void CLogX::Log(CString szText)
//{
//	EnterCriticalSection(&m_csLogX);
//
//	// 格式化时间
//	CTime m_tDateTime;   
//	m_tDateTime = m_tDateTime.GetCurrentTime();   
//	CString szTime =m_tDateTime.Format("%Y%m%d_%H_%M_%S");   // 2009-03-06 09:07:44
//
//	CStdioFile log;
//	char buf[512];
//	memset(buf,0,512);
//	char fpath[MAX_PATH];
//	memset(fpath,0,MAX_PATH);
//	sprintf(fpath,"%s\\Log\\log.txt",GetModuleFolder());
//
//	long nLength = 0;
//	if (log.Open(fpath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
//	{
//		log.SeekToEnd();
//		sprintf(buf,"%s   \t%s\r\n",szTime,szText);
//		//		int nRet = m_list.AddString(buf);
//		//		m_list.SetTopIndex(nRet);
//		log.WriteString(buf);
//		nLength = log.GetLength();
//		log.Close();
//	}
//	szTime = m_tDateTime.Format("%Y%m%d_%H_%M_%S");  // 2009-03-06 09:07:44
//	if(nLength>200*1024)
//	{
//		CString str;
//		str.Format(_T("%s\\Log\\log_%s.txt"),GetModuleFolder(),szTime);
//		//rename(fpath,"log_"+szTime+".txt");
//		rename(fpath,str);
//	}
//
//	LeaveCriticalSection(&m_csLogX);
//}
//
//
//CString CLogX:: GetModuleFolder()
//{
//	// 得到应用程序的目录
//	char   szFileName[256];   
//	GetModuleFileName(NULL,szFileName,256);   
//	char   szDrive[256]={0};   
//	char   szDir[256]={0};   
//	char   szPath[256]={0};   
//	_tsplitpath(szFileName,szDrive,szDir,NULL,NULL); //分开
//	sprintf(szPath,"%s%s",szDrive,szDir); 
//
//	return (CString(szPath));
//}

void CLogX::Log(CString szText)
{
    EnterCriticalSection(&m_csLogX);

    // 获取当前时间
    CTime m_tDateTime = CTime::GetCurrentTime();
    CString szTime = m_tDateTime.Format(_T("%Y%m%d_%H_%M_%S"));

    // 构造日志路径
    CString strModulePath = GetModuleFolder();
    CString logFolderPath = strModulePath + _T("\\Log");
    CString logFilePath = logFolderPath + _T("\\log.txt");

    // 创建 Log 目录（如果不存在）
    if (_taccess(logFolderPath, 0) != 0)
    {
        _tmkdir(logFolderPath);  // 注意：_tmkdir 对应 TCHAR 类型
    }

    CStdioFile log;
    CString line;
    line.Format(_T("%s\t%s\n"), szTime, szText);

    try
    {
        if (log.Open(logFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        {
            log.SeekToEnd();
            log.WriteString(line);
            long nLength = log.GetLength();

            log.Close();

            // 如果日志文件大于 200KB，重命名备份
            if (nLength > 200 * 1024)
            {
                CString newFileName;
                newFileName.Format(_T("%s\\log_%s.txt"), logFolderPath, szTime);

                // 删除旧的备份文件（可选）
                CFile::Remove(newFileName);

                // 重命名原文件
                CFile::Rename(logFilePath, newFileName);
            }
        }
    }
    catch (CFileException* e)
    {
        // 可以添加错误处理
        e->Delete();
    }

    LeaveCriticalSection(&m_csLogX);
}

CString CLogX::GetModuleFolder()
{
    TCHAR szFileName[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szFileName, MAX_PATH);

    TCHAR szDrive[_MAX_DRIVE] = { 0 };
    TCHAR szDir[_MAX_DIR] = { 0 };
    TCHAR szPath[_MAX_PATH] = { 0 };

    _tsplitpath_s(szFileName, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
    _stprintf_s(szPath, _T("%s%s"), szDrive, szDir);

    return CString(szPath);
}