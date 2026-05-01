#pragma once
#include <vector>

class CChildFrame;

/**
 * @brief Main MDI frame window
 *
 * Manages child frames and handles file open / drag-and-drop operations.
 */
class CMainFrame : public CMDIFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame() noexcept;

public:
    void OpenImageFile(CString path);
    void OnChildFrameClosed(CChildFrame* pFrame);

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    DECLARE_MESSAGE_MAP()

private:
    std::vector<CChildFrame*> m_childFrames;
};
