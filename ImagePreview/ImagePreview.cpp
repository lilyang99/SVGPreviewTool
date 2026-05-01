// ImagePreview.cpp: Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "ImagePreview.h"
#include "MainFrm.h"

#include <afxadv.h> // For CRecentFileList

#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CImagePreviewApp

BEGIN_MESSAGE_MAP(CImagePreviewApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CImagePreviewApp::OnAppAbout)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CImagePreviewApp::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)

    ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, &CImagePreviewApp::OnOpenRecentFile)
END_MESSAGE_MAP()


// CImagePreviewApp construction

CImagePreviewApp::CImagePreviewApp() noexcept
{

    // Support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
    // If the application is built using Common Language Runtime support (/clr):
    //     1) This additional setting is required for Restart Manager support to work properly.
    //     2) In your project, you must add a reference to System.Windows.Forms in the order they are built.
    System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

    // TODO: Replace the application ID string below with a unique ID string; recommended format:
    // CompanyName.ProductName.SubProduct.VersionInformation
    SetAppID(_T("ImagePreview.AppID.NoVersion"));

    // TODO: Add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CImagePreviewApp object

CImagePreviewApp theApp;


// CImagePreviewApp initialization

BOOL CImagePreviewApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Initialize MRU list
    LoadStdProfileSettings(4);

    // Create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
    {
        delete pMainFrame;
        return FALSE;
    }
    m_pMainWnd = pMainFrame;

    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();

    return TRUE;
}

int CImagePreviewApp::ExitInstance()
{
    //TODO: Handle additional resources you may have added
    AfxOleTerm(FALSE);

    return CWinApp::ExitInstance();
}

// CImagePreviewApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() noexcept;

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// Application command to run the dialog
void CImagePreviewApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CImagePreviewApp::OnFileOpen()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("All Supported Files (*.svg;*.bmp;*.jpg;*.jpeg;*.png;*.ico)|*.svg;*.bmp;*.jpg;*.jpeg;*.png;*.ico|")
        _T("SVG Files (*.svg)|*.svg|")
        _T("Image Files (*.bmp;*.jpg;*.jpeg;*.png)|*.bmp;*.jpg;*.jpeg;*.png|")
        _T("Icon Files (*.ico)|*.ico|")
        _T("All Files (*.*)|*.*||"));
    if (dlg.DoModal() == IDOK)
    {
        CString path = dlg.GetPathName();
        CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(m_pMainWnd);
        if (pMainFrame)
        {
            pMainFrame->OpenImageFile(path);
        }
    }
}

BOOL CImagePreviewApp::OnOpenRecentFile(UINT nID)
{
    if (m_pRecentFileList != NULL)
    {
        int nIndex = nID - ID_FILE_MRU_FILE1;
        ASSERT(nIndex < m_pRecentFileList->GetSize());

        CString strName = (*m_pRecentFileList)[nIndex];
        if (!strName.IsEmpty())
        {
            CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(m_pMainWnd);
            if (pMainFrame)
            {
                pMainFrame->OpenImageFile(strName);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// CImagePreviewApp









