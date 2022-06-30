#pragma once

#include <atomic>
#include <thread>
#include <mutex>

enum LOAD_SRT_DATA_STATE
{
   LOAD_INITIALIZE = 0,
   LOAD_NUMBER,
   LOAD_TIME_INFO,
   LOAD_CONTENT,
   LOAD_FAILED
};

enum LOADING_STATUS
{
   LOADING_INITIALIZE = 0,
   LOADING_OPEN_FAILED,
   LOADING_COUNT_LINES,
   LOADING_FAILED_READ_HEADER,
   LOADING_PROGRESS,
   LOADING_SUCEESS
};

enum ENCODE_TYPE
{
   ENCODE_DEFAULT,
   ENCODE_UTF8_BOM,
   ENCODE_UTF16_BE_BOM,
   ENCODE_UTF16_LE_BOM
};

class CDlgLoadSRTProgress : public CDialogEx
{
   DECLARE_DYNAMIC(CDlgLoadSRTProgress)

public:
   CDlgLoadSRTProgress(CWnd* pParent = NULL);
   ~CDlgLoadSRTProgress();

protected:
   virtual void DoDataExchange(CDataExchange* pDX);
   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()

   afx_msg BOOL OnEraseBkgnd(CDC* pDC);

   afx_msg void OnPaint();

   afx_msg void OnClose();

   LRESULT OnRefreshMsg(WPARAM wParam, LPARAM lParam);
   LRESULT OnLoadFinished(WPARAM wParam, LPARAM lParam);

public:
   HRESULT PreLoadFile(CString strFilePath);

   HRESULT SetFilePath(CString strFilePath);
   CString GetFilePath() { return m_strFilePath; };

   void DecodeFileStream(std::wistream& fileStream);

   void DecodeUTF8();
   void DecodeUTF8BOM();
   void DecodeUTF16BE();
   void DecodeUTF16LE();

   void DecodeFile();

   void SetAbortThread(BOOL bAbort) { m_bAbortThread = bAbort; };
   BOOL GetAbortThread() { return m_bAbortThread; };

   HRESULT GetSRTFileLineCount(int& nLineCount);

   void LoadSRTData();

   void LoadInit();
   void OpenFileFailed();
   void CountingLines();
   void SetLoadLineCount(int nCurLine);
   void LoadingFile(float fProgress);
   void LoadFailed();
   void LoadFinished();

   CString ConvertStringToUnicodeCString(const char* pszUTF8);

   HRESULT GetTimeInfo(CString strContent, CString& strStartTime, CString& strEndTime);

   void AddSRTData(CString strStartTime, CString strEndTime, CString strContent);

private:
   void StartThread();
   void AbortThread();

   HRESULT CountLineCount();

private:
   CString                          m_strFilePath;

   std::atomic<BOOL>                m_bAbortThread;
   std::unique_ptr<std::thread>     m_pThread;

   int                              m_nLoadingState;

   CString                          m_strDescription;
   float                            m_fProgress;

   CFont*                           m_pFont;

   int m_nEncodeType;
   int m_nLineCount;
};
