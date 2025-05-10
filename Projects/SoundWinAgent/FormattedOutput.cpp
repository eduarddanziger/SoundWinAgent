#include "stdafx.h"

#include "FormattedOutput.h"

#include <TimeUtils.h>
#include "public/StringUtils.h"

#include <SpdLogger.h>

#include <public/ClassDefHelper.h>

#include <magic_enum/magic_enum_iostream.hpp>


void FormattedOutput::LogAndPrint(const std::wstring & mess)
{
    SPD_L->info(ed::WString2StringTruncate(mess));
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

void FormattedOutput::PrintEvent(SoundDeviceEventType event, const std::string & devicePnpId)
{
    using magic_enum::iostream_operators::operator<<;

    std::ostringstream os; os << "Event caught: " << event << "."
        << " Device PnP id: " << devicePnpId << '\n';
    LogAndPrint(os.str());
}

void FormattedOutput::PrintDeviceInfo(const SoundDeviceInterface * device)
{
    using magic_enum::iostream_operators::operator<<;

    const auto idString = device->GetPnpId();
	std::ostringstream os; os << std::string(4, ' ')
        << idString
        << ", \"" << device->GetName()
        << "\", " << device->GetFlow()
        << ", Volume " << device->GetCurrentRenderVolume()
        << " / " << device->GetCurrentCaptureVolume();
    LogAndPrint(os.str());
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
