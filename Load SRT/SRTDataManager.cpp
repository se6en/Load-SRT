#include "stdafx.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include "SRTDataManager.h"

static constexpr auto BOLD_TAG_PREFIX = "<b>";
static constexpr auto BOLD_TAG_SUFFIX = "</b>";
static const std::regex boldTag("<b>(.*)</b>");

static constexpr auto ITALIC_TAG_PREFIX = "<i>";
static constexpr auto ITALIC_TAG_SUFFIX = "</i>";
static const std::regex italicTag("<i>(.*)</i>");

static constexpr auto UNDERLINE_TAG_PREFIX = "<u>";
static constexpr auto UNDERLINE_TAG_SUFFIX = "</u>";
static const std::regex underlineTag("<u>(.*)</u>");

static const std::regex ColorTag("<font color=(.*)>(.*)</font>");

int ConvertUnicodeToUTF8CString(const wchar_t* pszUnicode, CStringA& strUTF8)
{
   // determine the size of buffer required then allocate it
   int nStrLen = WideCharToMultiByte(CP_UTF8, 0, pszUnicode, -1, NULL, 0, NULL, NULL);
   if (nStrLen == 0)
   {
      return 0;
   }

   // do the conversion
   int nRet = WideCharToMultiByte(CP_UTF8, 0, pszUnicode, -1, strUTF8.GetBuffer(nStrLen), nStrLen, NULL, NULL);
   strUTF8.ReleaseBuffer();

   return nRet;
}

CString ConvertStringToUnicodeCString(const char * pszUTF8)
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

struct NamedColor { const char *const name; const COLORREF color; };

