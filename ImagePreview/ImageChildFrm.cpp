// ImageChildFrm.cpp: Implementation of the CImageChildFrame class
//

#include "pch.h"
#include "framework.h"
#include "ImagePreview.h"

#include "ImageChildFrm.h"

#include <afxpriv.h>
#include <atlimage.h>

#pragma comment(lib, "Msimg32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CImageChildFrame, CChildFrame)

BEGIN_MESSAGE_MAP(CImageChildFrame, CChildFrame)
    ON_UPDATE_COMMAND_UI(AFX_IDS_IDLEMESSAGE, &CChildFrame::OnUpdateIndicatorText)
END_MESSAGE_MAP()

// CImageChildFrame construction/destruction

CImageChildFrame::CImageChildFrame() noexcept
{
}

CImageChildFrame::~CImageChildFrame()
{
}

// CImageChildFrame diagnostics

#ifdef _DEBUG
void CImageChildFrame::AssertValid() const
{
    CChildFrame::AssertValid();
}

void CImageChildFrame::Dump(CDumpContext& dc) const
{
    CChildFrame::Dump(dc);
}
#endif //_DEBUG

// CImageChildFrame message handlers

BOOL CImageChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    return CChildFrame::PreCreateWindow(cs);
}

void CImageChildFrame::LoadFile(LPCTSTR lpszPath)
{
    LoadImageFile(lpszPath);
}

/**
 * @brief Resize the frame window to fit the loaded bitmap
 * @param nBitmapWidth  Width of the bitmap in pixels
 * @param nBitmapHeight Height of the bitmap in pixels
 */
void CImageChildFrame::ResizeFrameToFitBitmap(int nBitmapWidth, int nBitmapHeight)
{
    // Client area: margin + original + gap + zoom-room (1.5x) + margin
    int nClientWidth = 10 + nBitmapWidth + 10 + nBitmapWidth * 1.5 + 10;
    int nClientHeight = 10 + nBitmapHeight + 10;

    CRect rectStatusBar;
    m_wndStatusBar.GetWindowRect(&rectStatusBar);
    int nStatusBarHeight = rectStatusBar.Height();

    CRect rc(0, 0, nClientWidth, nClientHeight + nStatusBarHeight);
    CalcWindowRect(&rc);

    SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(),
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CImageChildFrame::LoadImageFile(LPCTSTR lpszPath)
{
    if (lpszPath == nullptr || _tcslen(lpszPath) == 0)
        return;

    // Clean up previous bitmap
    if (m_hBitmap != NULL)
    {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }

    m_strFilePath = lpszPath;
    CString strExt = m_strFilePath.Mid(m_strFilePath.ReverseFind('.') + 1);
    m_bIsIcon = (strExt.CompareNoCase(_T("ico")) == 0);
    m_bIsSvg = false;

    // -----------------------------------------------------------------------
    // Icon loading path — converts HICON to 32-bit HBITMAP via DIB section
    // -----------------------------------------------------------------------
    if (m_bIsIcon)
    {
        int nTargetW = m_nIconSize.cx == 0 ? 0 : m_nIconSize.cx;
        int nTargetH = m_nIconSize.cy == 0 ? 0 : m_nIconSize.cy;

        HICON hIcon = (HICON)::LoadImage(NULL, lpszPath, IMAGE_ICON,
            nTargetW, nTargetH, LR_LOADFROMFILE);
        if (!hIcon)
        {
            AfxMessageBox(_T("Failed to load icon file."));
            return;
        }

        ICONINFO ii = {};
        if (!::GetIconInfo(hIcon, &ii))
        {
            ::DestroyIcon(hIcon);
            return;
        }

        BITMAP bmMask;
        ::GetObject(ii.hbmMask, sizeof(BITMAP), &bmMask);

        int nW = bmMask.bmWidth;
        int nH = bmMask.bmHeight;
        if (ii.hbmColor)
        {
            BITMAP bmColor;
            ::GetObject(ii.hbmColor, sizeof(BITMAP), &bmColor);
            nW = bmColor.bmWidth;
            nH = bmColor.bmHeight;
        }
        else
        {
            // Monochrome icon: mask is 2x height (AND mask above XOR mask)
            nH = nH / 2;
        }

        // Create 32-bit top-down DIB section
        BITMAPINFO bi = {};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = nW;
        bi.bmiHeader.biHeight = nH;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        HDC hDC = ::GetDC(NULL);
        void* pBits = NULL;
        HBITMAP hDibBitmap = ::CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);
        ::ReleaseDC(NULL, hDC);

        if (!hDibBitmap)
        {
            if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
            if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
            ::DestroyIcon(hIcon);
            return;
        }

        HDC hMemDC = ::CreateCompatibleDC(NULL);
        HBITMAP hOld = (HBITMAP)::SelectObject(hMemDC, hDibBitmap);

        // Clear pixels to transparent black
        memset(pBits, 0, nW * nH * 4);

        // Draw icon onto the DIB section
        ::DrawIconEx(hMemDC, 0, 0, hIcon, nW, nH, 0, NULL, DI_NORMAL);

        ::SelectObject(hMemDC, hOld);
        ::DeleteDC(hMemDC);

        // Clean up icon resources
        if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
        if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
        ::DestroyIcon(hIcon);

        m_hBitmap = hDibBitmap;

        ResizeFrameToFitBitmap(nW, nH);
        Invalidate();
        return;
    }

    // -----------------------------------------------------------------------
    // Standard image loading path (BMP, PNG, JPG, etc.)
    // -----------------------------------------------------------------------
    CImage image;
    HRESULT hr = image.Load(lpszPath);
    if (FAILED(hr))
    {
        AfxMessageBox(_T("Failed to load image."));
        return;
    }

    m_hBitmap = image.Detach();

    BITMAP bm;
    ::GetObject(m_hBitmap, sizeof(BITMAP), &bm);

    ResizeFrameToFitBitmap(bm.bmWidth, bm.bmHeight);
    Invalidate();
}
