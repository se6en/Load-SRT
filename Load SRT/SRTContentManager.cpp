#include "stdafx.h"
#include "SRTContentManager.h"

std::unique_ptr<CSRTContentManager> CSRTContentManager::m_pInstance = nullptr;

CSRTContentManager::CSRTContentManager()
{

}

CSRTContentManager::~CSRTContentManager()
{
   m_vecSRTContentData.clear();
}

CSRTContentManager * CSRTContentManager::GetInstance()
{
   if (m_pInstance == nullptr)
   {
      m_pInstance = std::unique_ptr<CSRTContentManager>(new CSRTContentManager());
   }
   return m_pInstance.get();
}

void CSRTContentManager::AddSRTContentData(CString strStartTime, CString strEndTime, CString strContent)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTContentData);

   m_vecSRTContentData.emplace_back(std::make_tuple(strStartTime, strEndTime, strContent));
}

std::tuple<CString, CString, CString> CSRTContentManager::GetSRTContentData(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTContentData);

   if (m_vecSRTContentData.empty())
   {
      return std::tuple<CString, CString, CString>();
   }

   size_t nCount = m_vecSRTContentData.size();

   size_t nIndex = min(static_cast<size_t>(static_cast<float>(nCount) * fProgress), nCount - 1);

   return m_vecSRTContentData[nIndex];
}