const NamedColor namedColors[] = {
    { "transparent", RGB(0, 0, 0) },
    { "aliceblue", RGB(240, 248, 255) },
    { "antiquewhite", RGB(250, 235, 215) },
    { "aqua", RGB(0, 255, 255)},
    { "aquamarine", RGB(127, 255, 212) },
    { "azure", RGB(240, 255, 255) },
    { "beige", RGB(245, 245, 220) },
    { "bisque", RGB(255, 228, 196) },
    { "black", RGB(0, 0, 0) },
    { "blanchedalmond", RGB(255, 235, 205) },
    { "blue", RGB(0, 0, 255) },
    { "blueviolet", RGB(138, 43, 226) },
    { "brown", RGB(165, 42, 42) },
    { "burlywood", RGB(222, 184, 135) },
    { "cadetblue", RGB(95, 158, 160) },
    { "chartreuse", RGB(127, 255, 0) },
    { "chocolate", RGB(210, 105, 30) },
    { "coral", RGB(255, 127, 80) },
    { "cornflowerblue", RGB(100, 149, 237) },
    { "cornsilk", RGB(255, 248, 220) },
    { "crimson", RGB(220, 20, 60) },
    { "cyan", RGB(0, 255, 255) },
    { "darkblue", RGB(0, 0, 139) },
    { "darkcyan", RGB(0, 139, 139) },
    { "darkgoldenrod", RGB(184, 134, 11) },
    { "darkgray", RGB(169, 169, 169) },
    { "darkgreen", RGB(0, 100, 0) },
    { "darkgrey", RGB(169, 169, 169) },
    { "darkkhaki", RGB(189, 183, 107) },
    { "darkmagenta", RGB(139, 0, 139) },
    { "darkolivegreen", RGB(85, 107, 47) },
    { "darkorange", RGB(255, 140, 0) },
    { "darkorchid", RGB(153, 50, 204) },
    { "darkred", RGB(139, 0, 0) },
    { "darksalmon", RGB(233, 150, 122) },
    { "darkseagreen", RGB(143, 188, 143) },
    { "darkslateblue", RGB( 72, 61, 139)},
    { "darkslategray", RGB( 47, 79, 79)},
    { "darkslategrey", RGB( 47, 79, 79)},
    { "darkturquoise", RGB( 0, 206, 209)},
    { "darkviolet", RGB( 148, 0, 211)},
    { "deeppink", RGB( 255, 20, 147)},
    { "deepskyblue", RGB( 0, 191, 255)},
    { "dimgray", RGB( 105, 105, 105)},
    { "dimgrey", RGB( 105, 105, 105)},
    { "dodgerblue", RGB( 30, 144, 255)},
    { "firebrick", RGB( 178, 34, 34)},
    { "floralwhite", RGB( 255, 250, 240)},
    { "forestgreen", RGB( 34, 139, 34)},
    { "fuchsia", RGB( 255, 0, 255)},
    { "gainsboro", RGB( 220, 220, 220)},
    { "ghostwhite", RGB( 248, 248, 255)},
    { "gold", RGB( 255, 215, 0)},
    { "goldenrod", RGB( 218, 165, 32)},
    { "gray", RGB( 128, 128, 128)},
    { "green", RGB( 0, 128, 0)},
    { "greenyellow", RGB( 173, 255, 47)},
    { "grey", RGB( 128, 128, 128)},
    { "honeydew", RGB( 240, 255, 240)},
    { "hotpink", RGB( 255, 105, 180)},
    { "indianred", RGB( 205, 92, 92)},
    { "indigo", RGB( 75, 0, 130)},
    { "ivory", RGB( 255, 255, 240)},
    { "khaki", RGB( 240, 230, 140)},
    { "lavender", RGB( 230, 230, 250)},
    { "lavenderblush", RGB( 255, 240, 245)},
    { "lawngreen", RGB( 124, 252, 0)},
    { "lemonchiffon", RGB( 255, 250, 205)},
    { "lightblue", RGB( 173, 216, 230)},
    { "lightcoral", RGB( 240, 128, 128)},
    { "lightcyan", RGB( 224, 255, 255)},
    { "lightgoldenrodyellow", RGB( 250, 250, 210)},
    { "lightgray", RGB( 211, 211, 211)},
    { "lightgreen", RGB( 144, 238, 144)},
    { "lightgrey", RGB( 211, 211, 211)},
    { "lightpink", RGB( 255, 182, 193)},
    { "lightsalmon", RGB( 255, 160, 122)},
    { "lightseagreen", RGB( 32, 178, 170)},
    { "lightskyblue", RGB( 135, 206, 250)},
    { "lightslategray", RGB( 119, 136, 153)},
    { "lightslategrey", RGB( 119, 136, 153)},
    { "lightsteelblue", RGB( 176, 196, 222)},
    { "lightyellow", RGB( 255, 255, 224)},
    { "lime", RGB( 0, 255, 0)},
    { "limegreen", RGB( 50, 205, 50)},
    { "linen", RGB( 250, 240, 230)},
    { "magenta", RGB( 255, 0, 255)},
    { "maroon", RGB( 128, 0, 0)},
    { "mediumaquamarine", RGB( 102, 205, 170)},
    { "mediumblue", RGB( 0, 0, 205)},
    { "mediumorchid", RGB( 186, 85, 211)},
    { "mediumpurple", RGB( 147, 112, 219)},
    { "mediumseagreen", RGB( 60, 179, 113)},
    { "mediumslateblue", RGB( 123, 104, 238)},
    { "mediumspringgreen", RGB( 0, 250, 154)},
    { "mediumturquoise", RGB( 72, 209, 204)},
    { "mediumvioletred", RGB( 199, 21, 133)},
    { "midnightblue", RGB( 25, 25, 112)},
    { "mintcream", RGB( 245, 255, 250)},
    { "mistyrose", RGB( 255, 228, 225)},
    { "moccasin", RGB( 255, 228, 181)},
    { "navajowhite", RGB( 255, 222, 173)},
    { "navy", RGB( 0, 0, 128)},
    { "oldlace", RGB( 253, 245, 230)},
    { "olive", RGB( 128, 128, 0)},
    { "olivedrab", RGB( 107, 142, 35)},
    { "orange", RGB( 255, 165, 0)},
    { "orangered", RGB( 255, 69, 0)},
    { "orchid", RGB( 218, 112, 214)},
    { "palegoldenrod", RGB( 238, 232, 170)},
    { "palegreen", RGB( 152, 251, 152)},
    { "paleturquoise", RGB( 175, 238, 238)},
    { "palevioletred", RGB( 219, 112, 147)},
    { "papayawhip", RGB( 255, 239, 213)},
    { "peachpuff", RGB( 255, 218, 185)},
    { "peru", RGB( 205, 133, 63)},
    { "pink", RGB( 255, 192, 203)},
    { "plum", RGB( 221, 160, 221)},
    { "powderblue", RGB( 176, 224, 230)},
    { "purple", RGB( 128, 0, 128)},
    { "red", RGB( 255, 0, 0)},
    { "rosybrown", RGB( 188, 143, 143)},
    { "royalblue", RGB( 65, 105, 225)},
    { "saddlebrown", RGB( 139, 69, 19)},
    { "salmon", RGB( 250, 128, 114)},
    { "sandybrown", RGB( 244, 164, 96)},
    { "seagreen", RGB( 46, 139, 87)},
    { "seashell", RGB( 255, 245, 238)},
    { "sienna", RGB( 160, 82, 45)},
    { "silver", RGB( 192, 192, 192)},
    { "skyblue", RGB( 135, 206, 235)},
    { "slateblue", RGB( 106, 90, 205)},
    { "slategray", RGB( 112, 128, 144)},
    { "slategrey", RGB( 112, 128, 144)},
    { "snow", RGB( 255, 250, 250)},
    { "springgreen", RGB( 0, 255, 127)},
    { "steelblue", RGB( 70, 130, 180)},
    { "tan", RGB( 210, 180, 140)},
    { "teal", RGB( 0, 128, 128)},
    { "thistle", RGB( 216, 191, 216)},
    { "tomato", RGB( 255, 99, 71)},
    { "turquoise", RGB( 64, 224, 208)},
    { "violet", RGB( 238, 130, 238)},
    { "wheat", RGB( 245, 222, 179)},
    { "white", RGB( 255, 255, 255)},
    { "whitesmoke", RGB( 245, 245, 245)},
    { "yellow", RGB( 255, 255, 0)},
    { "yellowgreen", RGB( 154, 205, 50)}
};

