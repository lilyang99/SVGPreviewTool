#pragma once
#include "ChildFrm.h"

/**
 * @brief MDI child frame for standard image formats
 *
 * Derived from CChildFrame for loading standard image formats (BMP, PNG, JPG, etc.).
 */
class CImageChildFrame : public CChildFrame
{
    DECLARE_DYNAMIC(CImageChildFrame)
public:
    CImageChildFrame() noexcept;
    virtual ~CImageChildFrame();

public:
    virtual void LoadFile(LPCTSTR lpszPath) override;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
protected:
    void LoadImageFile(LPCTSTR lpszPath);

private:
    /**
     * @brief Resize the frame window to fit the loaded bitmap
     * @param nBitmapWidth  Width of the bitmap in pixels
     * @param nBitmapHeight Height of the bitmap in pixels
     */
    void ResizeFrameToFitBitmap(int nBitmapWidth, int nBitmapHeight);

    DECLARE_MESSAGE_MAP()
};
