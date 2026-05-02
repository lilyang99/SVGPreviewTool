// ChildFrm.cpp: Implementation of the CChildFrame class
//

#include "pch.h"
#include "framework.h"
#include "ImagePreview.h"

#include "ChildFrm.h"
#include "MainFrm.h"
#include "Resource.h"
#include "DpiScaleDlg.h"
#include "CustomIconSizeDlg.h"

#include <afxpriv.h>

#pragma comment(lib, "Msimg32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_COMMAND(ID_ZOOM_100, &CChildFrame::OnZoom100)
    ON_COMMAND(ID_ZOOM_125, &CChildFrame::OnZoom125)
    ON_COMMAND(ID_ZOOM_150, &CChildFrame::OnZoom150)
    ON_COMMAND(ID_ZOOM_175, &CChildFrame::OnZoom175)
    ON_COMMAND(ID_ZOOM_200, &CChildFrame::OnZoom200)
    ON_COMMAND(ID_ZOOM_225, &CChildFrame::OnZoom225)
    ON_COMMAND(ID_ZOOM_250, &CChildFrame::OnZoom250)
    ON_COMMAND(ID_ZOOM_300, &CChildFrame::OnZoom300)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_100, &CChildFrame::OnUpdateZoom100)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_125, &CChildFrame::OnUpdateZoom125)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_150, &CChildFrame::OnUpdateZoom150)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_175, &CChildFrame::OnUpdateZoom175)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_200, &CChildFrame::OnUpdateZoom200)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_225, &CChildFrame::OnUpdateZoom225)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_250, &CChildFrame::OnUpdateZoom250)
    ON_UPDATE_COMMAND_UI(ID_ZOOM_300, &CChildFrame::OnUpdateZoom300)
    ON_COMMAND(ID_DPI_SCALE, &CChildFrame::OnDpiScale)
    ON_COMMAND(ID_ZOOM_CLEAR, &CChildFrame::OnZoomClear)
    ON_COMMAND_RANGE(ID_ICON_DEFAULT, ID_ICON_256, &CChildFrame::OnIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_DEFAULT, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_16, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_32, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_48, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_64, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_128, &CChildFrame::OnUpdateIconSize)
    ON_UPDATE_COMMAND_UI(ID_ICON_256, &CChildFrame::OnUpdateIconSize)
    ON_COMMAND(ID_ICON_CUSTOM, &CChildFrame::OnIconCustom)
    ON_UPDATE_COMMAND_UI(ID_ICON_CUSTOM, &CChildFrame::OnUpdateIconCustom)
    ON_UPDATE_COMMAND_UI(AFX_IDS_IDLEMESSAGE, &CChildFrame::OnUpdateIndicatorText)
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame() noexcept
{
    m_dScale = 1.0;
    m_hBitmap = NULL;
    m_nIconSize = 0;
    m_bIsIcon = false;
    m_bIsSvg = false;
}

CChildFrame::~CChildFrame()
{
    if (m_hBitmap != NULL)
    {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
}

void CChildFrame::PostNcDestroy()
{
    CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
    if (pMainFrame)
    {
        pMainFrame->OnChildFrameClosed(this);
    }
    CMDIChildWnd::PostNcDestroy();
}

static UINT indicators[] =
{
    AFX_IDS_IDLEMESSAGE,
};

int CChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
            sizeof(indicators) / sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;
    }

    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

    return 0;
}

void CChildFrame::UpdateStatusBar()
{
    CString strStatus;
    int nWidth = 0;
    int nHeight = 0;

    if (m_hBitmap != NULL)
    {
        BITMAP bm;
        ::GetObject(m_hBitmap, sizeof(BITMAP), &bm);
        nWidth = bm.bmWidth;
        nHeight = bm.bmHeight;
    }

    strStatus.Format(_T("Image Origin Size: %d x %d | Zoom: %.0f%%"), nWidth, nHeight, m_dScale * 100.0);
    m_wndStatusBar.SetPaneText(0, strStatus);
    m_wndStatusBar.UpdateData(FALSE);
}

void CChildFrame::SetScale(double dScale)
{
    m_dScale = dScale;
    UpdateStatusBar();
    Invalidate();
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWnd::PreCreateWindow(cs))
        return FALSE;

    return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
    CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
    CMDIChildWnd::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers

void CChildFrame::OnPaint()
{
    CPaintDC dc(this);

    if (m_hBitmap != NULL)
    {
        BITMAP bm;
        ::GetObject(m_hBitmap, sizeof(BITMAP), &bm);
        int nWidth = bm.bmWidth;
        int nHeight = bm.bmHeight;

        HDC hdcMem = ::CreateCompatibleDC(dc.m_hDC);
        HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem, m_hBitmap);

        int nScaledWidth = (int)(nWidth * m_dScale);
        int nScaledHeight = (int)(nHeight * m_dScale);

        if (bm.bmBitsPixel == 32)
        {
            BLENDFUNCTION bf;
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = 255;
            bf.AlphaFormat = AC_SRC_ALPHA;

            ::AlphaBlend(dc.m_hDC, 10, 10, nWidth, nHeight,
                hdcMem, 0, 0, nWidth, nHeight, bf);

            ::AlphaBlend(dc.m_hDC, 10 + nWidth + 10, 10, nScaledWidth, nScaledHeight,
                hdcMem, 0, 0, nWidth, nHeight, bf);
        }
        else
        {
            ::BitBlt(dc.m_hDC, 10, 10, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);

            int nOldMode = ::SetStretchBltMode(dc.m_hDC, HALFTONE);
            POINT ptOrg;
            ::SetBrushOrgEx(dc.m_hDC, 0, 0, &ptOrg);

            ::StretchBlt(dc.m_hDC, 10 + nWidth + 10, 10, nScaledWidth, nScaledHeight,
                hdcMem, 0, 0, nWidth, nHeight, SRCCOPY);

            ::SetBrushOrgEx(dc.m_hDC, ptOrg.x, ptOrg.y, NULL);
            ::SetStretchBltMode(dc.m_hDC, nOldMode);
        }
        ::SelectObject(hdcMem, hOldBitmap);
        ::DeleteDC(hdcMem);
    }
}

