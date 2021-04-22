#include "stdafx.h"
#include "SRTDataManager.h"

std::unique_ptr<CSRTDataManager> CSRTDataManager::m_pInstance = nullptr;

CSRTDataManager::CSRTDataManager()
{

}

CSRTDataManager::~CSRTDataManager()
{
   m_vecSRTData.clear();
}

CSRTDataManager * CSRTDataManager::GetInstance()
{
   if (m_pInstance == nullptr)
   {
      m_pInstance = std::unique_ptr<CSRTDataManager>(new CSRTDataManager());
   }

   return m_pInstance.get();
}

void CSRTDataManager::AddSRTData(CString strStartTime, CString strEndTime, CString strContent)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   m_vecSRTData.emplace_back(std::make_tuple(strStartTime, strEndTime, strContent));
}

std::tuple<CString, CString, CString> CSRTDataManager::GetSRTData(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   if (m_vecSRTData.empty())
   {
      return std::make_tuple(_T(""), _T(""), _T(""));
   }

   int nCount = m_vecSRTData.size();

   int nIndex = min((int)(nCount * fProgress), nCount - 1);

   return m_vecSRTData[nIndex];
}



