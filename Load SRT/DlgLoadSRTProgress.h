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
   void SetFilePath(CString strFilePath);
   CString GetFilePath() { return m_strFilePath; };

   void SetAbortThread(BOOL bAbort) { m_bAbortThread = bAbort; };
   BOOL GetAbortThread() { return m_bAbortThread; };

   HRESULT GetSRTFileLineCount(int& nLineCount);

   void LoadSRTData();

   void OpenFileFailed();
   void CountingLines();
   void LoadingFile(float fProgress);
   void LoadFailed();
   void LoadFinished();

   CString ConvertStringToUnicodeCString(const char* pszUTF8);

   void GetTimeInfo(const char* pszUTF8, CString& strStartTime, CString& strEndTime);

   void AddSRTData(CString strStartTime, CString strEndTime, CString strContent);

private:
   void StartThread();
   void AbortThread();

private:
   CString                          m_strFilePath;

   std::atomic<BOOL>                m_bAbortThread;
   std::unique_ptr<std::thread>     m_pThread;

   int                              m_nLoadingState;

   CString                          m_strDescription;
   float                            m_fProgress;

   CFont*                           m_pFont;

};
