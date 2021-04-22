
// Load SRTDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Load SRT.h"
#include "Load SRTDlg.h"
#include "afxdialogex.h"
#include "DlgLoadSRTProgress.h"
#include "SRTDataManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLoadSRTDlg dialog



CLoadSRTDlg::CLoadSRTDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOADSRT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLoadSRTDlg::DoDataExchange(CDataExchange* pDX)
{
   DDX_Control(pDX, IDC_STATIC_DRAW_SRT, m_staticDrawSRT);
   DDX_Control(pDX, IDC_BUTTON_LOAD_SRT_FILE, m_btnLoad);
   DDX_Control(pDX, IDC_SLIDER_PROGRESS, m_sliderProgress);

	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLoadSRTDlg, CDialogEx)
	ON_WM_PAINT()
   ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
   ON_WM_HSCROLL()
   ON_BN_CLICKED(IDC_BUTTON_LOAD_SRT_FILE, &CLoadSRTDlg::OnBnClickedButtonLoadSrtFile)
END_MESSAGE_MAP()


// CLoadSRTDlg message handlers

BOOL CLoadSRTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   m_sliderProgress.SetRange(0, 100);
   m_sliderProgress.SetPos(0);

	// TODO: Add extra initialization here
   MoveWindow(0, 0, 900, 600);
   CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLoadSRTDlg::OnSize(UINT nType, int cx, int cy)
{
   CDialogEx::OnSize(nType, cx, cy);

   if (m_btnLoad.GetSafeHwnd() == nullptr || m_staticDrawSRT.GetSafeHwnd() == nullptr)
   {
      return;
   }

   CRect rcClient;
   GetClientRect(rcClient);

   CRect rcButton;
   rcButton.bottom = rcClient.bottom - 10;
   rcButton.top = rcButton.bottom - 20;
   rcButton.left = rcClient.left + 10;
   rcButton.right = rcButton.left + 60;
   m_btnLoad.MoveWindow(rcButton);

   CRect rcSlider;
   rcSlider.left = rcButton.right + 20;
   rcSlider.right = rcClient.right - 10;
   rcSlider.top = rcButton.top;
   rcSlider.bottom = rcButton.bottom;
   m_sliderProgress.MoveWindow(rcSlider);

   CRect rcStatic;
   rcStatic.bottom = rcButton.top - 10;
   rcStatic.top = rcClient.top + 10;
   rcStatic.left = rcClient.left + 10;
   rcStatic.right = rcClient.right - 10;
   m_staticDrawSRT.MoveWindow(rcStatic);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLoadSRTDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLoadSRTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLoadSRTDlg::OnBnClickedButtonLoadSrtFile()
{
   TCHAR szFilters[] = _T("Subtitle Files(*.srt)|*.srt||");
   CFileDialog dlgFileSelect(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilters, this);

   dlgFileSelect.m_ofn.lpstrTitle = _T("Open");

   if (dlgFileSelect.DoModal() != IDOK)
   {
      return;
   }

   CString strFilePath = dlgFileSelect.GetPathName();

   if (!PathFileExists(strFilePath))
   {
      return;
   }

   CDlgLoadSRTProgress dlgProgress;
   dlgProgress.SetFilePath(strFilePath);

   if (dlgProgress.DoModal() != IDOK)
   {
      return;
   }

   CString strStartTime, strEndTime, strContent;

   std::tie(strStartTime, strEndTime, strContent) = CSRTDataManager::GetInstance()->GetSRTData(0.f);

   m_staticDrawSRT.ShowSRTData(strStartTime, strEndTime, strContent);

}

void CLoadSRTDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   if (m_sliderProgress.GetSafeHwnd() == nullptr)
   {
      return;
   }

   int nCurPos = m_sliderProgress.GetPos();

   CString strStartTime, strEndTime, strContent;

   std::tie(strStartTime, strEndTime, strContent) = CSRTDataManager::GetInstance()->GetSRTData((float)nCurPos / 100.f);

   m_staticDrawSRT.ShowSRTData(strStartTime, strEndTime, strContent);
}
