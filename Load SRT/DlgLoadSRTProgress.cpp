#include "stdafx.h"
#include "resource.h"
#include "DlgLoadSRTProgress.h"
#include <iostream>
#include <fstream>
#include <string>
#include "SRTDataManager.h"

#define WM_MSG_REFRESH                  WM_USER + 111
#define WM_MSG_LOAD_FINISHED            WM_USER + 112

IMPLEMENT_DYNAMIC(CDlgLoadSRTProgress, CDialogEx)

void LoadSRTThread(LPVOID lpData)
{
   CDlgLoadSRTProgress* pWnd = (CDlgLoadSRTProgress*)lpData;

   BOOL bFinished = FALSE;

   while (!pWnd->GetAbortThread() && !bFinished)
   {
      CString strFilePath = pWnd->GetFilePath();

      if (strFilePath.IsEmpty() || !PathFileExists(strFilePath))
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(50));
         continue;
      }

      // 1. Get line count
      int nLineCount = 0;
      HRESULT hResult = pWnd->GetSRTFileLineCount(nLineCount);

      if (FAILED(hResult))
      {
         pWnd->OpenFileFailed();
         break;
      }

      std::ifstream fileSRT;
      fileSRT.open(strFilePath);

      if (fileSRT.fail())
      {
         break;
      }

      int nState = LOAD_INITIALIZE;
      
      CString strTimeInfo = _T("");
      CString strStratTimeInfo = _T("");
      CString strEndTimeInfo = _T("");
      CString strContent = _T("");

      int nCurLine = 0;

      std::string strLine;
      while (std::getline(fileSRT, strLine))
      {
         ++nCurLine;

         CString strCurLine = _T("");
         strCurLine.Format(_T("\r\n ----- current line : %d ------"), nCurLine);
         OutputDebugString(strCurLine);

         if (pWnd->GetAbortThread())
         {
            break;
         }

         if (strLine.empty())
         {
            if (nState != LOAD_CONTENT)
            {
               nState = LOAD_FAILED;
               break;
            }

            if (strStratTimeInfo.IsEmpty() || strEndTimeInfo.IsEmpty() || strContent.IsEmpty())
            {
               nState = LOAD_INITIALIZE;
               continue;
            }

            pWnd->AddSRTData(strStratTimeInfo, strEndTimeInfo, strContent);

            pWnd->LoadingFile((float)nCurLine / (float)nLineCount);

            strStratTimeInfo.Empty();
            strEndTimeInfo.Empty();
            strContent.Empty();

            nState = LOAD_INITIALIZE;
            continue;
         }

         switch (nState)
         {
            case LOAD_INITIALIZE:
               nState = LOAD_TIME_INFO;
               break;
            case LOAD_TIME_INFO:
               pWnd->GetTimeInfo(strLine.c_str(), strStratTimeInfo, strEndTimeInfo);
               nState = LOAD_CONTENT;
               break;
            case LOAD_CONTENT:
               if (!strContent.IsEmpty())
               {
                  strContent += _T("\n");
               }

               strContent += pWnd->ConvertStringToUnicodeCString(strLine.c_str());
               break;
            default:
               break;
         }
      }

      if (nState == LOAD_FAILED)
      {
         pWnd->LoadFailed();
      }
      else
      {
         pWnd->LoadFinished();
      }

      bFinished = TRUE;
   }

   pWnd->PostMessageW(WM_MSG_LOAD_FINISHED);
}

