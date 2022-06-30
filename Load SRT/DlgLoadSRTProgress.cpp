#include "stdafx.h"
#include "resource.h"
#include "DlgLoadSRTProgress.h"
#include <codecvt>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cwchar>
#include <cstdlib>
#include "SRTDataManager.h"
#include "SRTContentManager.h"

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

      //std::fstream fileStream;
      //fileStream.open(strFilePath, std::ios::in | std::ios::binary);

      ///*std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>> convert(fileStream.rdbuf());

      //std::wistream wideFileStream(&convert);*/
      //std::string wstr;
      //while (std::getline(fileStream, wstr))
      //{
      //   std::string result;
      //   WCHAR *strSrc;
      //   LPSTR szRes;

      //   int i = MultiByteToWideChar(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0);
      //   strSrc = new WCHAR[i + 1];
      //   MultiByteToWideChar(CP_UTF8, 0, wstr.c_str(), -1, strSrc, i);

      //   i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
      //   szRes = new CHAR[i + 1];
      //   WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

      //   result = szRes;
      //   delete[]strSrc;
      //   delete[]szRes;

      //   /*std::wstring strCorrect = wstr.substr(3);
      //   std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt2;
      //   std::string u8str = cvt2.to_bytes(wstr);*/
      //}

      int nState = LOAD_FAILED;

      // 2. Get line count
      int nLineCount = 0;
      HRESULT hResult = pWnd->GetSRTFileLineCount(nLineCount);

      if (FAILED(hResult))
      {
         pWnd->OpenFileFailed();
         break;
      }

      //std::fstream fileStream;
      //fileStream.open(strFilePath, std::ios::in | std::ios::binary);

      ////std::wbuffer_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::generate_header/*little_endian*/>> convert(fileStream.rdbuf());
      //std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header/*little_endian*/>> convert(fileStream.rdbuf());

      //std::wistream wideFileStream(&convert);
      //std::wstring wstr;
      //while (std::getline(wideFileStream, wstr))
      //{
      //   std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt2;
      //   std::string u8str = cvt2.to_bytes(wstr);
      //}


      std::ifstream fileSRT;
      //auto locUTF8 = std::locale(std::locale(""), new std::codecvt<wchar_t, char, std::mbstate_t>);
      //fileSRT.imbue(locUTF8);

      fileSRT.open(strFilePath);

      if (fileSRT.fail())
      {
         fileSRT.close();
         fileSRT.clear();
         break;
      }

      nState = LOAD_INITIALIZE;
      
      CString strTimeInfo = _T("");
      CString strStratTimeInfo = _T("");
      CString strEndTimeInfo = _T("");
      std::string content = "";

      int nCurLine = 0;

      // 3. begin to read file content
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

         if (nState == LOAD_FAILED)
         {
            break;
         }

         // checked several applications that an extra line without any content will be ignored directly
         if (strLine.empty())
         {
            if (nState == LOAD_CONTENT)
            {
               if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !content.empty())
               {
                  pWnd->AddSRTData(strStratTimeInfo, strEndTimeInfo, content);

                  strStratTimeInfo.Empty();
                  strEndTimeInfo.Empty();
                  content = "";

                  nState = LOAD_INITIALIZE;
               }
            }
            continue;
         }

         switch (nState)
         {
            case LOAD_INITIALIZE:
            {
               nState = LOAD_TIME_INFO;
            }
            break;
            case LOAD_TIME_INFO:
            {
               if (FAILED(pWnd->GetTimeInfo(strLine.c_str(), strStratTimeInfo, strEndTimeInfo)))
               {
                  nState = LOAD_FAILED;
               }
               else
               {
                  nState = LOAD_CONTENT;
               }
            }
            break;
            case LOAD_CONTENT:
            {
               if (!content.empty())
               {
                  content += "\n";
               }
               content += strLine;
            }
            break;
            default:
               break;
         }
      }

      fileSRT.close();
      fileSRT.clear();

      if (nState == LOAD_FAILED)
      {
         pWnd->LoadFailed();
      }
      else
      {
         if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !content.empty())
         {
            pWnd->AddSRTData(strStratTimeInfo, strEndTimeInfo, content);
         }

         pWnd->LoadingFile((float)nCurLine / (float)nLineCount);

         pWnd->LoadFinished();
      }

      bFinished = TRUE;
   }


   pWnd->PostMessageW(WM_MSG_LOAD_FINISHED);
}

