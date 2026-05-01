#pragma once
#include "afxdialogex.h"
#include "Resource.h"

/**
 * @brief DPI scaling dialog
 *
 * Allows the user to enter current and target DPI values for scaling.
 */
class CDpiScaleDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CDpiScaleDlg)

public:
    CDpiScaleDlg(CWnd* pParent = nullptr);
    virtual ~CDpiScaleDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DPI_SCALE };
#endif

    int m_nCurrentDpi;
    int m_nTargetDpi;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};