template <typename T>
int ClampValue(T value)
{
   value = static_cast<T>(::round(value));
   return value < 0 ? 0 : value > 255 ? 255 : static_cast<int>(value);
}

HRESULT GetColorFromHex(std::string const& content, COLORREF& color)
{
   if (content.front() != '#')
   {
      return E_FAIL;
   }

   std::string hexColor = content.substr(1, 6);
   int64_t hexValue = strtoll(hexColor.c_str(), nullptr, 16);
   if (hexValue < 0 || hexValue > 0xffffff)
   {
      return E_FAIL;
   }

   int nRed = static_cast<int>((hexValue & 0xff0000) >> 16);
   int nGreen = static_cast<int>((hexValue & 0xff00) >> 8);
   int nBlue = static_cast<int>((hexValue & 0xff));

   color = RGB(nRed, nGreen, nBlue);

   return S_OK;
}

HRESULT GetColorFromRGB(std::string const& content, COLORREF& color)
{
   if (content.find("rgb") == std::string::npos)
   {
      return E_FAIL;
   }

   size_t nStart = content.find_first_of('(');
   size_t nEnd = content.find_first_of(')');

   if (nStart == std::string::npos || nEnd == std::string::npos || nEnd + 1 != content.length())
   {
      return E_FAIL;
   }

   std::string rgbColor = content.substr(nStart + 1, nEnd - (nStart + 1));
   
   std::stringstream infostream(rgbColor);
   std::vector<std::string> vecValue;
   std::string item;

   while (std::getline(infostream, item, ','))
   {
      vecValue.push_back(item);
   }

   if (vecValue.size() != 3)
   {
      return E_FAIL;
   }

   int nRed = ClampValue(strtoll(vecValue[0].c_str(), nullptr, 10));
   int nGreen = ClampValue(strtoll(vecValue[1].c_str(), nullptr, 10));
   int nBlue = ClampValue(strtoll(vecValue[2].c_str(), nullptr, 10));

   color = RGB(nRed, nGreen, nBlue);
   return S_OK;
}

HRESULT GetColorFromName(std::string const& content, COLORREF& color)
{
   for (const auto& item : namedColors)
   {
      if (content.compare(item.name) == 0)
      {
         color = item.color;
         return S_OK;
      }
   }

   return E_FAIL;
}

HRESULT GetColor(std::string const& content, COLORREF& color)
{
   if (content.empty())
   {
      return E_FAIL;
   }

   // check if it is hex color
   if (SUCCEEDED(GetColorFromHex(content, color)))
   {
      return S_OK;
   }

   // check if it is rgb value
   if (SUCCEEDED(GetColorFromRGB(content, color)))
   {
      return S_OK;
   }

   return GetColorFromName(content, color);
}

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

std::string CSRTDataManager::RemoveTag(RemoveTagParams params)
{
   if (params.content.find(params.strPreTag) == std::string::npos || params.content.find(params.strSufTag) == std::string::npos)
   {
      return params.content;
   }

   std::string content = params.content;

   std::smatch result;
   if (std::regex_search(content, result, params.reInfo))
   {
      size_t nPos = content.find(params.strPreTag);
      size_t nLength = params.strPreTag.length();
      content.erase(nPos, nLength);

      nPos = content.find(params.strSufTag);
      nLength = params.strSufTag.length();
      content.erase(nPos, nLength);

      params.content = content;

      content = RemoveTag(params);
   }

   return content;
}

std::string CSRTDataManager::RemoveBoldTag(std::string const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = boldTag;
   params.strPreTag = BOLD_TAG_PREFIX;
   params.strSufTag = BOLD_TAG_SUFFIX;

   return RemoveTag(params);
}

