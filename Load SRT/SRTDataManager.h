#pragma once
#include <mutex>
#include <regex>

class CSRTDataManager
{
public:
   CSRTDataManager();
   ~CSRTDataManager();

   static CSRTDataManager* GetInstance();

   void AddSRTData(CString strStartTime, CString strEndTime, std::string content);

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
      std::string content;
      std::regex reInfo;
      std::string strPreTag;
      std::string strSufTag;
   };
   std::string RemoveTag(RemoveTagParams params);

   std::string RemoveBoldTag(std::string const& content);
   std::string RemoveItalicTag(std::string const& content);
   std::string RemoveUnderlineTag(std::string const& content);

   std::string RmoveColorTag(std::string const& content);

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
      std::string content;
      std::regex reInfo;
      std::string strPreTag;
      std::string strSufTag;
   };

   void ExtractInfo(ExtractInfoParams& info, std::vector<std::pair<int, int>>& vecRestlt);

   void ParseBoldInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt);
   void ParseItalicInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt);
   void ParseUnderlineInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt);

   void ExtractColorInfo(std::string const& content, std::vector<ColorInfo>& vecRestlt);

   void ParseColorInfo(std::string const& content, std::vector<ColorInfo>& vecRestlt);

   size_t GetRelatedIndex(float fProgress);

   std::vector<SRTData> m_vecSRTData;
};
