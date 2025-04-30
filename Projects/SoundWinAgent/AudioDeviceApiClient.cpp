#include "stdafx.h"

#include "AudioDeviceApiClient.h"

#include <iostream>
#include <public/SoundAgentInterface.h>

#include <SpdLogger.h>

#include <string>
#include <sstream>
#include <StringUtils.h>
#include <nlohmann/json.hpp>

#include "TimeUtils.h"

namespace ed
{
    template<typename Char_, typename Clock_, class Duration_ = typename Clock_::duration>
    std::basic_string<Char_> systemTimeToStringWithSystemTime(const std::chrono::time_point<Clock_, Duration_>& time, const std::basic_string<Char_>& betweenDateAndTime)
    {
        const time_t timeT = to_time_t(time);

        // ReSharper disable once CppUseStructuredBinding
        tm localTimeT{};
        if (gmtime_s(&localTimeT, &timeT) != 0)
        {
            return std::basic_string<Char_>();
        }

        const auto microsecondsFraction = chr::duration_cast<chr::microseconds>(
            time.time_since_epoch()
        ).count() % 1000000;

        std::basic_ostringstream<Char_> oss; oss
            << std::setbase(10)	// setbase is "sticky"
            << std::setfill(any_string_array<Char_>("0").data()[0]) // setfill is "sticky"
            << std::setw(4) << localTimeT.tm_year + 1900 // setw is not sticky
            << any_string_array<Char_>("-").data()
            << std::setw(2) << localTimeT.tm_mon + 1
            << any_string_array<Char_>("-").data()
            << std::setw(2) << localTimeT.tm_mday
            << betweenDateAndTime
            << std::setw(2) << localTimeT.tm_hour
            << any_string_array<Char_>(":").data()
            << std::setw(2) << localTimeT.tm_min
            << any_string_array<Char_>(":").data()
            << std::setw(2) << localTimeT.tm_sec
            << any_string_array<Char_>(".").data()
            << std::setw(6)
            << microsecondsFraction;

        return oss.str();
    }

    template<typename Clock_, class Duration_ = typename Clock_::duration>
    [[nodiscard]] std::string systemTimeAsStringWithSystemTime(const std::chrono::time_point<Clock_, Duration_>& time, const std::string& betweenDateAndTime = " ")
    {
        return systemTimeToStringWithSystemTime(time, betweenDateAndTime);
    }
}



#include "FormattedOutput.h"
#include "HttpRequestProcessor.h"


// ReSharper disable once CppPassValueParameterByConstReference
AudioDeviceApiClient::AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor, std::function<std::string()> getHostNameCallback)
    : requestProcessor_(processor)  // NOLINT(performance-unnecessary-value-param)
	, getHostNameCallback_(std::move(getHostNameCallback))
{
}

void AudioDeviceApiClient::PostDeviceToApi(SoundDeviceEventType eventType, const SoundDeviceInterface* device, const std::string& hintPrefix) const
{
    if (!device)
    {
        const auto msg = "Cannot post device data: nullptr provided";
        std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
        SPD_L->error(msg);
        return;
    }

    const std::string hostName = getHostNameCallback_();

    const auto nowTime = std::chrono::system_clock::now();
    auto nowTimeAsSystemTimeString = ed::systemTimeAsStringWithSystemTime(nowTime, "T");
	nowTimeAsSystemTimeString = nowTimeAsSystemTimeString.substr(0, nowTimeAsSystemTimeString.length() - 7);

    const nlohmann::json payload = {
        {"pnpId", ed::WString2StringTruncate(device->GetPnpId())},
        {"name", ed::WString2StringTruncate(device->GetName())},
        {"flowType", device->GetFlow()},
        {"renderVolume", device->GetCurrentRenderVolume()},
        {"captureVolume", device->GetCurrentCaptureVolume()},
        {"updateDate", nowTimeAsSystemTimeString},
        {"deviceMessageType", eventType},
        {"hostName", hostName}
    };

    // Convert nlohmann::json to string and to value
    const std::string payloadString = payload.dump();
    const auto hint = hintPrefix + "Post a device." + ed::WString2StringTruncate(device->GetPnpId());

    SPD_L->info("Enqueueing: {}...", hint);
    requestProcessor_->EnqueueRequest(true, nowTime, "", payloadString, {}, hint);
}

void AudioDeviceApiClient::PutVolumeChangeToApi(const std::string & pnpId, bool renderOrCapture, uint16_t volume, const std::string& hintPrefix) const
{
    const auto nowTime = std::chrono::system_clock::now();
    auto nowTimeAsSystemTimeString = ed::systemTimeAsStringWithSystemTime(nowTime, "T");
    nowTimeAsSystemTimeString = nowTimeAsSystemTimeString.substr(0, nowTimeAsSystemTimeString.length() - 7);

    const nlohmann::json payload = {
        {"deviceMessageType", renderOrCapture ? SoundDeviceEventType::VolumeRenderChanged : SoundDeviceEventType::VolumeCaptureChanged},
        {"volume", volume},
        {"updateDate", nowTimeAsSystemTimeString}
	};
    const std::string payloadString = payload.dump();

    const auto hint = hintPrefix + "Volume change (PUT) for a device: " + pnpId;
    SPD_L->info("Enqueueing: {}...", hint);
	// Instead of sending directly, enqueue the request in the processor

    const auto urlSuffix = std::format("/{}/{}", pnpId, getHostNameCallback_());
    requestProcessor_->EnqueueRequest(false, nowTime, urlSuffix, payloadString, {}, hint);
}

