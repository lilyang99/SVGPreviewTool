#pragma once
#include <atlimage.h>

/**
 * @brief Base MDI child frame class
 *
 * Provides zoom, icon-size selection, and shared rendering infrastructure.
 * Serves as the base class for both image and SVG child frames.
 */
class CChildFrame : public CMDIChildWnd
{
    DECLARE_DYNAMIC(CChildFrame)
public:
    CChildFrame() noexcept;
    virtual ~CChildFrame();

protected:
    HBITMAP m_hBitmap;
    double m_dScale;
    CStatusBar m_wndStatusBar;
    CSize m_nIconSize;
    bool m_bIsIcon;
    bool m_bIsSvg;
    CString m_strFilePath;

public:
    virtual void UpdateStatusBar();
    virtual void SetScale(double dScale);
    CString GetFilePath() const { return m_strFilePath; }
    double GetScale() const { return m_dScale; }
    HBITMAP GetBitmap() const { return m_hBitmap; }

public:
    virtual void LoadFile(LPCTSTR lpszPath);

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void PostNcDestroy() override;

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnZoom100();
    afx_msg void OnZoom125();
    afx_msg void OnZoom150();
    afx_msg void OnZoom175();
    afx_msg void OnZoom200();
    afx_msg void OnZoom225();
    afx_msg void OnZoom250();
    afx_msg void OnZoom300();
    afx_msg void OnUpdateZoom100(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom125(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom150(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom175(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom200(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom225(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom250(CCmdUI* pCmdUI);
    afx_msg void OnUpdateZoom300(CCmdUI* pCmdUI);
    afx_msg void OnDpiScale();
    afx_msg void OnZoomClear();
    afx_msg void OnIconSize(UINT nID);
    afx_msg void OnUpdateIconSize(CCmdUI* pCmdUI);
    afx_msg void OnIconCustom();
    afx_msg void OnUpdateIconCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorText(CCmdUI* pCmdUI);
    DECLARE_MESSAGE_MAP()
};
