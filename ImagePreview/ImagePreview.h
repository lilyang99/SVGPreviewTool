// ImagePreview.h: Main header file for the ImagePreview application
//
#pragma once

#ifndef __AFXWIN_H__
#error "Include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

/**
 * @brief ImagePreview application class
 *
 * Main application class for the ImagePreview MFC application.
 * See ImagePreview.cpp for the implementation.
 */
class CImagePreviewApp : public CWinApp
{
public:
    CImagePreviewApp() noexcept;

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    afx_msg void OnAppAbout();
    afx_msg void OnFileOpen();
    afx_msg BOOL OnOpenRecentFile(UINT nID);
    DECLARE_MESSAGE_MAP()
};

extern CImagePreviewApp theApp;
