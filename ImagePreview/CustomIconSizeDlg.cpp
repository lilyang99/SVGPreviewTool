// CustomIconSizeDlg.cpp : implementation file
//

#include "pch.h"
#include "ImagePreview.h"
#include "CustomIconSizeDlg.h"
#include "afxdialogex.h"


// CCustomIconSizeDlg dialog

IMPLEMENT_DYNAMIC(CCustomIconSizeDlg, CDialogEx)

CCustomIconSizeDlg::CCustomIconSizeDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_CUSTOM_ICON_SIZE, pParent)
    , m_nSize(CSize(0, 0))
{

}

CCustomIconSizeDlg::~CCustomIconSizeDlg()
{
}

void CCustomIconSizeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_ICON_SIZE, m_nSize.cy);
    DDX_Text(pDX, IDC_EDIT_ICON_SIZE2, m_nSize.cx);
    DDV_MinMaxInt(pDX, m_nSize.cy, 1, 2048);
    DDV_MinMaxInt(pDX, m_nSize.cx, 1, 2048);
}


BEGIN_MESSAGE_MAP(CCustomIconSizeDlg, CDialogEx)
END_MESSAGE_MAP()


// CCustomIconSizeDlg message handlers


BOOL CCustomIconSizeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Set initial values from m_nSize to dialog controls
    SetWindowTextForSize(m_nSize);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomIconSizeDlg::SetWindowTextForSize(const CSize& size)
{
    CString strWidth, strHeight;
    strWidth.Format(_T("%d"), size.cx);
    strHeight.Format(_T("%d"), size.cy);
    SetDlgItemText(IDC_EDIT_ICON_SIZE, strHeight);
    SetDlgItemText(IDC_EDIT_ICON_SIZE2, strWidth);
}
