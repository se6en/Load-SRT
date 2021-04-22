#pragma once
#include <mutex>

class CSRTDataManager
{
public:
   CSRTDataManager();
   ~CSRTDataManager();

   static CSRTDataManager* GetInstance();

   void AddSRTData(CString strStartTime, CString strEndTime, CString strContent);

   std::tuple<CString, CString, CString> GetSRTData(float fProgress);

private:
   static std::unique_ptr<CSRTDataManager>                                         m_pInstance;

   std::mutex                                                                      m_mutexSRTData;
   std::vector<std::tuple<CString, CString, CString>>                              m_vecSRTData;
};
