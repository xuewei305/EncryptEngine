

#include "stdafx.h"
#include "RestoreTool.h"
#include "RestoreToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRestoreToolApp

BEGIN_MESSAGE_MAP(CRestoreToolApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CRestoreToolApp

CRestoreToolApp::CRestoreToolApp()
{
	// TODO:
}




CRestoreToolApp theApp;


// CRestoreToolApp 

BOOL CRestoreToolApp::InitInstance()
{
	
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	
	SetRegistryKey(_T("Application"));

	CRestoreToolDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 
	}

	return FALSE;
}
