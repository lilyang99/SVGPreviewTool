// MainFrm.cpp: Implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "ImagePreview.h"

#include "MainFrm.h"
#include "ImageChildFrm.h"
#include "SvgChildFrm.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    ON_WM_CREATE()
    ON_WM_DROPFILES()
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
{
}

CMainFrame::~CMainFrame()
{
    // Destroy all tracked child frames that haven't been closed yet
    for (auto* pFrame : m_childFrames)
    {
        if (pFrame && ::IsWindow(pFrame->GetSafeHwnd()))
        {
            pFrame->DestroyWindow();
        }
    }
    m_childFrames.clear();
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}
#endif //_DEBUG

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    DragAcceptFiles(TRUE);

    return 0;
}

void CMainFrame::OpenImageFile(CString path)
{
    // Add to Recent File List
    if (AfxGetApp())
    {
        AfxGetApp()->AddToRecentFileList(path);
    }

    // Check if it's an SVG file
    CString strExt = path.Mid(path.ReverseFind('.') + 1);
    bool bIsSvg = (strExt.CompareNoCase(_T("svg")) == 0);

    CChildFrame* pFrame = bIsSvg
        ? static_cast<CChildFrame*>(new CSvgChildFrame())
        : static_cast<CChildFrame*>(new CImageChildFrame());

    if (!pFrame)
        return;

    if (!pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW, NULL, NULL))
    {
        delete pFrame;
        return;
    }

    // Track the frame — MFC will notify us via OnChildFrameClosed when it's destroyed
    m_childFrames.push_back(pFrame);

    pFrame->LoadFile(path);
    pFrame->InitialUpdateFrame(NULL, TRUE);

    // Set window title and refresh status bar AFTER InitialUpdateFrame,
    // otherwise MFC may clear/reset them during frame activation
    CString strFileName = path.Mid(path.ReverseFind('\\') + 1);
    pFrame->SetWindowText(strFileName);
}

void CMainFrame::OnChildFrameClosed(CChildFrame* pFrame)
{
    auto it = std::find(m_childFrames.begin(), m_childFrames.end(), pFrame);
    if (it != m_childFrames.end())
    {
        m_childFrames.erase(it);
    }
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    UINT nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    for (UINT i = 0; i < nFiles; i++)
    {
        TCHAR szFile[MAX_PATH];
        DragQueryFile(hDropInfo, i, szFile, MAX_PATH);
        OpenImageFile(szFile);
    }
    DragFinish(hDropInfo);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIFrameWnd::PreCreateWindow(cs))
        return FALSE;

    return TRUE;
}
