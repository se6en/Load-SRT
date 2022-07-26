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
static const std::wregex ColorPreTag(L"<font color=(.*)>");

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

void SplitString(CString const& strSource, CStringArray& strDest, CString const strTarget)
{
   strDest.RemoveAll();
   CString strContent = strSource;
   int nPosition = strContent.Find(strTarget);
   while (nPosition > 0)
   {
      strDest.Add(strContent.Left(nPosition));
      
      strContent = strContent.Right(strContent.GetLength() - nPosition - strTarget.GetLength());
      nPosition = strContent.Find(strTarget);
   }

   strDest.Add(strContent);
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

BOOL IsValidHexString(std::wstring const wHexString)
{
   for (size_t i = 0; i < wHexString.size(); i++)
   {
      if (wHexString[i] >= '0' && wHexString[i] <= '9')
      {
         continue;
      }

      if (wHexString[i] >= 'a' && wHexString[i] <= 'f')
      {
         continue;
      }

      if (wHexString[i] >= 'A' && wHexString[i] <= 'F')
      {
         continue;
      }

      return FALSE;
   }

   return TRUE;
}

CString GetColorString(COLORREF color, int nOpacity)
{
   CString strHexValue;
   strHexValue.Format(_T("<font color=#%02X%02X%02X%02X>"), GetRValue(color), GetGValue(color), GetBValue(color), nOpacity);
   ASSERT(!strHexValue.IsEmpty());

   return strHexValue;
}

HRESULT GetColorFromHex(CString const& strContent, COLORREF& color)
{
   if (strContent.GetAt(0) != L'#')
   {
      return E_FAIL;
   }

   std::wstring wsContent = strContent.GetString();
   std::wstring hexColor = wsContent.substr(1, 6);

   if (!IsValidHexString(hexColor))
   {
      return E_FAIL;
   }

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

      nPos = strContent.Find(params.strSufTag, nPos);
      ASSERT(nPos > 0);
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

CString CSRTDataManager::RemoveColorPrefixTag(CString const& content)
{
   CString strContent = content;
   std::wstring colorcontent(strContent.GetString());

   std::wsmatch result;
   if (std::regex_search(colorcontent, result, ColorPreTag))
   {
      int nPosition = static_cast<int>(result.position());

      int nPrefixTagEnd = strContent.Find(_T(">"), nPosition);
      ASSERT(nPrefixTagEnd > nPosition);

      int nTagLength = nPrefixTagEnd - nPosition + 1;
      strContent.Delete(nPosition, nTagLength);

      strContent = RemoveColorPrefixTag(strContent);
   }

   return strContent;
}

CString CSRTDataManager::RemoveColorSuffixTag(CString const& content)
{
   CString strContent = content;

   int nPosition = content.Find(COLOR_TAG_SUFFIX);
   if (nPosition < 0)
   {
      return strContent;
   }

   CString strTag = COLOR_TAG_SUFFIX;
   int nTagLength = strTag.GetLength();

   strContent.Delete(nPosition, nTagLength);

   strContent = RemoveColorSuffixTag(strContent);

   return strContent;
}

CString CSRTDataManager::RemoveColorTag(CString const& strContent)
{
   // according to the behavior in Premiere Pro, we need to remove all the color tag even their are not paired
   CString content = strContent;

   std::wstring wsContent(strContent.GetString());

   std::wsmatch result;
   if (!std::regex_search(wsContent, result, ColorTag))
   {
      // no need to do this if no paired color info exist
      return content;
   }

   CString strPrefixContent = result.prefix().matched ? result.prefix().str().c_str() : _T("");
   CString strSuffixContent = result.suffix().matched ? result.suffix().str().c_str() : _T("");

   std::wssub_match sub_match = result[0];
   std::wstring stb_string = sub_match.str();

   CString strMatchContent(stb_string.c_str());
   strMatchContent = RemoveColorPrefixTag(strMatchContent);
   strMatchContent = RemoveColorSuffixTag(strMatchContent);

   content = strPrefixContent + strMatchContent;
   content = content + strSuffixContent;

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
   strBold = RemoveColorTag(strBold);

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
   strItalic = RemoveColorTag(strItalic);

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
   strUnderline = RemoveColorTag(strUnderline);

   ExtractInfoParams params;
   params.content = strUnderline;
   params.reInfo = underlineTag;
   params.strPreTag = UNDERLINE_TAG_PREFIX;
   params.strSufTag = UNDERLINE_TAG_SUFFIX;

   ExtractInfo(params, vecRestlt);
}

BOOL CSRTDataManager::ColorInfoTagExist(CString const& strContent)
{
   CString strCurContent = strContent;

   std::wstring content(strCurContent.GetString());

   std::wsmatch result;
   if (std::regex_search(content, result, ColorTag))
   {
      return TRUE;
   }

   return FALSE;
}

int CSRTDataManager::ExtractColorTagInfoEndIndex(CString& strContent, int nStartIndex)
{
   int nPosition = strContent.Find(COLOR_TAG_SUFFIX, nStartIndex);
   CString strSuffixTag = COLOR_TAG_SUFFIX;

   CString strCurContent = strContent;

   std::wstring content(strCurContent.GetString());

   std::wsmatch result;
   if (!std::regex_search(content, result, ColorPreTag))
   {
      // no more color tag found and the position if the first suffix tag
      // no color tag need to be removed
      if (nPosition < 0)
      {
         return strContent.GetLength();
      }

      strContent.Delete(nPosition, strSuffixTag.GetLength());
      return nPosition;
   }

   int nPreTagPosition = static_cast<int>(result.position());

   if (nPosition < 0)
   {
      return nPreTagPosition;
   }

   if (nPosition < nPreTagPosition)
   {
      strContent.Delete(nPosition, strSuffixTag.GetLength());
      return nPosition;
   }

   return nPreTagPosition;
}

void CSRTDataManager::ExtractColorTagInfo(CString const& strContent, std::vector<colorTagInfo>& vecRestlt)
{
   // all we need to do is to focusing on the pre tag
   CString strCurContent = strContent;
   std::wstring content(strCurContent.GetString());

   std::wsmatch result;
   if (std::regex_search(content, result, ColorTag))
   {
      // 1. remove the extra useless suffix tag to get correct index
      int nIndexOffset = 0;
      if (result.prefix().matched)
      {
         CString strPrefixContent = result.prefix().str().c_str();
         CString strTrimmedContent = RemoveColorSuffixTag(strPrefixContent);

         nIndexOffset = strPrefixContent.GetLength() - strTrimmedContent.GetLength();
      }

      int nPosition = static_cast<int>(result.position());

      int nPrefixTagEnd = strCurContent.Find(_T(">"), nPosition);
      ASSERT(nPrefixTagEnd > nPosition);

      CString strPrefixColorTag = strCurContent.Left(nPrefixTagEnd);
      int nTagLength = strPrefixColorTag.GetLength();
      strPrefixColorTag = strPrefixColorTag.Right(nTagLength - nPosition - 1);

      CString strColorTag = strPrefixColorTag;
      strColorTag.Delete(0, 11);  // remove "font color="

      strCurContent.Delete(nPosition, nPrefixTagEnd - nPosition + 1); // 2 stand for "<" and ">"

      int nEndIndex = ExtractColorTagInfoEndIndex(strCurContent, nPosition) - 1;

      vecRestlt.emplace_back(strColorTag, nPosition - nIndexOffset, nEndIndex - nIndexOffset);

      ExtractColorTagInfo(strCurContent, vecRestlt);
   }
}

void CSRTDataManager::ExtractColorInfo(CString const& strContent, std::vector<CSRTData::ColorInfo>& vecRestlt)
{
   CString strCurContent = strContent;

   std::wstring content(strCurContent.GetString());

   std::wsmatch result;
   if (!std::regex_search(content, result, ColorTag))
   {
      return;
   }

   // 1. extract color tag info
   std::vector<colorTagInfo> vecColorTagInfo;
   ExtractColorTagInfo(strCurContent, vecColorTagInfo);

   if (vecColorTagInfo.empty())
   {
      return;
   }

   // 2. convert color tag info to color info
   for (auto tagInfo : vecColorTagInfo)
   {
      std::wstring colortag(tagInfo.tag.GetString());

      std::transform(colortag.begin(), colortag.end(), colortag.begin(),
         [](auto c) {return std::tolower(c); });

      COLORREF color = RGB(255, 255, 255); // default color

      if (FAILED(GetColor(colortag.c_str(), color)))
      {
         OutputDebugString(_T("\r\n ----- Failed to extract color info ----- \r\n"));
         continue;
      }

      auto colorItem = std::find_if(vecRestlt.begin(), vecRestlt.end(),
         [color](auto& item) {return item.color == color; });

      if (colorItem != vecRestlt.end())
      {
         (*colorItem).vecIndex.emplace_back(std::make_pair(tagInfo.start, tagInfo.end));
      }
      else
      {
         CSRTData::ColorInfo info;
         info.color = color;
         info.vecIndex.emplace_back(std::make_pair(tagInfo.start, tagInfo.end));
         vecRestlt.emplace_back(info);
      }
   }
}

void CSRTDataManager::ParseColorInfo(CString const& content, std::vector<CSRTData::ColorInfo>& vecRestlt)
{
   CString strColor = content;

   strColor = RemoveBoldTag(strColor);
   strColor = RemoveItalicTag(strColor);
   strColor = RemoveUnderlineTag(strColor);

   ExtractColorInfo(strColor, vecRestlt);

}

void CSRTDataManager::ClearSRTData()
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);
   m_vecSRTData.clear();
}

