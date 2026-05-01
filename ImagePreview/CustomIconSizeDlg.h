#pragma once
#include "afxdialogex.h"
#include "Resource.h"

/**
 * @brief Custom icon size dialog
 *
 * Allows the user to enter a custom size for icon rendering.
 */
class CCustomIconSizeDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CCustomIconSizeDlg)

public:
    CCustomIconSizeDlg(CWnd* pParent = nullptr);
    virtual ~CCustomIconSizeDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CUSTOM_ICON_SIZE };
#endif

    CSize m_nSize;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();

protected:
    void SetWindowTextForSize(const CSize& size);
};
