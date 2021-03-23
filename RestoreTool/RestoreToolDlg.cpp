#include "stdafx.h"
#include "RestoreTool.h"
#include "RestoreToolDlg.h"
#include "file.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);   
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()






CRestoreToolDlg::CRestoreToolDlg(CWnd* pParent /*=NULL*/)
: CDialog(CRestoreToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRestoreToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRestoreToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CRestoreToolDlg::OnBnClickedOpenFile)
	ON_BN_CLICKED(IDC_BUTTON2, &CRestoreToolDlg::OnBnClickedSaveFile)
	ON_BN_CLICKED(IDC_BUTTON3, &CRestoreToolDlg::OnBnClickedRestore)
END_MESSAGE_MAP()


// CRestoreToolDlg

BOOL CRestoreToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	// TODO: 

	return TRUE; 
}

void CRestoreToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}



void CRestoreToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


//
HCURSOR CRestoreToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#define ENC_BUFF_KEY_BIT 0x77
void File_DecryptBuffer(PVOID buffer,ULONG Length)
{
	PCHAR buf;
	DWORD i;

	buf = (PCHAR)buffer;

	for(i=0;i<Length;i++)
	{
		buf[i] ^= ENC_BUFF_KEY_BIT;
	}

}


CString FilePathNameOpen;
CString FilePathNameSave;

CString  GetFileExtName(CString csFileFullName)  
{  
	int nPos = csFileFullName.ReverseFind('.');  
	CString  csFileExtName = csFileFullName.Right(csFileFullName.GetLength() - nPos);   
	return csFileExtName;  
} 

void CRestoreToolDlg::OnBnClickedOpenFile()
{
	CFileDialog dlgOpen(TRUE, 
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("All Files (*.*)|*.*||"),
		NULL);
	if(dlgOpen.DoModal()==IDOK)
	{
		FilePathNameOpen=dlgOpen.GetPathName();
	}else
		return;

	GetDlgItem(IDC_STATIC)->SetWindowText(FilePathNameOpen);
}

void CRestoreToolDlg::OnBnClickedSaveFile()
{
	CFileDialog dlgSave(FALSE,
		NULL, 
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("All Files (*.*)|*.*||"),
		NULL);
	if(dlgSave.DoModal()==IDOK)
	{
		FilePathNameSave=dlgSave.GetPathName();
	}else	
		return;

	CString FilePathNameOpenExt = GetFileExtName(FilePathNameOpen);
	FilePathNameSave = FilePathNameSave + FilePathNameOpenExt;
	GetDlgItem(IDC_STATIC1)->SetWindowText(FilePathNameSave);
}

#define FILE_FLAG_LENGTH  sizeof(FILE_FLAG)
void CRestoreToolDlg::OnBnClickedRestore()
{
	if (wcslen(FilePathNameOpen) == 0)
	{
		MessageBox(L"Please set path of OpenFile!");
	}else if (wcslen(FilePathNameSave) == 0)
	{
		MessageBox(L"Please set path of SaveFile!");
	}else
	{	
		HANDLE hFile = CreateFile(FilePathNameOpen,
			GENERIC_READ,
			NULL,
			NULL,
			OPEN_EXISTING,
			NULL,
			NULL
			);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(L"Restore is failed !");
			return;
		}
		int FileCount = GetFileSize(hFile,NULL);
		FileCount -= FILE_FLAG_LENGTH;
		char* FileBuffer = new char[FileCount];
		memset(FileBuffer,0,FileCount);
		DWORD retdata = 0;
		if (!ReadFile(hFile,FileBuffer,FileCount,&retdata,NULL))
		{
			MessageBox(L"Restore is failed !");
			return;
		}
		CloseHandle(hFile);
		File_DecryptBuffer(FileBuffer,FileCount);
		HANDLE hFileBuild = CreateFile(FilePathNameSave,  
			GENERIC_WRITE,            
			0,                        
			NULL,                    
			CREATE_ALWAYS,           
			FILE_ATTRIBUTE_NORMAL,          
			NULL);                   
		if (hFileBuild == INVALID_HANDLE_VALUE)  
		{  
			MessageBox(L"Restore is failed !");
			return;
		}
		DWORD dwWritenSize = 0;  
		BOOL bRet = WriteFile(hFileBuild, FileBuffer, FileCount, &dwWritenSize, NULL);  
		if (bRet)  
		{  
			OutputDebugString(TEXT("WriteFile 写文件成功\r\n"));  
		}  
		
		FlushFileBuffers(hFile);
		CloseHandle(hFileBuild);
		MessageBox(L"Restore is ok !");
	}	
}