std::string CSRTDataManager::RemoveItalicTag(std::string const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = italicTag;
   params.strPreTag = ITALIC_TAG_PREFIX;
   params.strSufTag = ITALIC_TAG_SUFFIX;

   return RemoveTag(params);
}

std::string CSRTDataManager::RemoveUnderlineTag(std::string const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = underlineTag;
   params.strPreTag = UNDERLINE_TAG_PREFIX;
   params.strSufTag = UNDERLINE_TAG_SUFFIX;

   return RemoveTag(params);
}

std::string CSRTDataManager::RmoveColorTag(std::string const& strContent)
{
   std::string content = strContent;
   std::cmatch m;

   auto ret = std::regex_search(content.c_str(), m, ColorTag);

   if (ret)
   {
      size_t nPosition = m.position();
      ASSERT(m.size() > 1);
      std::string colortag = m[1];
      size_t colorTagSize = colortag.length();

      size_t nStart = nPosition;
      size_t nLength = colorTagSize;
      content.erase(nPosition, nLength + 13); // add extra length for <font color=>

      nPosition = content.find("</font>");
      size_t nEndIndex = nPosition - 1;
      content.erase(nPosition, 7); // strength of </font>

      content = RmoveColorTag(content);
   }

   return content;
}

void CSRTDataManager::ExtractInfo(ExtractInfoParams& info, std::vector<std::pair<int, int>>& vecResult)
{
   std::string content = info.content;

   std::smatch result;
   if (std::regex_search(content, result, info.reInfo))
   {
      size_t nPos = content.find(info.strPreTag);
      size_t nStartIndex = nPos;
      size_t nLength = info.strPreTag.length();
      content.erase(nPos, nLength);

      nPos = content.find(info.strSufTag);
      size_t nEndIndex = nPos - 1;
      nLength = info.strSufTag.length();
      content.erase(nPos, nLength);

      vecResult.push_back(std::make_pair(static_cast<int>(nStartIndex), static_cast<int>(nEndIndex)));

      info.content = content;

      ExtractInfo(info, vecResult);
   }
}

void CSRTDataManager::ParseBoldInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt)
{
   std::string boldcontent = content;

   boldcontent = RemoveItalicTag(boldcontent);
   boldcontent = RemoveUnderlineTag(boldcontent);
   boldcontent = RmoveColorTag(boldcontent);

   ExtractInfoParams params;
   params.content = boldcontent;
   params.reInfo = boldTag;
   params.strPreTag = BOLD_TAG_PREFIX;
   params.strSufTag = BOLD_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);

   //std::smatch result;
   //if (std::regex_search(content, result, boldTag))
   //{
   //   // find "<b>*</b>", extract it first
   //   int nPos = strCurContent.Find(_T("<b>"));
   //   int nStartIndex = nPos;
   //   strCurContent.Delete(nPos, 3);

   //   nPos = strCurContent.Find(_T("</b>"));
   //   int nEndIndex = nPos - 1;
   //   strCurContent.Delete(nPos, 4);

   //   m_vecBoldInfo.push_back(std::make_pair(nStartIndex, nEndIndex));

   //   ExtractBoldInfo(strCurContent);
   //}
}

void CSRTDataManager::ParseItalicInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt)
{
   std::string italiccontent = content;

   italiccontent = RemoveBoldTag(italiccontent);
   italiccontent = RemoveUnderlineTag(italiccontent);
   italiccontent = RmoveColorTag(italiccontent);

   ExtractInfoParams params;
   params.content = italiccontent;
   params.reInfo = italicTag;
   params.strPreTag = ITALIC_TAG_PREFIX;
   params.strSufTag = ITALIC_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);
}

void CSRTDataManager::ParseUnderlineInfo(std::string const& content, std::vector<std::pair<int, int>>& vecRestlt)
{
   std::string underlinecontent = content;

   underlinecontent = RemoveBoldTag(underlinecontent);
   underlinecontent = RemoveItalicTag(underlinecontent);
   underlinecontent = RmoveColorTag(underlinecontent);

   ExtractInfoParams params;
   params.content = underlinecontent;
   params.reInfo = underlineTag;
   params.strPreTag = UNDERLINE_TAG_PREFIX;
   params.strSufTag = UNDERLINE_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);
}