void CChildFrame::OnZoom100() { SetScale(1.0); }
void CChildFrame::OnZoom125() { SetScale(1.25); }
void CChildFrame::OnZoom150() { SetScale(1.5); }
void CChildFrame::OnZoom175() { SetScale(1.75); }
void CChildFrame::OnZoom200() { SetScale(2.0); }
void CChildFrame::OnZoom225() { SetScale(2.25); }
void CChildFrame::OnZoom250() { SetScale(2.5); }
void CChildFrame::OnZoom300() { SetScale(3.0); }

void CChildFrame::OnUpdateZoom100(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 1.0); }
void CChildFrame::OnUpdateZoom125(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 1.25); }
void CChildFrame::OnUpdateZoom150(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 1.5); }
void CChildFrame::OnUpdateZoom175(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 1.75); }
void CChildFrame::OnUpdateZoom200(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 2.0); }
void CChildFrame::OnUpdateZoom225(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 2.25); }
void CChildFrame::OnUpdateZoom250(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 2.5); }
void CChildFrame::OnUpdateZoom300(CCmdUI* pCmdUI) { pCmdUI->SetCheck(m_dScale == 3.0); }

void CChildFrame::OnDpiScale()
{
    CDpiScaleDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        if (dlg.m_nTargetDpi > 0)
        {
            SetScale((double)dlg.m_nTargetDpi / (double)dlg.m_nCurrentDpi);
        }
    }
}

void CChildFrame::OnZoomClear()
{
    SetScale(1.0);
}

void CChildFrame::OnIconSize(UINT nID)
{
    switch (nID)
    {
    case ID_ICON_DEFAULT: m_nIconSize = CSize(0, 0); break;
    case ID_ICON_16: m_nIconSize = CSize(16, 16); break;
    case ID_ICON_32: m_nIconSize = CSize(32, 32); break;
    case ID_ICON_48: m_nIconSize = CSize(48, 48); break;
    case ID_ICON_64: m_nIconSize = CSize(64, 64); break;
    case ID_ICON_128: m_nIconSize = CSize(128, 128); break;
    case ID_ICON_256: m_nIconSize = CSize(256, 256); break;
    }

    if (!m_strFilePath.IsEmpty())
    {
        this->LoadFile(m_strFilePath);
    }
}

void CChildFrame::OnUpdateIconSize(CCmdUI* pCmdUI)
{
    CSize szSize(0, 0);
    switch (pCmdUI->m_nID)
    {
    case ID_ICON_DEFAULT: szSize = CSize(0, 0); break;
    case ID_ICON_16: szSize = CSize(16, 16); break;
    case ID_ICON_32: szSize = CSize(32, 32); break;
    case ID_ICON_48: szSize = CSize(48, 48); break;
    case ID_ICON_64: szSize = CSize(64, 64); break;
    case ID_ICON_128: szSize = CSize(128, 128); break;
    case ID_ICON_256: szSize = CSize(256, 256); break;
    }

    pCmdUI->Enable(m_bIsIcon);
    pCmdUI->SetCheck(m_nIconSize == szSize);
}

void CChildFrame::OnIconCustom()
{
    CCustomIconSizeDlg dlg;
    dlg.m_nSize = m_nIconSize;
    if (dlg.DoModal() == IDOK)
    {
        m_nIconSize = dlg.m_nSize;
    }
    if (!m_strFilePath.IsEmpty())
    {
        this->LoadFile(m_strFilePath);
    }
}

void CChildFrame::OnUpdateIconCustom(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_bIsIcon);

    bool bIsStandard = (m_nIconSize.cx == 0 && m_nIconSize.cy == 0) ||
        (m_nIconSize == CSize(16, 16)) ||
        (m_nIconSize == CSize(32, 32)) ||
        (m_nIconSize == CSize(48, 48)) ||
        (m_nIconSize == CSize(64, 64)) ||
        (m_nIconSize == CSize(128, 128)) ||
        (m_nIconSize == CSize(256, 256));
    pCmdUI->SetCheck(!bIsStandard);
}

void CChildFrame::OnUpdateIndicatorText(CCmdUI* pCmdUI)
{
    UpdateStatusBar();
}

void CChildFrame::LoadFile(LPCTSTR lpszPath)
{
    UNREFERENCED_PARAMETER(lpszPath);
    // Base implementation: derived classes (CSvgChildFrame, CImageChildFrame)
    // must override this method.
}
