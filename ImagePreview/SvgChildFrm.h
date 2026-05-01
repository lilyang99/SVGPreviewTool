#pragma once
#include "../SvgRender/Image/CSVGImage.h"
#include "ChildFrm.h"

/**
 * @brief MDI child frame for SVG files
 *
 * Derived from CChildFrame for loading and rendering SVG files.
 */
class CSvgChildFrame : public CChildFrame
{
    DECLARE_DYNAMIC(CSvgChildFrame)
public:
    CSvgChildFrame() noexcept;
    virtual ~CSvgChildFrame();

protected:
    std::unique_ptr<CSVGImage> m_svgImage;

public:
    virtual void LoadFile(LPCTSTR lpszPath) override;

protected:
    void LoadSvgFile(LPCTSTR lpszPath);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    virtual void SetScale(double dScale);
    afx_msg void OnPaint();
    virtual void UpdateStatusBar();
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDpiScale();
    DECLARE_MESSAGE_MAP()
};
