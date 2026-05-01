// SvgChildFrm.cpp: Implementation of the CSvgChildFrame class
// Derived from CChildFrame for loading SVG files
//

#include "pch.h"
#include "framework.h"
#include "ImagePreview.h"

#include "SvgChildFrm.h"
#include "Resource.h"
#include "DpiScaleDlg.h"

#include <afxpriv.h>

#pragma comment(lib, "Msimg32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSvgChildFrame

IMPLEMENT_DYNAMIC(CSvgChildFrame, CChildFrame)

BEGIN_MESSAGE_MAP(CSvgChildFrame, CChildFrame)
    ON_WM_CREATE()
    ON_COMMAND(ID_DPI_SCALE, &CSvgChildFrame::OnDpiScale)
    ON_WM_PAINT()
    ON_UPDATE_COMMAND_UI(AFX_IDS_IDLEMESSAGE, &CSvgChildFrame::OnUpdateIndicatorText)
END_MESSAGE_MAP()

// CSvgChildFrame construction/destruction

CSvgChildFrame::CSvgChildFrame() noexcept
{
    m_svgImage = nullptr;
    m_nIconSize.cx = 24;
    m_nIconSize.cy = 24;
}

CSvgChildFrame::~CSvgChildFrame()
{
    // m_svgImage (unique_ptr) auto-destroys after this body, calling
    // ~CSVGImage() → Cleanup() before ~CChildFrame() deletes m_hBitmap.
    // No explicit cleanup needed — unique_ptr ensures correct ordering.
}

// CSvgChildFrame diagnostics

#ifdef _DEBUG
void CSvgChildFrame::AssertValid() const
{
    CChildFrame::AssertValid();
}

void CSvgChildFrame::Dump(CDumpContext& dc) const
{
    CChildFrame::Dump(dc);
}
#endif //_DEBUG

// CSvgChildFrame message handlers

int CSvgChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CChildFrame::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_svgImage = std::make_unique<CSVGImage>();
    if (!m_svgImage->Initialize(USER_DEFAULT_SCREEN_DPI))
    {
        return -1;
    }
    return 0;
}

void CSvgChildFrame::OnDpiScale()
{
    CDpiScaleDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        if (dlg.m_nTargetDpi > 0 && m_strFilePath.GetLength() > 0)
        {
            SetScale((double)dlg.m_nTargetDpi / (double)dlg.m_nCurrentDpi);
        }
    }
}

void CSvgChildFrame::SetScale(double dScale)
{
    m_dScale = dScale;
    m_svgImage->SetDPI(m_dScale * USER_DEFAULT_SCREEN_DPI);
    DeleteObject(m_hBitmap);
    m_hBitmap = nullptr;
    LoadSvgFile(m_strFilePath);
    Invalidate();
}

void CSvgChildFrame::LoadFile(LPCTSTR lpszPath)
{
    // Delegate to LoadSvgFile
    LoadSvgFile(lpszPath);
}

void CSvgChildFrame::LoadSvgFile(LPCTSTR lpszPath)
{
    // Clean up previous resources
    if (m_hBitmap != NULL)
    {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }

    m_strFilePath = lpszPath;
    CString strExt = m_strFilePath.Mid(m_strFilePath.ReverseFind('.') + 1);
    m_bIsSvg = (strExt.CompareNoCase(_T("svg")) == 0);

    // Load SVG file using new Image layer
    HRESULT hr = m_svgImage->LoadFromFile(lpszPath);
    if (SUCCEEDED(hr))
    {
        // Get original size
        SIZE originalSize = m_svgImage->GetOriginalSize();
        UINT currentDPI = m_svgImage->GetCurrentDPI();

        // Calculate scaled size based on current scale factor
        FLOAT scaledWidth = static_cast<FLOAT>(originalSize.cx) * m_dScale;
        FLOAT scaledHeight = static_cast<FLOAT>(originalSize.cy) * m_dScale;

        // Convert to HBITMAP
        m_hBitmap = m_svgImage->ToHBITMAP();

        if (m_hBitmap)
        {
            CString strTitle = lpszPath;

            BITMAP bm;
            ::GetObject(m_hBitmap, sizeof(BITMAP), &bm);
            int nWidth = bm.bmWidth;
            int nHeight = bm.bmHeight;

            int nClientWidth = 10 + nWidth + 10 + 10;
            int nClientHeight = 10 + nHeight + 10;

            CRect rectStatusBar;
            m_wndStatusBar.GetWindowRect(&rectStatusBar);
            int nStatusBarHeight = rectStatusBar.Height();

            CRect rc(0, 0, nClientWidth, nClientHeight + nStatusBarHeight);
            CalcWindowRect(&rc);

            SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(),
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

            Invalidate();
        }
    }

    if (!m_hBitmap)
    {
        AfxMessageBox(_T("Failed to load SVG file."));
    }
    Invalidate();
}

void CSvgChildFrame::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    if (m_hBitmap != NULL)
    {
        // Get Bitmap Information
        BITMAP bm;
        ::GetObject(m_hBitmap, sizeof(BITMAP), &bm);
        int nWidth = bm.bmWidth;
        int nHeight = bm.bmHeight;

        // Create Memory DC
        HDC hdcMem = ::CreateCompatibleDC(dc.m_hDC);
        HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem, m_hBitmap);

        // Calculate scaled size
        int nScaledWidth = nWidth;
        int nScaledHeight = nHeight;

        if (bm.bmBitsPixel == 32)
        {
            // Use AlphaBlend for 32-bit images to handle transparency
            BLENDFUNCTION bf;
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = 255;
            bf.AlphaFormat = AC_SRC_ALPHA;

            // Draw original at (10,10)
            ::AlphaBlend(dc.m_hDC, 10, 10, nWidth, nHeight,
                hdcMem, 0, 0, nWidth, nHeight, bf);
        }
        // Cleanup
        ::SelectObject(hdcMem, hOldBitmap);
        ::DeleteDC(hdcMem);
    }
}

BOOL CSvgChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CChildFrame::PreCreateWindow(cs))
        return FALSE;

    return TRUE;
}

void CSvgChildFrame::UpdateStatusBar()
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

    strStatus.Format(_T("SVG Current Size: %d x %d | Zoom: %.0f%%"), nWidth, nHeight, m_dScale * 100.0);
    m_wndStatusBar.SetPaneText(0, strStatus);
    m_wndStatusBar.UpdateData(FALSE);
}
