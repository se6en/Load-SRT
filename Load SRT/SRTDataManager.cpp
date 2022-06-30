#include "stdafx.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include "SRTDataManager.h"

static constexpr auto BOLD_TAG_PREFIX = _T("<b>");
static constexpr auto BOLD_TAG_SUFFIX = _T("</b>");
static const std::wregex boldTag(L"<b>(.*)</b>");

static constexpr auto ITALIC_TAG_PREFIX = _T("<i>");
static constexpr auto ITALIC_TAG_SUFFIX = _T("</i>");
static const std::wregex italicTag(L"<i>(.*)</i>");

static constexpr auto UNDERLINE_TAG_PREFIX = _T("<u>");
static constexpr auto UNDERLINE_TAG_SUFFIX = _T("</u>");
static const std::wregex underlineTag(L"<u>(.*)</u>");

static constexpr auto COLOR_TAG_SUFFIX = _T("</font>");
static const std::wregex ColorTag(L"<font color=(.*)>(.*)</font>");

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

struct NamedColor { CString name; const COLORREF color; };

const NamedColor namedColors[] = {
    { _T("transparent"), RGB(0, 0, 0) },
    { _T("aliceblue"), RGB(240, 248, 255) },
    { _T("antiquewhite"), RGB(250, 235, 215) },
    { _T("aqua"), RGB(0, 255, 255)},
    { _T("aquamarine"), RGB(127, 255, 212) },
    { _T("azure"), RGB(240, 255, 255) },
    { _T("beige"), RGB(245, 245, 220) },
    { _T("bisque"), RGB(255, 228, 196) },
    { _T("black"), RGB(0, 0, 0) },
    { _T("blanchedalmond"), RGB(255, 235, 205) },
    { _T("blue"), RGB(0, 0, 255) },
    { _T("blueviolet"), RGB(138, 43, 226) },
    { _T("brown"), RGB(165, 42, 42) },
    { _T("burlywood"), RGB(222, 184, 135) },
    { _T("cadetblue"), RGB(95, 158, 160) },
    { _T("chartreuse"), RGB(127, 255, 0) },
    { _T("chocolate"), RGB(210, 105, 30) },
    { _T("coral"), RGB(255, 127, 80) },
    { _T("cornflowerblue"), RGB(100, 149, 237) },
    { _T("cornsilk"), RGB(255, 248, 220) },
    { _T("crimson"), RGB(220, 20, 60) },
    { _T("cyan"), RGB(0, 255, 255) },
    { _T("darkblue"), RGB(0, 0, 139) },
    { _T("darkcyan"), RGB(0, 139, 139) },
    { _T("darkgoldenrod"), RGB(184, 134, 11) },
    { _T("darkgray"), RGB(169, 169, 169) },
    { _T("darkgreen"), RGB(0, 100, 0) },
    { _T("darkgrey"), RGB(169, 169, 169) },
    { _T("darkkhaki"), RGB(189, 183, 107) },
    { _T("darkmagenta"), RGB(139, 0, 139) },
    { _T("darkolivegreen"), RGB(85, 107, 47) },
    { _T("darkorange"), RGB(255, 140, 0) },
    { _T("darkorchid"), RGB(153, 50, 204) },
    { _T("darkred"), RGB(139, 0, 0) },
    { _T("darksalmon"), RGB(233, 150, 122) },
    { _T("darkseagreen"), RGB(143, 188, 143) },
    { _T("darkslateblue"), RGB( 72, 61, 139)},
    { _T("darkslategray"), RGB( 47, 79, 79)},
    { _T("darkslategrey"), RGB( 47, 79, 79)},
    { _T("darkturquoise"), RGB( 0, 206, 209)},
    { _T("darkviolet"), RGB( 148, 0, 211)},
    { _T("deeppink"), RGB( 255, 20, 147)},
    { _T("deepskyblue"), RGB( 0, 191, 255)},
    { _T("dimgray"), RGB( 105, 105, 105)},
    { _T("dimgrey"), RGB( 105, 105, 105)},
    { _T("dodgerblue"), RGB( 30, 144, 255)},
    { _T("firebrick"), RGB( 178, 34, 34)},
    { _T("floralwhite"), RGB( 255, 250, 240)},
    { _T("forestgreen"), RGB( 34, 139, 34)},
    { _T("fuchsia"), RGB( 255, 0, 255)},
    { _T("gainsboro"), RGB( 220, 220, 220)},
    { _T("ghostwhite"), RGB( 248, 248, 255)},
    { _T("gold"), RGB( 255, 215, 0)},
    { _T("goldenrod"), RGB( 218, 165, 32)},
    { _T("gray"), RGB( 128, 128, 128)},
    { _T("green"), RGB( 0, 128, 0)},
    { _T("greenyellow"), RGB( 173, 255, 47)},
    { _T("grey"), RGB( 128, 128, 128)},
    { _T("honeydew"), RGB( 240, 255, 240)},
    { _T("hotpink"), RGB( 255, 105, 180)},
    { _T("indianred"), RGB( 205, 92, 92)},
    { _T("indigo"), RGB( 75, 0, 130)},
    { _T("ivory"), RGB( 255, 255, 240)},
    { _T("khaki"), RGB( 240, 230, 140)},
    { _T("lavender"), RGB( 230, 230, 250)},
    { _T("lavenderblush"), RGB( 255, 240, 245)},
    { _T("lawngreen"), RGB( 124, 252, 0)},
    { _T("lemonchiffon"), RGB( 255, 250, 205)},
    { _T("lightblue"), RGB( 173, 216, 230)},
    { _T("lightcoral"), RGB( 240, 128, 128)},
    { _T("lightcyan"), RGB( 224, 255, 255)},
    { _T("lightgoldenrodyellow"), RGB( 250, 250, 210)},
    { _T("lightgray"), RGB( 211, 211, 211)},
    { _T("lightgreen"), RGB( 144, 238, 144)},
    { _T("lightgrey"), RGB( 211, 211, 211)},
    { _T("lightpink"), RGB( 255, 182, 193)},
    { _T("lightsalmon"), RGB( 255, 160, 122)},
    { _T("lightseagreen"), RGB( 32, 178, 170)},
    { _T("lightskyblue"), RGB( 135, 206, 250)},
    { _T("lightslategray"), RGB( 119, 136, 153)},
    { _T("lightslategrey"), RGB( 119, 136, 153)},
    { _T("lightsteelblue"), RGB( 176, 196, 222)},
    { _T("lightyellow"), RGB( 255, 255, 224)},
    { _T("lime"), RGB( 0, 255, 0)},
    { _T("limegreen"), RGB( 50, 205, 50)},
    { _T("linen"), RGB( 250, 240, 230)},
    { _T("magenta"), RGB( 255, 0, 255)},
    { _T("maroon"), RGB( 128, 0, 0)},
    { _T("mediumaquamarine"), RGB( 102, 205, 170)},
    { _T("mediumblue"), RGB( 0, 0, 205)},
    { _T("mediumorchid"), RGB( 186, 85, 211)},
    { _T("mediumpurple"), RGB( 147, 112, 219)},
    { _T("mediumseagreen"), RGB( 60, 179, 113)},
    { _T("mediumslateblue"), RGB( 123, 104, 238)},
    { _T("mediumspringgreen"), RGB( 0, 250, 154)},
    { _T("mediumturquoise"), RGB( 72, 209, 204)},
    { _T("mediumvioletred"), RGB( 199, 21, 133)},
    { _T("midnightblue"), RGB( 25, 25, 112)},
    { _T("mintcream"), RGB( 245, 255, 250)},
    { _T("mistyrose"), RGB( 255, 228, 225)},
    { _T("moccasin"), RGB( 255, 228, 181)},
    { _T("navajowhite"), RGB( 255, 222, 173)},
    { _T("navy"), RGB( 0, 0, 128)},
    { _T("oldlace"), RGB( 253, 245, 230)},
    { _T("olive"), RGB( 128, 128, 0)},
    { _T("olivedrab"), RGB( 107, 142, 35)},
    { _T("orange"), RGB( 255, 165, 0)},
    { _T("orangered"), RGB( 255, 69, 0)},
    { _T("orchid"), RGB( 218, 112, 214)},
    { _T("palegoldenrod"), RGB( 238, 232, 170)},
    { _T("palegreen"), RGB( 152, 251, 152)},
    { _T("paleturquoise"), RGB( 175, 238, 238)},
    { _T("palevioletred"), RGB( 219, 112, 147)},
    { _T("papayawhip"), RGB( 255, 239, 213)},
    { _T("peachpuff"), RGB( 255, 218, 185)},
    { _T("peru"), RGB( 205, 133, 63)},
    { _T("pink"), RGB( 255, 192, 203)},
    { _T("plum"), RGB( 221, 160, 221)},
    { _T("powderblue"), RGB( 176, 224, 230)},
    { _T("purple"), RGB( 128, 0, 128)},
    { _T("red"), RGB( 255, 0, 0)},
    { _T("rosybrown"), RGB( 188, 143, 143)},
    { _T("royalblue"), RGB( 65, 105, 225)},
    { _T("saddlebrown"), RGB( 139, 69, 19)},
    { _T("salmon"), RGB( 250, 128, 114)},
    { _T("sandybrown"), RGB( 244, 164, 96)},
    { _T("seagreen"), RGB( 46, 139, 87)},
    { _T("seashell"), RGB( 255, 245, 238)},
    { _T("sienna"), RGB( 160, 82, 45)},
    { _T("silver"), RGB( 192, 192, 192)},
    { _T("skyblue"), RGB( 135, 206, 235)},
    { _T("slateblue"), RGB( 106, 90, 205)},
    { _T("slategray"), RGB( 112, 128, 144)},
    { _T("slategrey"), RGB( 112, 128, 144)},
    { _T("snow"), RGB( 255, 250, 250)},
    { _T("springgreen"), RGB( 0, 255, 127)},
    { _T("steelblue"), RGB( 70, 130, 180)},
    { _T("tan"), RGB( 210, 180, 140)},
    { _T("teal"), RGB( 0, 128, 128)},
    { _T("thistle"), RGB( 216, 191, 216)},
    { _T("tomato"), RGB( 255, 99, 71)},
    { _T("turquoise"), RGB( 64, 224, 208)},
    { _T("violet"), RGB( 238, 130, 238)},
    { _T("wheat"), RGB( 245, 222, 179)},
    { _T("white"), RGB( 255, 255, 255)},
    { _T("whitesmoke"), RGB( 245, 245, 245)},
    { _T("yellow"), RGB( 255, 255, 0)},
    { _T("yellowgreen"), RGB( 154, 205, 50)}
};

