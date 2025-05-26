#include "stdafx.h"

#include "FormattedOutput.h"

#include <TimeUtils.h>

#include <SpdLogger.h>

#include <public/ClassDefHelper.h>

#include <magic_enum/magic_enum_iostream.hpp>


void FormattedOutput::LogAndPrint(const std::string & mess)
{
    SPD_L->info(mess);
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
	spdlog::info(R"({}, "{}", {}, Volume {} / {})", device->GetPnpId(), device->GetName(), magic_enum::enum_name(device->GetFlow()), device->GetCurrentRenderVolume(), device->GetCurrentCaptureVolume());
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
