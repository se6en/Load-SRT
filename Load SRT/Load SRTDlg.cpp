
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
   DDX_Control(pDX, IDC_BUTTON_EXPORT_TXT, m_btnExportTXT);
   DDX_Control(pDX, IDC_BUTTON_EXPORT_SRT, m_btnExportSRT);

	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLoadSRTDlg, CDialogEx)
	ON_WM_PAINT()
   ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
   ON_WM_HSCROLL()
   ON_BN_CLICKED(IDC_BUTTON_LOAD_SRT_FILE, &CLoadSRTDlg::OnBnClickedButtonLoadSrtFile)
   ON_BN_CLICKED(IDC_BUTTON_EXPORT_TXT, &CLoadSRTDlg::OnBnClickedButtonExportTXT)
   ON_BN_CLICKED(IDC_BUTTON_EXPORT_SRT, &CLoadSRTDlg::OnBnClickedButtonExportSRT)
END_MESSAGE_MAP()


// CLoadSRTDlg message handlers

BOOL CLoadSRTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   m_btnExportTXT.EnableWindow(FALSE);
   m_btnExportSRT.EnableWindow(FALSE);

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

   CRect rcLoadButton;
   rcLoadButton.bottom = rcClient.bottom - 10;
   rcLoadButton.top = rcLoadButton.bottom - 20;
   rcLoadButton.left = rcClient.left + 10;
   rcLoadButton.right = rcLoadButton.left + 60;
   m_btnLoad.MoveWindow(rcLoadButton);

   CRect rcExportButton(rcLoadButton);
   rcExportButton.right = rcClient.right - 10;
   rcExportButton.left = rcExportButton.right - 60;
   m_btnExportSRT.MoveWindow(rcExportButton);

   rcExportButton.right = rcExportButton.left - 10;
   rcExportButton.left = rcExportButton.right - 60;
   m_btnExportTXT.MoveWindow(rcExportButton);

   CRect rcSlider;
   rcSlider.left = rcLoadButton.right + 20;
   rcSlider.right = rcExportButton.left - 20;
   rcSlider.top = rcLoadButton.top;
   rcSlider.bottom = rcLoadButton.bottom;
   m_sliderProgress.MoveWindow(rcSlider);

   CRect rcStatic;
   rcStatic.bottom = rcLoadButton.top - 10;
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
   
   if (FAILED(dlgProgress.SetFilePath(strFilePath)))
   {
      return;
   }

   if (dlgProgress.DoModal() != IDOK)
   {
      return;
   }

   m_btnExportTXT.EnableWindow(TRUE);
   m_btnExportSRT.EnableWindow(TRUE);

   /*CSRTDataManager::SRTData data;
   data.startTime = CSRTDataManager::GetInstance()->GetSRTDataStartTime(0.f);
   data.endTime = CSRTDataManager::GetInstance()->GetSRTDataEndTime(0.f);
   data.content = CSRTDataManager::GetInstance()->GetSRTDataContent(0.f);
   CSRTDataManager::GetInstance()->GetSRTDataBoldInfo(0.f, data.vecBoldInfo);
   CSRTDataManager::GetInstance()->GetSRTDataItalicInfo(0.f, data.vecItalicInfo);
   CSRTDataManager::GetInstance()->GetSRTDataUnderlineInfo(0.f, data.vecUnderlineInfo);
   CSRTDataManager::GetInstance()->GetSRTDataColorInfo(0.f, data.vecColorInfo);*/

   m_staticDrawSRT.ShowSRTData(0.f);
}