template <typename T>
int ClampValue(T value)
{
   value = static_cast<T>(::round(value));
   return value < 0 ? 0 : value > 255 ? 255 : static_cast<int>(value);
}

HRESULT GetColorFromHex(CString const& strContent, COLORREF& color)
{
   if (strContent.GetAt(0) != L'#')
   {
      return E_FAIL;
   }

   std::wstring wsContent = strContent.GetString();
   std::wstring hexColor = wsContent.substr(1, 6);
   int64_t hexValue = wcstol(hexColor.c_str(), nullptr, 16);
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

HRESULT GetColorFromRGB(CString const& content, COLORREF& color)
{
   if (content.Find(_T("rgb")) < 0)
   {
      return E_FAIL;
   }

   int nStart = content.Find(_T("("));
   int nEnd = content.Find(_T(")"));

   if (nStart < 0 || nEnd < 0 || nEnd + 1 != content.GetLength())
   {
      return E_FAIL;
   }

   std::wstring wsContent = content.GetString();
   std::wstring rgbColor = wsContent.substr(nStart + 1, nEnd - (nStart + 1));
   
   std::wstringstream infostream(rgbColor);
   std::vector<std::wstring> vecValue;
   std::wstring item;

   while (std::getline(infostream, item, L','))
   {
      vecValue.push_back(item);
   }

   if (vecValue.size() != 3)
   {
      return E_FAIL;
   }

   int nRed = ClampValue(wcstol(vecValue[0].c_str(), nullptr, 10));
   int nGreen = ClampValue(wcstol(vecValue[1].c_str(), nullptr, 10));
   int nBlue = ClampValue(wcstol(vecValue[2].c_str(), nullptr, 10));

   color = RGB(nRed, nGreen, nBlue);
   return S_OK;
}

HRESULT GetColorFromName(CString const& content, COLORREF& color)
{
   for (const auto& item : namedColors)
   {
      if (content.CompareNoCase(item.name) == 0)
      {
         color = item.color;
         return S_OK;
      }
   }

   return E_FAIL;
}

HRESULT GetColor(CString const& content, COLORREF& color)
{
   if (content.IsEmpty())
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

CString CSRTDataManager::RemoveTag(RemoveTagParams params)
{
   if (params.content.Find(params.strPreTag) < 0 || params.content.Find(params.strSufTag) < 0)
   {
      return params.content;
   }

   CString strContent = params.content;
   std::wstring content(strContent.GetString());

   std::wsmatch result;
   if (std::regex_search(content, result, params.reInfo))
   {
      int nPos = strContent.Find(params.strPreTag);
      int nLength = params.strPreTag.GetLength();
      strContent.Delete(nPos, nLength);

      nPos = strContent.Find(params.strSufTag);
      nLength = params.strSufTag.GetLength();
      strContent.Delete(nPos, nLength);

      params.content = strContent;

      strContent = RemoveTag(params);
   }

   return strContent;
}

CString CSRTDataManager::RemoveBoldTag(CString const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = boldTag;
   params.strPreTag = BOLD_TAG_PREFIX;
   params.strSufTag = BOLD_TAG_SUFFIX;

   return RemoveTag(params);
}

CString CSRTDataManager::RemoveItalicTag(CString const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = italicTag;
   params.strPreTag = ITALIC_TAG_PREFIX;
   params.strSufTag = ITALIC_TAG_SUFFIX;

   return RemoveTag(params);
}

CString CSRTDataManager::RemoveUnderlineTag(CString const& strContent)
{
   RemoveTagParams params;
   params.content = strContent;
   params.reInfo = underlineTag;
   params.strPreTag = UNDERLINE_TAG_PREFIX;
   params.strSufTag = UNDERLINE_TAG_SUFFIX;

   return RemoveTag(params);
}

CString CSRTDataManager::RmoveColorTag(CString const& strContent)
{
   CString content = strContent;

   std::wstring wsContent(strContent.GetString());

   std::wsmatch result;
   if (std::regex_search(wsContent, result, ColorTag))
   {
      int nPosition = static_cast<int>(result.position());
      ASSERT(result.size() > 1);
      std::wstring colortag = result[1];
      size_t colorTagSize = colortag.length();

      int nStart = static_cast<int>(nPosition);
      int nLength = static_cast<int>(colorTagSize);
      content.Delete(nPosition, nLength + 13); // add extra length for <font color=>

      nPosition = content.Find(COLOR_TAG_SUFFIX);
      int nEndIndex = static_cast<int>(nPosition - 1);
      content.Delete(nPosition, 7); // strength of </font>

      content = RmoveColorTag(content);
   }

   return content;
}

void CSRTDataManager::ExtractInfo(ExtractInfoParams& info, std::vector<std::pair<int, int>>& vecResult)
{
   CString strContent = info.content;

   std::wstring content(strContent.GetString());

   std::wsmatch result;
   if (std::regex_search(content, result, info.reInfo))
   {
      int nPos = strContent.Find(info.strPreTag);
      int nStartIndex = nPos;
      int nLength = info.strPreTag.GetLength();
      strContent.Delete(nPos, nLength);

      nPos = strContent.Find(info.strSufTag);
      int nEndIndex = nPos - 1;
      nLength = info.strSufTag.GetLength();
      strContent.Delete(nPos, nLength);

      vecResult.push_back(std::make_pair(static_cast<int>(nStartIndex), static_cast<int>(nEndIndex)));

      info.content = strContent;

      ExtractInfo(info, vecResult);
   }
}

void CSRTDataManager::ParseBoldInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt)
{
   CString strBold = strContent;

   strBold = RemoveItalicTag(strBold);
   strBold = RemoveUnderlineTag(strBold);
   strBold = RmoveColorTag(strBold);

   ExtractInfoParams params;
   params.content = strBold;
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

void CSRTDataManager::ParseItalicInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt)
{
   CString strItalic = strContent;

   strItalic = RemoveBoldTag(strItalic);
   strItalic = RemoveUnderlineTag(strItalic);
   strItalic = RmoveColorTag(strItalic);

   ExtractInfoParams params;
   params.content = strItalic;
   params.reInfo = italicTag;
   params.strPreTag = ITALIC_TAG_PREFIX;
   params.strSufTag = ITALIC_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);
}