CDlgLoadSRTProgress::CDlgLoadSRTProgress(CWnd* pParent) : CDialogEx(IDD_DIALOG_PROGRESS, pParent)
{
   m_bAbortThread = FALSE;
   m_pThread = nullptr;

   m_nLoadingState = LOADING_INITIALIZE;

   m_strDescription = _T("Initializing...");

   m_fProgress = 0.f;

   m_pFont = new CFont();
   m_pFont->CreateFont(14,//   nHeight   
      0,                                  //   nWidth   
      0,                                  //   nEscapement   
      0,                                  //   nOrientation   
      FW_NORMAL,                          //   nWeight   
      FALSE,                              //   bItalic   
      FALSE,                              //   bUnderline   
      0,                                  //   cStrikeOut   
      ANSI_CHARSET,                       //   nCharSet   
      OUT_STROKE_PRECIS,                  //   nOutPrecision   
      CLIP_STROKE_PRECIS,                 //   nClipPrecision   
      PROOF_QUALITY,                      //   nQuality   
      DEFAULT_PITCH | FF_DONTCARE,        //   nPitchAndFamily   
      _T("Arial"));                       //   lpszFacename  
}

CDlgLoadSRTProgress::~CDlgLoadSRTProgress()
{
   delete m_pFont;
   m_pFont = nullptr;
}

void CDlgLoadSRTProgress::DoDataExchange(CDataExchange* pDX)
{
   CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgLoadSRTProgress, CDialogEx)
   ON_WM_ERASEBKGND()
   ON_WM_PAINT()
   ON_MESSAGE(WM_MSG_REFRESH, OnRefreshMsg)
   ON_MESSAGE(WM_MSG_LOAD_FINISHED, OnLoadFinished)
END_MESSAGE_MAP()

BOOL CDlgLoadSRTProgress::OnInitDialog()
{
   CDialogEx::OnInitDialog();

   return TRUE;
}

BOOL CDlgLoadSRTProgress::OnEraseBkgnd(CDC* pDC)
{
   return FALSE;
}


