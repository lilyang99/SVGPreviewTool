// DpiScaleDlg.cpp : implementation file
//

#include "pch.h"
#include "ImagePreview.h"
#include "DpiScaleDlg.h"
#include "afxdialogex.h"


// CDpiScaleDlg dialog

IMPLEMENT_DYNAMIC(CDpiScaleDlg, CDialogEx)

CDpiScaleDlg::CDpiScaleDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DPI_SCALE, pParent)
    , m_nCurrentDpi(96)
    , m_nTargetDpi(96)
{

}

CDpiScaleDlg::~CDpiScaleDlg()
{
}

void CDpiScaleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_CURRENT_DPI, m_nCurrentDpi);
    DDX_Text(pDX, IDC_EDIT_TARGET_DPI, m_nTargetDpi);
    DDV_MinMaxInt(pDX, m_nCurrentDpi, 1, 1000);
    DDV_MinMaxInt(pDX, m_nTargetDpi, 1, 1000);
}


BEGIN_MESSAGE_MAP(CDpiScaleDlg, CDialogEx)
END_MESSAGE_MAP()


// CDpiScaleDlg message handlers


BOOL CDpiScaleDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