void CSRTDataManager::AddSRTData(CString strStartTime, CString strEndTime, CString strContent)
{
   std::shared_ptr<CSRTData> pData = std::make_shared<CSRTData>();

   pData->SetStartTime(strStartTime);
   pData->SetEndTime(strEndTime);

   if (strContent.IsEmpty())
   {
      pData->SetContent(strContent);
      return;
   }

   CStringArray strContentArray;
   SplitString(strContent, strContentArray, _T("\n"));

   // need to seperate content to lines to extract info
   CString strRealcontent;
   std::vector<std::pair<int, int>> vecBoldInfo, vecItalicInfo, vecUnderlineInfo;
   std::vector<CSRTData::ColorInfo> vecColorInfo;
   int nIndexOffset = 0;
   for (int i = 0; i < strContentArray.GetSize(); i++)
   {
      // 1. extract content
      CString strContent = strContentArray[i];

      strContent = RemoveBoldTag(strContent);
      strContent = RemoveItalicTag(strContent);
      strContent = RemoveUnderlineTag(strContent);
      strContent = RemoveColorTag(strContent);

      if (!strRealcontent.IsEmpty())
      {
         strRealcontent += _T("\n");
      }

      strRealcontent += strContent;

      // 2. extract format info
      CString strLine = strContentArray[i];

      // Bold Info
      std::vector<std::pair<int, int>> vecCurBoldInfo;
      ParseBoldInfo(strLine, vecCurBoldInfo);

      for (auto pBoldInfo : vecCurBoldInfo)
      {
         vecBoldInfo.emplace_back(std::make_pair(pBoldInfo.first + nIndexOffset, pBoldInfo.second + nIndexOffset));
      }
      
      // Italic Info
      std::vector<std::pair<int, int>> vecCurItalicInfo;
      ParseItalicInfo(strLine, vecCurItalicInfo);

      for (auto pItalicInfo : vecCurItalicInfo)
      {
         vecItalicInfo.emplace_back(std::make_pair(pItalicInfo.first + nIndexOffset, pItalicInfo.second + nIndexOffset));
      }

      // Underline Info
      std::vector<std::pair<int, int>> vecCurUnderlineInfo;
      ParseUnderlineInfo(strLine, vecCurUnderlineInfo);

      for (auto pUnderlineInfo : vecCurUnderlineInfo)
      {
         vecUnderlineInfo.emplace_back(std::make_pair(pUnderlineInfo.first + nIndexOffset, pUnderlineInfo.second + nIndexOffset));
      }

      // Color Info
      std::vector<CSRTData::ColorInfo> vecCurColorInfo;
      ParseColorInfo(strLine, vecCurColorInfo);

      for (auto pColorInfo : vecCurColorInfo)
      {
         auto colorItem = std::find_if(vecColorInfo.begin(), vecColorInfo.end(),
            [pColorInfo](auto& item) {return item.color == pColorInfo.color; });

         if (colorItem != vecColorInfo.end())
         {
            for (auto pIndex : pColorInfo.vecIndex)
            {
               (*colorItem).vecIndex.emplace_back(pIndex.first + nIndexOffset, pIndex.second + nIndexOffset);
            }
         }
         else
         {
            CSRTData::ColorInfo info;
            info.color = pColorInfo.color;
            for (auto pIndex : pColorInfo.vecIndex)
            {
               info.vecIndex.emplace_back(pIndex.first + nIndexOffset, pIndex.second + nIndexOffset);
            }
            vecColorInfo.emplace_back(info);
         }
      }

      nIndexOffset = strRealcontent.GetLength() + 1;
   }

   pData->SetContent(strRealcontent);

   pData->SetBoldInfo(vecBoldInfo);

   pData->SetItalicInfo(vecItalicInfo);

   pData->SetUnderlineInfo(vecUnderlineInfo);

   pData->SetColorInfo(vecColorInfo);

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   m_vecSRTData.emplace_back(pData);
}