void CSRTDataManager::ExtractColorInfo(std::string const& content, std::vector<ColorInfo>& vecRestlt)
{
   std::string curcontent = content;
   std::cmatch m;

   auto ret = std::regex_search(curcontent.c_str(), m, ColorTag);

   if (ret)
   {
      size_t nPosition = m.position();
      ASSERT(m.size() > 1);
      std::string colortag = m[1];
      size_t colorTagSize = colortag.length();

      std::transform(colortag.begin(), colortag.end(), colortag.begin(),
         [](unsigned char c) {return std::tolower(c); });

      COLORREF color = RGB(255, 255, 255); // default color

      if (FAILED(GetColor(colortag, color)))
      {
         OutputDebugString(_T("\r\n ----- Failed to extract color info ----- \r\n"));
      }

      size_t nStart = nPosition;
      size_t nLength = colorTagSize;
      curcontent.erase(nPosition, nLength + 13); // add extra length for <font color=>

      nPosition = curcontent.find("</font>");
      size_t nEndIndex = nPosition - 1;
      curcontent.erase(nPosition, 7); // strength of </font>

      auto colorItem = std::find_if(vecRestlt.begin(), vecRestlt.end(),
         [color](auto& item) {return item.color == color; });

      if (colorItem != vecRestlt.end())
      {
         (*colorItem).vecIndex.emplace_back(std::make_pair(static_cast<int>(nStart), static_cast<int>(nEndIndex)));
      }
      else
      {
         ColorInfo info;
         info.color = color;
         info.vecIndex.emplace_back(std::make_pair(static_cast<int>(nStart), static_cast<int>(nEndIndex)));
         vecRestlt.emplace_back(info);
      }

      ExtractColorInfo(curcontent, vecRestlt);
   }
}

void CSRTDataManager::ParseColorInfo(std::string const& content, std::vector<ColorInfo>& vecRestlt)
{
   std::string colorcontent = content;

   colorcontent = RemoveBoldTag(colorcontent);
   colorcontent = RemoveItalicTag(colorcontent);
   colorcontent = RemoveUnderlineTag(colorcontent);

   ExtractColorInfo(colorcontent, vecRestlt);
   
}

void CSRTDataManager::AddSRTData(CString strStartTime, CString strEndTime, std::string content)
{
   SRTData data;
   data.startTime = strStartTime;
   data.endTime = strEndTime;

   // remove tags from content
   std::string realcontent = content;
   realcontent = RemoveBoldTag(realcontent);
   realcontent = RemoveItalicTag(realcontent);
   realcontent = RemoveUnderlineTag(realcontent);
   realcontent = RmoveColorTag(realcontent);

   data.content = ConvertStringToUnicodeCString(realcontent.c_str());

   ParseBoldInfo(content, data.vecBoldInfo);

   ParseItalicInfo(content, data.vecItalicInfo);

   ParseUnderlineInfo(content, data.vecUnderlineInfo);

   ParseColorInfo(content, data.vecColorInfo);

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   m_vecSRTData.emplace_back(data);
}

size_t CSRTDataManager::GetRelatedIndex(float fProgress)
{
   size_t nCount = m_vecSRTData.size();

   size_t nIndex = static_cast<size_t>(static_cast<float>(nCount) * fProgress);

   return min(nIndex, nCount - 1);
}

CString CSRTDataManager::GetSRTDataStartTime(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index].startTime;
}

CString CSRTDataManager::GetSRTDataEndTime(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index].endTime;
}

CString CSRTDataManager::GetSRTDataContent(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index].content;
}

void CSRTDataManager::GetSRTDataBoldInfo(float fProgress, std::vector<std::pair<int, int>>& vecBold)
{
   vecBold.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   vecBold.insert(vecBold.end(), m_vecSRTData[index].vecBoldInfo.begin(), m_vecSRTData[index].vecBoldInfo.end());
}

void CSRTDataManager::GetSRTDataItalicInfo(float fProgress, std::vector<std::pair<int, int>>& vecItalic)
{
   vecItalic.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   vecItalic.insert(vecItalic.end(), m_vecSRTData[index].vecItalicInfo.begin(), m_vecSRTData[index].vecItalicInfo.end());
}

void CSRTDataManager::GetSRTDataUnderlineInfo(float fProgress, std::vector<std::pair<int, int>>& vecUnderline)
{
   vecUnderline.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   vecUnderline.insert(vecUnderline.end(), m_vecSRTData[index].vecUnderlineInfo.begin(), m_vecSRTData[index].vecUnderlineInfo.end());
}

void CSRTDataManager::GetSRTDataColorInfo(float fProgress, std::vector<ColorInfo>& vecColor)
{
   vecColor.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   vecColor.insert(vecColor.end(), m_vecSRTData[index].vecColorInfo.begin(), m_vecSRTData[index].vecColorInfo.end());
}