void CDlgLoadSRTProgress::OnPaint()
{
   CPaintDC dc(this);

   CRect rcClient;
   GetClientRect(rcClient);

   CDC memDC;
   memDC.CreateCompatibleDC(&dc);

   CBitmap memBmp;
   memBmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
   memDC.SelectObject(memBmp);

   memDC.FillSolidRect(0, 0, rcClient.Width(), rcClient.Height(), RGB(255, 0, 0));

   // draw text
   CRect rcDescription;
   rcDescription.left = rcClient.left + 10;
   rcDescription.right = rcDescription.left + 60;
   rcDescription.top = rcClient.top + 10;
   rcDescription.bottom = rcDescription.top + 20;

   CFont* pOldFont = memDC.SelectObject(m_pFont);
   memDC.SetTextColor(RGB(255, 255, 255));
   memDC.DrawText(m_strDescription, -1, rcDescription, DT_WORDBREAK | DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

   // draw progress
   CRect rcProgress;
   rcProgress.left = rcClient.left + 10;
   rcProgress.right = rcClient.right - 10;
   rcProgress.top = rcDescription.bottom + 20;
   rcProgress.bottom = rcProgress.top + 20;

   memDC.FillSolidRect(rcProgress, RGB(0, 255, 0));

   int nProgressWidth = rcProgress.Width();

   rcProgress.right = rcProgress.left + nProgressWidth * m_fProgress;

   memDC.FillSolidRect(rcProgress, RGB(0, 0, 255));

   dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

   memDC.DeleteDC();
   memBmp.DeleteObject();
}

void CDlgLoadSRTProgress::OnClose()
{
   AbortThread();
}

LRESULT CDlgLoadSRTProgress::OnRefreshMsg(WPARAM wParam, LPARAM lParam)
{
   Invalidate();
   UpdateWindow();

   return LRESULT();
}

LRESULT CDlgLoadSRTProgress::OnLoadFinished(WPARAM wParam, LPARAM lParam)
{
   if (m_pThread != nullptr)
   {
      if (m_pThread->joinable())
      {
         m_pThread->join();
      }
   }

   EndDialog(IDOK);

   return LRESULT();
}

void CDlgLoadSRTProgress::SetFilePath(CString strFilePath)
{
   m_strFilePath = strFilePath;

   StartThread();
}

HRESULT CDlgLoadSRTProgress::GetSRTFileLineCount(int & nLineCount)
{
   if (m_strFilePath.IsEmpty() || !PathFileExists(m_strFilePath))
   {
      return S_FALSE;
   }

   std::ifstream fileSRT;
   fileSRT.open(m_strFilePath);

   if (fileSRT.fail())
   {
      return S_FALSE;
   }

   int nSRTLineCount = 0;

   std::string strLine;
   while (std::getline(fileSRT, strLine))
   {
      if (m_bAbortThread)
      {
         return S_FALSE;
      }

      ++nSRTLineCount;
   }

   nLineCount = nSRTLineCount;

   return S_OK;
}

void CDlgLoadSRTProgress::LoadSRTData()
{
}

void CDlgLoadSRTProgress::OpenFileFailed()
{
   m_nLoadingState = LOADING_OPEN_FAILED;

   EndDialog(IDCLOSE);
}

void CDlgLoadSRTProgress::CountingLines()
{
   m_nLoadingState = LOADING_COUNT_LINES;

   m_strDescription = _T("Couting Lines");

   Invalidate();
   UpdateWindow();
}

void CDlgLoadSRTProgress::LoadingFile(float fProgress)
{
   m_nLoadingState = LOADING_PROGRESS;

   m_strDescription = _T("Loading...");

   m_fProgress = fProgress;

   PostMessage(WM_MSG_REFRESH);
}

void CDlgLoadSRTProgress::LoadFailed()
{
   /*m_bAbortThread = TRUE;

   EndDialog(IDOK);*/
}

void CDlgLoadSRTProgress::LoadFinished()
{
   m_nLoadingState = LOADING_SUCEESS;

   m_strDescription = _T("Finished!");

   m_fProgress = 1.f;

   PostMessage(WM_MSG_REFRESH);

   /*m_bAbortThread = TRUE;

   EndDialog(IDOK);*/
}

CString CDlgLoadSRTProgress::ConvertStringToUnicodeCString(const char * pszUTF8)
{
   int nStrLen = MultiByteToWideChar(CP_UTF8, 0, pszUTF8, -1, NULL, 0);
   if (nStrLen == 0)
   {
      return CString();
   }

   CString strConvert = _T("");
   int nRet = MultiByteToWideChar(CP_UTF8, 0, pszUTF8, -1, strConvert.GetBuffer(nStrLen), nStrLen);
   strConvert.ReleaseBuffer();

   return strConvert;
}

void CDlgLoadSRTProgress::GetTimeInfo(const char * pszUTF8, CString & strStartTime, CString & strEndTime)
{
   CString strTimeInfo = ConvertStringToUnicodeCString(pszUTF8);

   if (strTimeInfo.IsEmpty())
   {
      return;
   }

   int nPos = strTimeInfo.Find(_T("-->"));
   if (nPos < 0)
   {
      return;
   }

   strStartTime = strTimeInfo.Left(nPos);
   strStartTime.Trim();

   nPos += 3;
   int nLength = strTimeInfo.GetLength() - nPos;
   strEndTime = strTimeInfo.Mid(nPos, nLength);
   strEndTime.Trim();
}

void CDlgLoadSRTProgress::AddSRTData(CString strStartTime, CString strEndTime, CString strContent)
{
   CSRTDataManager::GetInstance()->AddSRTData(strStartTime, strEndTime, strContent);
}

void CDlgLoadSRTProgress::StartThread()
{
   if (m_pThread != nullptr)
   {
      return;
   }

   m_pThread = std::make_unique<std::thread>(LoadSRTThread, (LPVOID)this);
}

void CDlgLoadSRTProgress::AbortThread()
{
   if (m_pThread == nullptr)
   {
      return;
   }

   m_bAbortThread = TRUE;

   if (m_pThread->joinable())
   {
      m_pThread->join();
   }

   m_pThread = nullptr;
}