void LoadUTF16SRTThread(LPVOID lpData)
{
   CDlgLoadSRTProgress* pWnd = (CDlgLoadSRTProgress*)lpData;

   BOOL bFinished = FALSE;

   while (!pWnd->GetAbortThread() && !bFinished)
   {
      CString strFilePath = pWnd->GetFilePath();

      int nState = LOAD_FAILED;

      // Get line count
      int nLineCount = 0;
      HRESULT hResult = pWnd->GetSRTFileLineCount(nLineCount);

      if (FAILED(hResult))
      {
         pWnd->OpenFileFailed();
         break;
      }

      /*FILE* pFile = nullptr;
      _wfopen_s(&pFile, strFilePath, TEXT("rb"));
      if (!pFile)
      {
         break;
      }

      fseek(pFile, 0, SEEK_END);
      long lSize = ftell(pFile);
      rewind(pFile);

      char* pBuffer = (char*)malloc(sizeof(char) * lSize);
      if (pBuffer == nullptr)
      {
         break;
      }

      size_t length = fread(pBuffer, 1, lSize, pFile);
      if (ferror(pFile) != 0 || length != lSize)
      {
         break;
      }

      pBuffer += 2;*/

      /*unsigned short nCurCharacter = 0;

      while (!pWnd->GetAbortThread())
      {
         wchar_t nCur = static_cast<unsigned short>(*pBuffer++ << 8);
         nCur |= *pBuffer;

         ++pBuffer;
         strCur += nCur;

      }*/

      /*fclose(pFile);
      free(pBuffer);*/

      nState = LOAD_INITIALIZE;

      CString strTimeInfo = _T("");
      CString strStratTimeInfo = _T("");
      CString strEndTimeInfo = _T("");
      CString strContent = _T("");

      int nCurLine = 0;

      std::ifstream fileSRT;
      fileSRT.open(strFilePath);

      // 3. begin to read file content
      std::string strLine;
      int nOffset = 2;        // the default offset of BOM need to apply to the first line
      while (std::getline(fileSRT, strLine))
      {
         ++nCurLine;

         CString strCurLineInfo = _T("");
         strCurLineInfo.Format(_T("\r\n ----- current line : %d ------"), nCurLine);
         OutputDebugString(strCurLineInfo);

         if (pWnd->GetAbortThread())
         {
            break;
         }

         if (nState == LOAD_FAILED)
         {
            break;
         }

         CString strCurLine;

         std::string::iterator iter = strLine.begin() + nOffset;

         while (iter != strLine.end())
         {
            unsigned short nCur = static_cast<unsigned short>(*iter++ << 8);

            if (iter == strLine.end())
            {
               break;
            }

            nCur |= *iter;

            wchar_t charCur = static_cast<wchar_t>(nCur);
            int cBuf = WideCharToMultiByte(CP_UTF8, 0, &charCur, -1, NULL, 0, NULL, NULL);
            char *buf = new char[cBuf];
            cBuf = WideCharToMultiByte(CP_UTF8, 0, &charCur, -1, buf, 1024, NULL, NULL);

            strCurLine += static_cast<wchar_t>(nCur);
            ++iter;
         }

         OutputDebugString(strCurLine);

         // reset offset
         nOffset = 0;

         if (strCurLine.IsEmpty())
         {
            if (nState == LOAD_CONTENT)
            {
               if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !strContent.IsEmpty())
               {
                  pWnd->AddSRTContentData(strStratTimeInfo, strEndTimeInfo, strContent);

                  strStratTimeInfo.Empty();
                  strEndTimeInfo.Empty();
                  strContent.Empty();

                  nState = LOAD_INITIALIZE;
               }
            }
            continue;
         }

         switch (nState)
         {
         case LOAD_INITIALIZE:
         {
            nState = LOAD_TIME_INFO;
         }
         break;
         case LOAD_TIME_INFO:
         {
            if (FAILED(pWnd->GetContentTimeInfo(strCurLine, strStratTimeInfo, strEndTimeInfo)))
            {
               nState = LOAD_FAILED;
            }
            else
            {
               nState = LOAD_CONTENT;
            }
         }
         break;
         case LOAD_CONTENT:
         {
            if (!strContent.IsEmpty())
            {
               strContent += _T("\n");
            }
            strContent += strCurLine;
         }
         break;
         default:
            break;
         }
      }

      fileSRT.close();
      fileSRT.clear();

      if (nState == LOAD_FAILED)
      {
         pWnd->LoadFailed();
      }
      else
      {
         if (!strStratTimeInfo.IsEmpty() && !strEndTimeInfo.IsEmpty() && !strContent.IsEmpty())
         {
            pWnd->AddSRTContentData(strStratTimeInfo, strEndTimeInfo, strContent);
         }

         pWnd->LoadingFile((float)nCurLine / (float)nLineCount);

         pWnd->LoadFinished();
      }

      bFinished = TRUE;
   }


   pWnd->PostMessageW(WM_MSG_LOAD_FINISHED);
          
          //while (true/*pLineBuffe[0] != '\0'*/)
          //{
          //   wchar_t nCur = static_cast<unsigned short>(*pLineBuffe++ << 8);

          //   nCur |= *pLineBuffe;

          //   ++pLineBuffe;
          //   strCur += nCur;
          //}



      /*fclose(pFile);
      free(pBuffer);*/

}

