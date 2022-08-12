// Copyright � 2008-2018 Alex Kukhtin. All rights reserved.

#pragma once

#undef AFX_DATA
#define AFX_DATA AFX_BASE_DATA

class CModuleVersion : public VS_FIXEDFILEINFO 
{
protected:
   BYTE* m_pVersionInfo;   // all version info

   struct TRANSLATION 
	 {
      WORD langID;         // language ID
      WORD charset;        // character set (code page)
   } m_translation;

public:
  CModuleVersion()
		: m_pVersionInfo(NULL) 
  {
	  m_translation.langID = 0;
	  m_translation.charset = 0;
  }
  virtual ~CModuleVersion();

  BOOL    GetFileVersionInfo(LPCTSTR modulename);
  CString GetValue(LPCTSTR lpKeyName);
	static long GetCurrentAppVersion();
	static CString GetCurrentFullAppVersion();
	static CString GetModuleVersionString(HINSTANCE hInstance);
};

#undef AFX_DATA
#define AFX_DATA
