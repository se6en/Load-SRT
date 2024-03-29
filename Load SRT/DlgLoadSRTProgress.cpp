#include "stdafx.h"
#include "resource.h"
#include "DlgLoadSRTProgress.h"
#include <iostream>
#include <fstream>
#include <string>
#include <codecvt>
#include "SRTDataManager.h"

#define WM_MSG_REFRESH                  WM_USER + 111
#define WM_MSG_LOAD_FINISHED            WM_USER + 112

static constexpr auto RETURN_SYMBOL = _T('\r');

IMPLEMENT_DYNAMIC(CDlgLoadSRTProgress, CDialogEx)

void LoadSRTThread(LPVOID lpData)
{
   CDlgLoadSRTProgress* pWnd = (CDlgLoadSRTProgress*)lpData;

   pWnd->DecodeFile();

   //BOOL bFinished = FALSE;

   //while (!pWnd->GetAbortThread() && !bFinished)
   //{
   //   CString strFilePath = pWnd->GetFilePath();

   //   if (strFilePath.IsEmpty() || !PathFileExists(strFilePath))
   //   {
   //      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   //      continue;
   //   }

   //   //1. init state
   //   int nState = LOAD_FAILED;

   //   // 2. Get line count
   //   int nLineCount = 0;
   //   HRESULT hResult = pWnd->GetSRTFileLineCount(nLineCount);

   //   if (FAILED(hResult))
   //   {
   //      pWnd->OpenFileFailed();
   //      break;
   //   }

   //   std::ifstream fileSRT;
   //   fileSRT.open(strFilePath);

   //   if (fileSRT.fail())
   //   {
   //      fileSRT.close();
   //      fileSRT.clear();
   //      break;
   //   }

   //   nState = LOAD_INITIALIZE;
   //   
   //   CString strTimeInfo = _T("");
   //   CString strStratTimeInfo = _T("");
   //   CString strEndTimeInfo = _T("");
   //   std::string content = "";

   //   int nCurLine = 0;

   //   // 3. begin to read file content
   //   std::string strLine;
   //   while (std::getline(fileSRT, strLine))
   //   {
   //      ++nCurLine;

   //      CString strCurLine = _T("");
   //      strCurLine.Format(_T("\r\n ----- current line : %d ------"), nCurLine);
   //      OutputDebugString(strCurLine);

   //      if (pWnd->GetAbortThread())
   //      {
   //         break;
   //      }

   //      if (nState == LOAD_FAILED)
   //      {
   //         break;
   //      }

   //      // checked several applications that an extra line without any content will be ignored directly
   //      if (strLine.empty())
   //      {
   //         if (nState == LOAD_CONTENT)
   //         {
   //            if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !content.empty())
   //            {
   //               pWnd->AddSRTData(strStratTimeInfo, strEndTimeInfo, content);

   //               strStratTimeInfo.Empty();
   //               strEndTimeInfo.Empty();
   //               content = "";

   //               nState = LOAD_INITIALIZE;
   //            }
   //         }
   //         continue;
   //      }

   //      switch (nState)
   //      {
   //         case LOAD_INITIALIZE:
   //         {
   //            nState = LOAD_TIME_INFO;
   //         }
   //         break;
   //         case LOAD_TIME_INFO:
   //         {
   //            if (FAILED(pWnd->GetTimeInfo(strLine.c_str(), strStratTimeInfo, strEndTimeInfo)))
   //            {
   //               nState = LOAD_FAILED;
   //            }
   //            else
   //            {
   //               nState = LOAD_CONTENT;
   //            }
   //         }
   //         break;
   //         case LOAD_CONTENT:
   //         {
   //            if (!content.empty())
   //            {
   //               content += "\n";
   //            }
   //            content += strLine;
   //         }
   //         break;
   //         default:
   //            break;
   //      }
   //   }

   //   fileSRT.close();
   //   fileSRT.clear();

   //   if (nState == LOAD_FAILED)
   //   {
   //      pWnd->LoadFailed();
   //   }
   //   else
   //   {
   //      if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !content.empty())
   //      {
   //         pWnd->AddSRTData(strStratTimeInfo, strEndTimeInfo, content);
   //      }

   //      pWnd->LoadingFile((float)nCurLine / (float)nLineCount);

   //      pWnd->LoadFinished();
   //   }

   //   bFinished = TRUE;
   //}


   pWnd->PostMessageW(WM_MSG_LOAD_FINISHED);
}