BOOL CSRTDataManager::HasValidData() const
{
   return !m_vecSRTData.empty();
}

int CSRTDataManager::GetSRTDataCount()
{
   return static_cast<int>(m_vecSRTData.size());
}

CString CSRTDataManager::GetUnstyledContent(int nIndex)
{
   if (nIndex < 0 || nIndex >= GetSRTDataCount())
   {
      return CString();
   }

   return m_vecSRTData[nIndex]->GetUnstyledContent();
}

CString CSRTDataManager::GetTime(int nIndex)
{
   if (nIndex < 0 || nIndex >= GetSRTDataCount())
   {
      return CString();
   }

   return m_vecSRTData[nIndex]->GetFormatTime();
}

CString CSRTDataManager::GetStyledContent(int nIndex)
{
   if (nIndex < 0 || nIndex >= GetSRTDataCount())
   {
      return CString();
   }

   return m_vecSRTData[nIndex]->GetStyledContent();
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

   return m_vecSRTData[index]->GetStartTime();
}

CString CSRTDataManager::GetSRTDataEndTime(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index]->GetEndTime();
}

CString CSRTDataManager::GetSRTDataContent(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index]->GetUnstyledContent();
}

void CSRTDataManager::GetSRTDataBoldInfo(float fProgress, std::vector<std::pair<int, int>>& vecBold)
{
   vecBold.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   m_vecSRTData[index]->GetBoldInfo(vecBold);
}

