#pragma once
class CLogX
{
public:
	CLogX(void);
	virtual ~CLogX(void);

	void Log(CString szText);
	CString GetModuleFolder();

	CRITICAL_SECTION m_csLogX;
};