CDlgLoadSRTProgress::CDlgLoadSRTProgress(CWnd* pParent) : CDialogEx(IDD_DIALOG_PROGRESS, pParent)
{
   m_bAbortThread = FALSE;
   m_pThread = nullptr;

   m_nLoadingState = LOADING_INITIALIZE;

   m_strDescription = _T("Initializing...");

   m_fProgress = 0.f;

   m_nEncodeType = static_cast<int>(ENCODE_DEFAULT);

   m_nLineCount = 0;

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

   StartThread();

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

   rcProgress.right = rcProgress.left + static_cast<int>(static_cast<float>(nProgressWidth) * m_fProgress);

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

HRESULT CDlgLoadSRTProgress::PreLoadFile(CString strFilePath)
{
   //1. check file is valid
   if (strFilePath.IsEmpty() || !PathFileExists(strFilePath))
   {
      return E_FAIL;
   }

   // 2. try to get the encode info from the file
   FILE* pFile = nullptr;
   _wfopen_s(&pFile, strFilePath, TEXT("rb"));
   if (!pFile)
   {
      return E_FAIL;
   }

   fseek(pFile, 0, SEEK_END);
   long lSize = ftell(pFile);
   rewind(pFile);

   char* pBuffer = (char*)malloc(sizeof(char) * lSize);
   if (pBuffer == nullptr)
   {
      return E_FAIL;
   }

   size_t length = fread(pBuffer, 1, lSize, pFile);
   if (ferror(pFile) != 0 || length != lSize)
   {
      return E_FAIL;
   }

   if (length > 2)
   {
      switch (pBuffer[0])
      {
      case '\xEF':
         if (('\xBB' == pBuffer[1]) && ('\xBF' == pBuffer[2]))
         {
            m_nEncodeType = ENCODE_UTF8_BOM;
         }
         break;
      case '\xFE':
         if ('\xFF' == pBuffer[1])
         {
            m_nEncodeType = ENCODE_UTF16_BE_BOM;
         }
         break;
      case '\xFF':
         if ('\xFE' == pBuffer[1])
         {
            m_nEncodeType = ENCODE_UTF16_LE_BOM;
         }
         break;
      default:
         break;
      }
   }

   fclose(pFile);
   free(pBuffer);

   m_strFilePath = strFilePath;

   return CountLineCount();
}

HRESULT CDlgLoadSRTProgress::SetFilePath(CString strFilePath)
{
   m_strFilePath = strFilePath;

   return PreLoadFile(strFilePath);
}

void CDlgLoadSRTProgress::DecodeFileStream(std::wistream& fileStream)
{
   std::wstring wstr;

   LoadInit();

   CString strTimeInfo = _T("");
   CString strStratTimeInfo = _T("");
   CString strEndTimeInfo = _T("");
   CString strContent = _T("");

   int nLineCount = 0;
   int nDataState = LOAD_INITIALIZE;

   while (std::getline(fileStream, wstr))
   {
      ++nLineCount;
      SetLoadLineCount(nLineCount);

      if (GetAbortThread())
      {
         break;
      }

      if (LOAD_FAILED == nDataState)
      {
         break;
      }

      CString strLine = wstr.c_str();
      
      int nPos = strLine.ReverseFind(RETURN_SYMBOL);
      if (nPos >= 0)
      {
         strLine.Delete(nPos);
      }

      if (strLine.IsEmpty() && LOAD_CONTENT == nDataState)
      {
         if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !strContent.IsEmpty())
         {
            AddSRTData(strStratTimeInfo, strEndTimeInfo, strContent);

            strStratTimeInfo.Empty();
            strEndTimeInfo.Empty();
            strContent.Empty();

            nDataState = LOAD_INITIALIZE;
         }
         continue;
      }

      if (strLine.IsEmpty())
      {
         continue;
      }

      switch (nDataState)
      {
      case LOAD_INITIALIZE:
      {
         nDataState = LOAD_TIME_INFO;
      }
      break;
      case LOAD_TIME_INFO:
      {
         if (FAILED(GetTimeInfo(strLine, strStratTimeInfo, strEndTimeInfo)))
         {
            nDataState = LOAD_FAILED;
         }
         else
         {
            nDataState = LOAD_CONTENT;
         }
      }
      break;
      case LOAD_CONTENT:
      {
         if (!strContent.IsEmpty())
         {
            strContent += _T("\n");
         }
         strContent += strLine;
      }
      break;
      default:
         break;
      }
   }

   if (LOAD_FAILED == nDataState)
   {
      LoadFailed();
   }
   else
   {
      if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !strContent.IsEmpty())
      {
         AddSRTData(strStratTimeInfo, strEndTimeInfo, strContent);
      }

      LoadFinished();
   }
}

