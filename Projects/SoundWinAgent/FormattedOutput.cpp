#include "stdafx.h"

#include "FormattedOutput.h"

#include <TimeUtils.h>

#include <public/ClassDefHelper.h>
#include <public/DefToString.h>

#include <SpdLogger.h>


void FormattedOutput::LogAndPrint(const std::wstring & mess)
{
    SPD_L->info(WString2StringTruncate(mess));
    std::wcout << CurrentLocalTimeWithoutDate << mess << '\n';
}

void FormattedOutput::LogAndPrint(const std::string & mess)
{
    SPD_L->info(mess);
    std::cout << CurrentLocalTimeWithoutDate << mess << '\n';
}

void FormattedOutput::LogAsErrorPrintAndThrow(const std::string & mess)
{
    SPD_L->error(mess);
    std::cerr << CurrentLocalTimeWithoutDate << mess << '\n';
	throw std::runtime_error(mess);
}

void FormattedOutput::PrintEvent(SoundDeviceEventType event, const std::wstring & devicePnpId)
{
    std::wostringstream wos; wos << L"Event caught: " << ed::GetDeviceCollectionEventAsString(event) << L"."
        << L" Device PnP id: " << devicePnpId << L'\n';
    LogAndPrint(wos.str());
}

void FormattedOutput::PrintDeviceInfo(const SoundDeviceInterface * device)
{
    const auto idString = device->GetPnpId();
    const std::wstring idAsWideString(idString.begin(), idString.end());
	std::wostringstream wos; wos << std::wstring(4, L' ')
        << idString
        << ", \"" << device->GetName()
        << "\", " << ed::GetFlowAsString(device->GetFlow())
        << ", Volume " << device->GetCurrentRenderVolume()
        << " / " << device->GetCurrentCaptureVolume();
    LogAndPrint(wos.str());
}

std::string FormattedOutput::WString2StringTruncate(const std::wstring & str)
{
    std::string result;
    result.reserve(str.size());

    std::ranges::for_each(str, [&result](wchar_t wc) {
        result += static_cast<char>(wc);
    });

    return result;
}

std::wostream & FormattedOutput::CurrentLocalTimeWithoutDate(std::wostream & os)
{
    const std::wstring currentTime = ed::getLocalTimeAsWideString();
    if
    (
        constexpr int beginOfTimeCountingFromTheEnd = 15;
        currentTime.size() >= beginOfTimeCountingFromTheEnd
    )
    {
        os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << L" ";
    }
    return os;
}

std::ostream & FormattedOutput::CurrentLocalTimeWithoutDate(std::ostream & os)
{
    const std::string currentTime = ed::getLocalTimeAsString();
    if
    (
        constexpr int beginOfTimeCountingFromTheEnd = 15;
        currentTime.size() >= beginOfTimeCountingFromTheEnd
    )
    {
        os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << " ";
    }
    return os;
}
