#include "stdafx.h"

#include "ServiceObserver.h"

#include "FormattedOutput.h"
#include "AudioDeviceApiClient.h"
#include "HttpRequestProcessor.h"

#include <SpdLogger.h>
#include <magic_enum/magic_enum.hpp>

#include "public/StringUtils.h"

ServiceObserver::ServiceObserver(SoundDeviceCollectionInterface& collection,
    std::string apiBaseUrl,
    std::string universalToken,
    std::string codeSpaceName)
    : collection_(collection)
    , apiBaseUrl_(std::move(apiBaseUrl))
    , universalToken_(std::move(universalToken))
    , codeSpaceName_(std::move(codeSpaceName))
    , requestProcessorSmartPtr_(std::make_shared<HttpRequestProcessor>(apiBaseUrl_, universalToken_, codeSpaceName_))
{
}

void ServiceObserver::PostDeviceToApi(const SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr, const std::string & hintPrefix) const
{
    const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_, GetHostName);
    apiClient.PostDeviceToApi(messageType, devicePtr, hintPrefix);
}

void ServiceObserver::PutVolumeChangeToApi(const std::string & pnpId, bool renderOrCapture, uint16_t volume, const std::string & hintPrefix) const
{
	const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_, GetHostName);
	apiClient.PutVolumeChangeToApi(pnpId, renderOrCapture, volume, hintPrefix);
}

void ServiceObserver::PostAndPrintCollection() const
{
    std::string message("Processing device collection...");
    FormattedOutput::LogAndPrint(message);

    for (size_t i = 0; i < collection_.GetSize(); ++i)
    {
        const auto deviceSmartPtr(collection_.CreateItem(i));

        FormattedOutput::PrintDeviceInfo(deviceSmartPtr.get());
        if (!apiBaseUrl_.empty())
        {
            PostDeviceToApi(SoundDeviceEventType::Confirmed, deviceSmartPtr.get(), "(by iteration on device collection) ");
        }
        else
        {
            SPD_L->info("No API base URL configured. Skipping API call.");
        }
    }
    message = "...Processing device collection finished.";
    FormattedOutput::LogAndPrint(message);
}

void ServiceObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string & devicePnpId)
{
    spdlog::info("Event caught: {}, device PnP id: {}.", magic_enum::enum_name(event), devicePnpId);

	//There is no (event == SoundDeviceEventType::Confirmed). "Confirmed" is sent by collection initialization
    if (event == SoundDeviceEventType::Discovered)
    {
        const auto soundDeviceInterface = collection_.CreateItem(devicePnpId);
        PostDeviceToApi(event, soundDeviceInterface.get(), "(by device discovery) ");
    }
    else if (event == SoundDeviceEventType::VolumeRenderChanged || event == SoundDeviceEventType::VolumeCaptureChanged)
    {
        const auto soundDeviceInterface = collection_.CreateItem(devicePnpId);
		const bool renderOrCapture = event == SoundDeviceEventType::VolumeRenderChanged;
        PutVolumeChangeToApi(devicePnpId, renderOrCapture, renderOrCapture ? soundDeviceInterface->GetCurrentRenderVolume() : soundDeviceInterface->GetCurrentCaptureVolume());
    }
    else if (event == SoundDeviceEventType::Detached)
    {
        // not yet implemented RemoveToApi(devicePnpId);
    }
    else
	{
		SPD_L->warn("Unexpected event type: {}", static_cast<int>(event));
	}

}

std::string ServiceObserver::GetHostName()
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
