#include "stdafx.h"

#include "public/StringUtils.h"
#include "public/TimeUtil.h"

#include <cpprest/http_client.h>

std::string ed::Utf16ToUtf8(const std::wstring& str)
{
	return utility::conversions::to_utf8string(str);
}

