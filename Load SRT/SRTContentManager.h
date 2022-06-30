#pragma once
#include <mutex>

class CSRTContentManager
{
public:
   CSRTContentManager();
   ~CSRTContentManager();

   static CSRTContentManager* GetInstance();

   void AddSRTContentData(CString strStartTime, CString strEndTime, CString strContent);

   std::tuple<CString, CString, CString> GetSRTContentData(float fProgress);

private:
   static std::unique_ptr<CSRTContentManager> m_pInstance;

   std::mutex m_mutexSRTContentData;
   std::vector<std::tuple<CString, CString, CString>> m_vecSRTContentData;
};
