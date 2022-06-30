#pragma once
#include <mutex>
#include <regex>

class CSRTDataManager
{
public:
   CSRTDataManager();
   ~CSRTDataManager();

   static CSRTDataManager* GetInstance();

   void AddSRTData(CString strStartTime, CString strEndTime, CString content);

   struct ColorInfo
   {
      COLORREF color;
      std::vector<std::pair<int, int>> vecIndex;
   };

   struct SRTData
   {
      CString startTime;
      CString endTime;
      CString content;

      std::vector<std::pair<int, int>> vecBoldInfo;
      std::vector<std::pair<int, int>> vecItalicInfo;
      std::vector<std::pair<int, int>> vecUnderlineInfo;

      std::vector<ColorInfo> vecColorInfo;
   };

   struct RemoveTagParams
   {
      CString content;
      std::wregex reInfo;
      CString strPreTag;
      CString strSufTag;
   };
   CString RemoveTag(RemoveTagParams params);

   CString RemoveBoldTag(CString const& content);
   CString RemoveItalicTag(CString const& content);
   CString RemoveUnderlineTag(CString const& content);

   CString RmoveColorTag(CString const& content);

   CString GetSRTDataStartTime(float fProgress);
   CString GetSRTDataEndTime(float fProgress);
   CString GetSRTDataContent(float fProgress);
   void GetSRTDataBoldInfo(float fProgress, std::vector<std::pair<int, int>>& vecBold);
   void GetSRTDataItalicInfo(float fProgress, std::vector<std::pair<int, int>>& vecItalic);
   void GetSRTDataUnderlineInfo(float fProgress, std::vector<std::pair<int, int>>& vecUnderline);
   void GetSRTDataColorInfo(float fProgress, std::vector<ColorInfo>& vecColor);

private:
   static std::unique_ptr<CSRTDataManager> m_pInstance;

   std::mutex m_mutexSRTData;

   struct ExtractInfoParams
   {
      CString content;
      std::wregex reInfo;
      CString strPreTag;
      CString strSufTag;
   };

   void ExtractInfo(ExtractInfoParams& info, std::vector<std::pair<int, int>>& vecRestlt);

   void ParseBoldInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt);
   void ParseItalicInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt);
   void ParseUnderlineInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt);

   void ExtractColorInfo(CString const& strContent, std::vector<ColorInfo>& vecRestlt);

   void ParseColorInfo(CString const& strContent, std::vector<ColorInfo>& vecRestlt);

   size_t GetRelatedIndex(float fProgress);

   std::vector<SRTData> m_vecSRTData;
};