void CSRTDataManager::GetSRTDataItalicInfo(float fProgress, std::vector<std::pair<int, int>>& vecItalic)
{
   vecItalic.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   m_vecSRTData[index]->GetItalicInfo(vecItalic);
}

void CSRTDataManager::GetSRTDataUnderlineInfo(float fProgress, std::vector<std::pair<int, int>>& vecUnderline)
{
   vecUnderline.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   m_vecSRTData[index]->GetUnderlineInfo(vecUnderline);
}

void CSRTDataManager::GetSRTDataColorInfo(float fProgress, std::vector<CSRTData::ColorInfo>& vecColor)
{
   vecColor.clear();

   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   m_vecSRTData[index]->GetColorInfo(vecColor);
}

std::shared_ptr<CSRTData> CSRTDataManager::GetData(float fProgress)
{
   std::lock_guard<std::mutex> lock(m_mutexSRTData);

   size_t index = GetRelatedIndex(fProgress);

   return m_vecSRTData[index];
}

int CSRTDataManager::ConvertUnicodeToUTF8CString(const wchar_t* pszUnicode, CStringA& strUTF8)
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

void CSRTData::SetContent(CString const & strContent)
{
   m_strContent = strContent;
}

CString CSRTData::GetUnstyledContent()
{
   return m_strContent;
}

CString CSRTData::GetStyledContent()
{
   CString strContent = m_strContent;
   std::vector<InsertInfo> vecInsertInfo;
   
   InsertFormatInfo(strContent, BOLD_TAG_PREFIX, BOLD_TAG_SUFFIX, m_vecBoldInfo, vecInsertInfo);
   InsertFormatInfo(strContent, ITALIC_TAG_PREFIX, ITALIC_TAG_SUFFIX, m_vecItalicInfo, vecInsertInfo);
   InsertFormatInfo(strContent, UNDERLINE_TAG_PREFIX, UNDERLINE_TAG_SUFFIX, m_vecUnderlineInfo, vecInsertInfo);

   InsertColorInfo(strContent, vecInsertInfo);

   return strContent;
}

void CSRTData::SetStartTime(CString const & strTime)
{
   m_strStartTime = strTime;
}

CString CSRTData::GetStartTime()
{
   return m_strStartTime;
}

void CSRTData::SetEndTime(CString const & strTime)
{
   m_strEndTime = strTime;
}

CString CSRTData::GetEndTime()
{
   return m_strEndTime;
}

CString CSRTData::GetFormatTime()
{
   CString strTime = m_strStartTime;
   strTime += _T(" --> ");
   strTime += m_strEndTime;

   return strTime;
}

void CSRTData::SetBoldInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   m_vecBoldInfo.clear();
   m_vecBoldInfo.insert(m_vecBoldInfo.end(), vecInfo.begin(), vecInfo.end());
}