void CSRTDataManager::ParseUnderlineInfo(CString const& strContent, std::vector<std::pair<int, int>>& vecRestlt)
{
   CString strUnderline = strContent;

   strUnderline = RemoveBoldTag(strUnderline);
   strUnderline = RemoveItalicTag(strUnderline);
   strUnderline = RmoveColorTag(strUnderline);

   ExtractInfoParams params;
   params.content = strUnderline;
   params.reInfo = underlineTag;
   params.strPreTag = UNDERLINE_TAG_PREFIX;
   params.strSufTag = UNDERLINE_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);
}

void CSRTDataManager::ExtractColorInfo(CString const& strContent, std::vector<ColorInfo>& vecRestlt)
{
   CString strCurContent = strContent;

   std::wstring content(strCurContent.GetString());

   std::wsmatch result;
   if (std::regex_search(content, result, ColorTag))
   {
      int nPosition = static_cast<int>(result.position());
      ASSERT(result.size() > 1);
      std::wstring colortag = result[1];
      size_t colorTagSize = colortag.length();

      std::transform(colortag.begin(), colortag.end(), colortag.begin(),
         [](auto c) {return std::tolower(c); });

      COLORREF color = RGB(255, 255, 255); // default color

      if (FAILED(GetColor(colortag.c_str(), color)))
      {
         OutputDebugString(_T("\r\n ----- Failed to extract color info ----- \r\n"));
      }

      int nStart = nPosition;
      int nLength = static_cast<int>(colorTagSize);
      strCurContent.Delete(nPosition, nLength + 13); // add extra length for <font color=>

      nPosition = strCurContent.Find(COLOR_TAG_SUFFIX);
      int nEndIndex = nPosition - 1;
      strCurContent.Delete(nPosition, 7); // strength of </font>

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

      ExtractColorInfo(strCurContent, vecRestlt);
   }
}

void CSRTDataManager::ParseColorInfo(CString const& content, std::vector<ColorInfo>& vecRestlt)
{
   CString strColor = content;

   strColor = RemoveBoldTag(strColor);
   strColor = RemoveItalicTag(strColor);
   strColor = RemoveUnderlineTag(strColor);

   ExtractColorInfo(strColor, vecRestlt);
   
}

void CSRTDataManager::AddSRTData(CString strStartTime, CString strEndTime, CString strContent)
{
   SRTData data;
   data.startTime = strStartTime;
   data.endTime = strEndTime;

   // remove tags from content
   CString strRealcontent = strContent;
   strRealcontent = RemoveBoldTag(strRealcontent);
   strRealcontent = RemoveItalicTag(strRealcontent);
   strRealcontent = RemoveUnderlineTag(strRealcontent);
   strRealcontent = RmoveColorTag(strRealcontent);

   data.content = strRealcontent;

   ParseBoldInfo(strContent, data.vecBoldInfo);

   ParseItalicInfo(strContent, data.vecItalicInfo);

   ParseUnderlineInfo(strContent, data.vecUnderlineInfo);

   ParseColorInfo(strContent, data.vecColorInfo);

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



