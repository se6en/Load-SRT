#pragma once
#include <mutex>
#include <regex>

class CSRTData
{
public:
   CSRTData() = default;
   ~CSRTData() = default;

   void SetContent(CString const& strContent);

   CString GetUnstyledContent();
   CString GetStyledContent();

   void SetStartTime(CString const& strTime);
   CString GetStartTime();

   void SetEndTime(CString const& strTime);
   CString GetEndTime();

   CString GetFormatTime();

   void SetBoldInfo(std::vector<std::pair<int, int>>& vecInfo);
   void GetBoldInfo(std::vector<std::pair<int, int>>& vecInfo);

   void SetItalicInfo(std::vector<std::pair<int, int>>& vecInfo);
   void GetItalicInfo(std::vector<std::pair<int, int>>& vecInfo);

   void SetUnderlineInfo(std::vector<std::pair<int, int>>& vecInfo);
   void GetUnderlineInfo(std::vector<std::pair<int, int>>& vecInfo);

   struct ColorInfo
   {
      COLORREF color;
      std::vector<std::pair<int, int>> vecIndex;
   };

   void SetColorInfo(std::vector<ColorInfo>& vecInfo);
   void GetColorInfo(std::vector<ColorInfo>& vecInfo);

private:
   CString m_strStartTime;
   CString m_strEndTime;
   CString m_strContent;

   std::vector<std::pair<int, int>> m_vecBoldInfo;
   std::vector<std::pair<int, int>> m_vecItalicInfo;
   std::vector<std::pair<int, int>> m_vecUnderlineInfo;

   std::vector<ColorInfo> m_vecColorInfo;
};

class CSRTDataManager
{
public:
   CSRTDataManager();
   ~CSRTDataManager();

   int ConvertUnicodeToUTF8CString(const wchar_t* pszUnicode, CStringA& strUTF8);

   static CSRTDataManager* GetInstance();

   void AddSRTData(CString strStartTime, CString strEndTime, CString content);

   BOOL HasValidData() const;

   int GetSRTDataCount();

   CString GetUnstyledContent(int nIndex);

   CString GetTime(int nIndex);

   struct InsertInfo
   {
      InsertInfo() {}
      InsertInfo(int nStart, int nLength)
         : start(nStart), length(nLength)
      {}
      int start;
      int length;
   };

   CString GetStyledContent(int nIndex);

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

   CString RemoveColorPrefixTag(CString const& content);
   CString RemoveColorSuffixTag(CString const& content);

   CString RemoveColorTag(CString const& content);

   CString GetSRTDataStartTime(float fProgress);
   CString GetSRTDataEndTime(float fProgress);
   CString GetSRTDataContent(float fProgress);
   void GetSRTDataBoldInfo(float fProgress, std::vector<std::pair<int, int>>& vecBold);
   void GetSRTDataItalicInfo(float fProgress, std::vector<std::pair<int, int>>& vecItalic);
   void GetSRTDataUnderlineInfo(float fProgress, std::vector<std::pair<int, int>>& vecUnderline);
   void GetSRTDataColorInfo(float fProgress, std::vector<CSRTData::ColorInfo>& vecColor);

   std::shared_ptr<CSRTData> GetData(float fProgree);

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

   BOOL ColorInfoTagExist(CString const& strContent);

   int ExtractColorTagInfoEndIndex(CString& strContent, int nStartIndex);

   struct colorTagInfo
   {
      colorTagInfo() {}
      colorTagInfo(CString strTag, int nStart, int nEnd)
         : tag(strTag), start(nStart), end(nEnd) {}
      CString tag;
      int start;
      int end;
   };
   void ExtractColorTagInfo(CString const& strContent, std::vector<colorTagInfo>& vecRestlt);
   
   void ExtractColorInfo(CString const& strContent, std::vector<CSRTData::ColorInfo>& vecRestlt);

   void ParseColorInfo(CString const& strContent, std::vector<CSRTData::ColorInfo>& vecRestlt);

   size_t GetRelatedIndex(float fProgress);

   std::vector<std::shared_ptr<CSRTData>> m_vecSRTData;
};
