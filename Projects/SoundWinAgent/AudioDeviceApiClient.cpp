#include "stdafx.h"
// ReSharper disable CppExpressionWithoutSideEffects

#include "AudioDeviceApiClient.h"

#include <public/SoundAgentInterface.h>

#include <SpdLogger.h>

#include <string>
#include <sstream>
#include <nlohmann/json.hpp>


#include <TimeUtils.h>

#include "FormattedOutput.h"
#include "HttpRequestProcessor.h"


// ReSharper disable once CppPassValueParameterByConstReference
AudioDeviceApiClient::AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor)  // NOLINT(performance-unnecessary-value-param, modernize-pass-by-value)
    : requestProcessor_(processor)  // NOLINT(performance-unnecessary-value-param)
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

    // Convert wstring parameters to UTF-8 strings for JSON
    const std::string pnpIdUtf8 = utility::conversions::to_utf8string(device->GetPnpId());
    const std::string nameUtf8 = utility::conversions::to_utf8string(device->GetName());
    const std::string hostName = GetHostName();

    auto localTimeAsString = ed::getLocalTimeAsString("T");
    localTimeAsString = localTimeAsString.substr(0, localTimeAsString.length() - 7);

    const nlohmann::json payload = {
        {"pnpId", pnpIdUtf8},
        {"name", nameUtf8},
        {"flowType", static_cast<const int>(device->GetFlow())},
        {"renderVolume", static_cast<const int>(device->GetCurrentRenderVolume())},
        {"captureVolume", static_cast<const int>(device->GetCurrentCaptureVolume())},
        {"updateDate", localTimeAsString},
        {"deviceMessageType", static_cast<const int>(eventType)},
        {"hostName", hostName}
    };

    // Convert nlohmann::json to cpprestsdk::json::value
    const web::json::value jsonPayload = web::json::value::parse(payload.dump());

    // Create HTTP client and request
    web::http::http_request request(web::http::methods::POST);
    request.set_body(jsonPayload);
    request.headers().set_content_type(U("application/json"));

    const auto hint = hintPrefix + "Post a device: " + pnpIdUtf8;
    SPD_L->info("Enqueueing: {}...", hint);
    requestProcessor_->EnqueueRequest(request, L"", hint);
    FormattedOutput::LogAndPrint("Enqueued: " + hint);
}

void AudioDeviceApiClient::PutVolumeChangeToApi(const std::wstring & pnpId, bool renderOrCapture, uint16_t volume, const std::string& hintPrefix) const
{
	const std::string pnpIdUtf8 = utility::conversions::to_utf8string(pnpId);
	auto localTimeAsString = ed::getLocalTimeAsString("T");
	localTimeAsString = localTimeAsString.substr(0, localTimeAsString.length() - 7);
	const nlohmann::json payload = {
        {"deviceMessageType", static_cast<const int>(renderOrCapture ? SoundDeviceEventType::VolumeRenderChanged : SoundDeviceEventType::VolumeCaptureChanged)},
        {"volume", static_cast<const int>(volume)},
        {"updateDate", localTimeAsString}
	};
	// Convert nlohmann::json to cpprestsdk::json::value
	const web::json::value jsonPayload = web::json::value::parse(payload.dump());
	// Create HTTP client and request
	web::http::http_request request(web::http::methods::PUT);
	request.set_body(jsonPayload);
	request.headers().set_content_type(U("application/json"));

    const auto hint = hintPrefix + "Volume change (PUT) for a device: " + pnpIdUtf8;
    SPD_L->info("Enqueueing: {}...", hint);
	// Instead of sending directly, enqueue the request in the processor

    const auto urlSuffix = std::format(L"/{}/{}", pnpId, GetHostNameW());
    requestProcessor_->EnqueueRequest(request, urlSuffix, hint);
    FormattedOutput::LogAndPrint("Enqueued: " + hint);
}

std::string AudioDeviceApiClient::GetHostName()
{
    static const std::string HOST_NAME = []() -> std::string
        {
            wchar_t hostNameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD bufferSize = std::size(hostNameBuffer);
            GetComputerNameW(hostNameBuffer, &bufferSize);
            std::wstring hostName(hostNameBuffer);
            std::ranges::transform(hostName, hostName.begin(),
                [](wchar_t c) { return std::toupper(c); });
            return utility::conversions::to_utf8string(hostNameBuffer);
        }();
    return HOST_NAME;
}

std::wstring AudioDeviceApiClient::GetHostNameW()
{
    static const std::wstring HOST_NAME = []() -> std::wstring
        {
            wchar_t hostNameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
            DWORD bufferSize = std::size(hostNameBuffer);
            GetComputerNameW(hostNameBuffer, &bufferSize);
            return std::wstring(hostNameBuffer);
        }();
    return HOST_NAME;
}