void CDlgLoadSRTProgress::DecodeUTF8()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);

   DecodeFileStream(wideFileStream);
}

void CDlgLoadSRTProgress::DecodeUTF8BOM()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);

   DecodeFileStream(wideFileStream);
}

void CDlgLoadSRTProgress::DecodeUTF16BE()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::generate_header>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);

   DecodeFileStream(wideFileStream);
}

void CDlgLoadSRTProgress::DecodeUTF16LE()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);

   DecodeFileStream(wideFileStream);
}

void CDlgLoadSRTProgress::DecodeFile()
{
   switch (m_nEncodeType)
   {
      case ENCODE_UTF8_BOM:
         DecodeUTF8BOM();
         break;
      case ENCODE_UTF16_BE_BOM:
         DecodeUTF16BE();
         break;
      case ENCODE_UTF16_LE_BOM:
         DecodeUTF16LE();
         break;
      default:
         DecodeUTF8();
         break;
   }
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

void CDlgLoadSRTProgress::LoadInit()
{
   CSRTDataManager::GetInstance()->ClearSRTData();

   m_nLoadingState = LOADING_INITIALIZE;
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

void CDlgLoadSRTProgress::SetLoadLineCount(int nCurLine)
{
   ASSERT(m_nLineCount > 0);

   m_nLoadingState = LOADING_PROGRESS;

   m_strDescription = _T("Loading...");

   m_fProgress = static_cast<float>(nCurLine) / static_cast<float>(m_nLineCount);

   PostMessage(WM_MSG_REFRESH);
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

HRESULT CDlgLoadSRTProgress::GetTimeInfo(CString strContent, CString & strStartTime, CString & strEndTime)
{
   CString strTimeInfo = strContent;

   if (strTimeInfo.IsEmpty())
   {
      return S_FALSE;
   }

   int nPos = strTimeInfo.Find(_T("-->"));
   if (nPos < 0)
   {
      return S_FALSE;
   }

   strStartTime = strTimeInfo.Left(nPos);
   strStartTime.Trim();

   if (strStartTime.IsEmpty())
   {
      return S_FALSE;
   }

   nPos += 3;
   int nLength = strTimeInfo.GetLength() - nPos;

   if (nLength <= 0)
   {
      return S_FALSE;
   }

   strEndTime = strTimeInfo.Mid(nPos, nLength);
   strEndTime.Trim();

   if (strEndTime.IsEmpty())
   {
      return S_FALSE;
   }

   return S_OK;
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

HRESULT CDlgLoadSRTProgress::CountLineCount()
{
   FILE* pFile = nullptr;
   _wfopen_s(&pFile, m_strFilePath, TEXT("rb"));
   if (!pFile)
   {
      return E_FAIL;
   }

   int nFlag = 0;
   int nLineCount = 0;

   while (!feof(pFile))
   {
      nFlag = fgetc(pFile);
      if (nFlag == '\n')
      {
         nLineCount++;
      }
   }

   fclose(pFile);

   nLineCount++;
   m_nLineCount = nLineCount;

   if (m_nLineCount > 0)
   {
      return S_OK;
   }

   return E_FAIL;
}
