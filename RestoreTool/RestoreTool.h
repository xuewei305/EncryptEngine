// RestoreTool.h : 
//

#pragma once

#ifndef __AFXWIN_H__
	#error "stdafx.h PCH file"
#endif

#include "resource.h"		// 


// CRestoreToolApp:
// 
//

class CRestoreToolApp : public CWinApp
{
public:
	CRestoreToolApp();

// overwrite
	public:
	virtual BOOL InitInstance();



	DECLARE_MESSAGE_MAP()
};

extern CRestoreToolApp theApp;