CDlgLoadSRTProgress::CDlgLoadSRTProgress(CWnd* pParent) : CDialogEx(IDD_DIALOG_PROGRESS, pParent)
{
   m_bAbortThread = FALSE;
   m_pThread = nullptr;

   m_nLoadingState = LOADING_INITIALIZE;

   m_strDescription = _T("Initializing...");

   m_fProgress = 0.f;

   m_nEncodeType = static_cast<int>(ENCODE_DEFAULT);

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
   StartThread();
 
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

   return S_OK;
}

void CDlgLoadSRTProgress::DecodeUTF8()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);
   std::wstring wstr;
   while (std::getline(wideFileStream, wstr))
   {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt2;
      std::string u8str = cvt2.to_bytes(wstr);
   }
}

void CDlgLoadSRTProgress::DecodeUTF8BOM()
{
   std::fstream fileStream;
   fileStream.open(m_strFilePath, std::ios::in | std::ios::binary);

   std::wbuffer_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>> convert(fileStream.rdbuf());

   std::wistream wideFileStream(&convert);
   std::wstring wstr;
   while (std::getline(wideFileStream, wstr))
   {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt2;
      std::string u8str = cvt2.to_bytes(wstr);
   }
}

void CDlgLoadSRTProgress::DecodeUTF16BE()
{

}

void CDlgLoadSRTProgress::DecodeUTF16LE()
{

}

void CDlgLoadSRTProgress::DecodeFile()
{

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

HRESULT CDlgLoadSRTProgress::GetTimeInfo(const char * pszUTF8, CString & strStartTime, CString & strEndTime)
{
   CString strTimeInfo = ConvertStringToUnicodeCString(pszUTF8);

   if (strTimeInfo.IsEmpty())
   {
      return E_FAIL;
   }

   int nPos = strTimeInfo.Find(_T("-->"));
   if (nPos < 0)
   {
      return E_FAIL;
   }

   strStartTime = strTimeInfo.Left(nPos);
   strStartTime.Trim();

   if (strStartTime.IsEmpty())
   {
      return E_FAIL;
   }

   nPos += 3;
   int nLength = strTimeInfo.GetLength() - nPos;

   if (nLength <= 0)
   {
      return E_FAIL;
   }

   strEndTime = strTimeInfo.Mid(nPos, nLength);
   strEndTime.Trim();

   if (strEndTime.IsEmpty())
   {
      return E_FAIL;
   }

   return S_OK;
}

HRESULT CDlgLoadSRTProgress::GetContentTimeInfo(CString const & strTimeInfo, CString & strStartTime, CString & strEndTime)
{
   if (strTimeInfo.IsEmpty())
   {
      return E_FAIL;
   }

   int nPos = strTimeInfo.Find(_T("-->"));
   if (nPos < 0)
   {
      return E_FAIL;
   }

   strStartTime = strTimeInfo.Left(nPos);
   strStartTime.Trim();

   if (strStartTime.IsEmpty())
   {
      return E_FAIL;
   }

   nPos += 3;
   int nLength = strTimeInfo.GetLength() - nPos;

   if (nLength <= 0)
   {
      return E_FAIL;
   }

   strEndTime = strTimeInfo.Mid(nPos, nLength);
   strEndTime.Trim();

   if (strEndTime.IsEmpty())
   {
      return E_FAIL;
   }

   return S_OK;
}

void CDlgLoadSRTProgress::AddSRTData(CString strStartTime, CString strEndTime, std::string content)
{
   CSRTDataManager::GetInstance()->AddSRTData(strStartTime, strEndTime, content);
}

void CDlgLoadSRTProgress::AddSRTContentData(CString strStartTime, CString strEndTime, CString strContent)
{
   CSRTContentManager::GetInstance()->AddSRTContentData(strStartTime, strEndTime, strContent);
}

void CDlgLoadSRTProgress::StartThread()
{
   if (m_pThread != nullptr)
   {
      return;
   }

   if (m_nEncodeType == ENCODE_DEFAULT || m_nEncodeType == ENCODE_UTF8_BOM)
   {
      m_pThread = std::make_unique<std::thread>(LoadSRTThread, (LPVOID)this);
   }
   else
   {
      m_pThread = std::make_unique<std::thread>(LoadUTF16SRTThread, (LPVOID)this);
   }
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