void CSRTData::GetBoldInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   vecInfo.clear();
   vecInfo.insert(vecInfo.end(), m_vecBoldInfo.begin(), m_vecBoldInfo.end());
}

void CSRTData::SetItalicInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   m_vecItalicInfo.clear();
   m_vecItalicInfo.insert(m_vecItalicInfo.end(), vecInfo.begin(), vecInfo.end());
}

void CSRTData::GetItalicInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   vecInfo.clear();
   vecInfo.insert(vecInfo.end(), m_vecItalicInfo.begin(), m_vecItalicInfo.end());
}

void CSRTData::SetUnderlineInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   m_vecUnderlineInfo.clear();
   m_vecUnderlineInfo.insert(m_vecUnderlineInfo.end(), vecInfo.begin(), vecInfo.end());
}

void CSRTData::GetUnderlineInfo(std::vector<std::pair<int, int>>& vecInfo)
{
   vecInfo.clear();
   vecInfo.insert(vecInfo.end(), m_vecUnderlineInfo.begin(), m_vecUnderlineInfo.end());
}

void CSRTData::SetColorInfo(std::vector<ColorInfo>& vecInfo)
{
   m_vecColorInfo.clear();
   m_vecColorInfo.insert(m_vecColorInfo.end(), vecInfo.begin(), vecInfo.end());
}

void CSRTData::GetColorInfo(std::vector<ColorInfo>& vecInfo)
{
   vecInfo.clear();
   vecInfo.insert(vecInfo.end(), m_vecColorInfo.begin(), m_vecColorInfo.end());
}

void CSRTData::InsertFormatInfo(CString& strContent, CString const strStart, CString const strEnd, std::vector<std::pair<int, int>>& vecFormatInfo, std::vector<InsertInfo>& vecInsertInfo)
{
   if (vecFormatInfo.empty())
   {
      return;
   }

   for (auto pInfo : vecFormatInfo)
   {
      int nStart = pInfo.first;
      int nEnd = pInfo.second;

      int nStartOffset = 0;
      int nEndOffset = 0;
      for (size_t i = 0; i < vecInsertInfo.size(); i++)
      {
         if (nStart >= vecInsertInfo[i].start)
         {
            nStartOffset += vecInsertInfo[i].length;
         }

         if (nEnd >= vecInsertInfo[i].start)
         {
            nEndOffset += vecInsertInfo[i].length;
         }
      }

      strContent.Insert(nStart + nStartOffset, strStart);
      
      nEnd += strStart.GetLength();
      strContent.Insert(nEnd + nEndOffset + 1, strEnd);

      vecInsertInfo.emplace_back(pInfo.first, strStart.GetLength());
      vecInsertInfo.emplace_back(pInfo.second, strEnd.GetLength());
   }
}

void CSRTData::InsertColorInfo(CString& strContent, std::vector<InsertInfo>& vecInsertInfo)
{
   if (m_vecColorInfo.empty())
   {
      return;
   }

   std::vector<InsertInfo> vecInsertColorInfo;
   for (auto pInfo : m_vecColorInfo)
   {
      CString strColorStartTag = GetColorString(pInfo.color, 255);
      CString strColorEndTag = COLOR_TAG_SUFFIX;

      for (auto pIndex : pInfo.vecIndex)
      {
         int nStart = pIndex.first;
         int nEnd = pIndex.second;

         vecInsertColorInfo.emplace_back(nStart, strColorStartTag.GetLength());
         vecInsertColorInfo.emplace_back(nEnd, strColorEndTag.GetLength());

         int nStartOffset = 0;
         int nEndOffset = 0;
         for (size_t i = 0; i < vecInsertInfo.size(); i++)
         {
            if (nStart >= vecInsertInfo[i].start)
            {
               nStartOffset += vecInsertInfo[i].length;
            }

            if (nEnd >= vecInsertInfo[i].start)
            {
               nEndOffset += vecInsertInfo[i].length;
            }
         }

         strContent.Insert(nStart + nStartOffset, strColorStartTag);

         nEnd += strColorStartTag.GetLength();
         strContent.Insert(nEnd + nEndOffset + 1, strColorEndTag);

         vecInsertInfo.emplace_back(pIndex.first, strColorStartTag.GetLength());
         vecInsertInfo.emplace_back(pIndex.second, strColorEndTag.GetLength());
      }
   }

}