void CLoadSRTDlg::OnBnClickedButtonExportTXT()
{
   if (!CSRTDataManager::GetInstance()->HasValidData())
   {
      return;
   }

   // need to ask for a name and need a workflow to judge whether can create a file, just use a default name as this case
   TCHAR szFolder[MAX_PATH + 1] = _T("");
   if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, szFolder) != S_OK)
   {
      return;
   }

   CString strFolderRoot = szFolder;
   CString strFileName = strFolderRoot + _T("\\ExportTXT.txt");

   CFile file(strFileName, CFile::modeCreate | CFile::modeWrite);

   int nCount = CSRTDataManager::GetInstance()->GetSRTDataCount();
   for (int i = 0; i < nCount; i++)
   {
      CString strContent = CSRTDataManager::GetInstance()->GetUnstyledContent(i);
      if (!strContent.IsEmpty())
      {
         CStringA strUTF8;
         int nLegnth = CSRTDataManager::GetInstance()->ConvertUnicodeToUTF8CString(strContent, strUTF8);
         if (nLegnth > 0 && !strUTF8.IsEmpty())
         {
            file.Write(strUTF8, strUTF8.GetLength());
         }
      }

      file.Write("\r\n", 2);
      file.Write("\r\n", 2);
   }

   file.Close();
}

void CLoadSRTDlg::OnBnClickedButtonExportSRT()
{
   if (!CSRTDataManager::GetInstance()->HasValidData())
   {
      return;
   }

   // need to ask for a name and need a workflow to judge whether can create a file, just use a default name as this case
   TCHAR szFolder[MAX_PATH + 1] = _T("");
   if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, szFolder) != S_OK)
   {
      return;
   }

   CString strFolderRoot = szFolder;
   CString strFileName = strFolderRoot + _T("\\ExportSRT.srt");

   CFile file(strFileName, CFile::modeCreate | CFile::modeWrite);

   int nCount = CSRTDataManager::GetInstance()->GetSRTDataCount();
   for (int i = 0; i < nCount; i++)
   {
      // index
      CString strIndex;
      strIndex.Format(_T("%d"), i + 1);
      CStringA strUTF8Index;
      int nIndexLegnth = CSRTDataManager::GetInstance()->ConvertUnicodeToUTF8CString(strIndex, strUTF8Index);
      if (nIndexLegnth > 0 && !strUTF8Index.IsEmpty())
      {
         file.Write(strUTF8Index, strUTF8Index.GetLength());
         file.Write("\r\n", 2);
      }

      CString strTime = CSRTDataManager::GetInstance()->GetTime(i);
      CStringA strUTF8Time;
      int nTimeLegnth = CSRTDataManager::GetInstance()->ConvertUnicodeToUTF8CString(strTime, strUTF8Time);
      if (nTimeLegnth > 0 && !strUTF8Time.IsEmpty())
      {
         file.Write(strUTF8Time, strUTF8Time.GetLength());
         file.Write("\r\n", 2);
      }

      CString strContent = CSRTDataManager::GetInstance()->GetStyledContent(i);
      if (!strContent.IsEmpty())
      {
         CStringA strUTF8;
         int nLegnth = CSRTDataManager::GetInstance()->ConvertUnicodeToUTF8CString(strContent, strUTF8);
         if (nLegnth > 0 && !strUTF8.IsEmpty())
         {
            file.Write(strUTF8, strUTF8.GetLength());
            file.Write("\r\n", 2);
         }
      }

      file.Write("\r\n", 2);
   }

   file.Close();
}

void CLoadSRTDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   if (m_sliderProgress.GetSafeHwnd() == nullptr)
   {
      return;
   }

   float fCurPos = static_cast<float>(m_sliderProgress.GetPos()) / 100.f;

   /*CSRTDataManager::SRTData data;
   data.startTime = CSRTDataManager::GetInstance()->GetSRTDataStartTime(fCurPos);
   data.endTime = CSRTDataManager::GetInstance()->GetSRTDataEndTime(fCurPos);
   data.content = CSRTDataManager::GetInstance()->GetSRTDataContent(fCurPos);
   CSRTDataManager::GetInstance()->GetSRTDataBoldInfo(fCurPos, data.vecBoldInfo);
   CSRTDataManager::GetInstance()->GetSRTDataItalicInfo(fCurPos, data.vecItalicInfo);
   CSRTDataManager::GetInstance()->GetSRTDataUnderlineInfo(fCurPos, data.vecUnderlineInfo);
   CSRTDataManager::GetInstance()->GetSRTDataColorInfo(fCurPos, data.vecColorInfo);*/

   m_staticDrawSRT.ShowSRTData(fCurPos);
}
