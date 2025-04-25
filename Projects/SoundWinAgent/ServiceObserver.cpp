#include "stdafx.h"

#include "ServiceObserver.h"

#include "FormattedOutput.h"
#include "AudioDeviceApiClient.h"
#include "HttpRequestProcessor.h"

#include <SpdLogger.h>
#include <StringUtils.h>

ServiceObserver::ServiceObserver(SoundDeviceCollectionInterface& collection,
    std::wstring apiBaseUrl,
    std::wstring universalToken,
    std::wstring codespaceName) // Added codespaceName parameter
    : collection_(collection)
    , apiBaseUrl_(std::move(apiBaseUrl))
    , universalToken_(std::move(universalToken))
    , codespaceName_(std::move(codespaceName)) // Initialize new member
    , requestProcessorSmartPtr_(std::make_shared<HttpRequestProcessor>(apiBaseUrl_, universalToken_, codespaceName_))
{
}

void ServiceObserver::PostDeviceToApi(const SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr, const std::string & hintPrefix) const
{
    const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_);
    apiClient.PostDeviceToApi(messageType, devicePtr, hintPrefix);
    apiClient.PostDeviceToApi(messageType, devicePtr, hintPrefix + "(copy) ");
}

void ServiceObserver::PutVolumeChangeToApi(const std::wstring & pnpId, bool renderOrCapture, uint16_t volume, const std::string & hintPrefix) const
{
	const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_);
	apiClient.PutVolumeChangeToApi(pnpId, renderOrCapture, volume, hintPrefix);
	apiClient.PutVolumeChangeToApi(pnpId, renderOrCapture, volume, hintPrefix + "(copy) ");
}

void ServiceObserver::PostAndPrintCollection() const
{
    std::string message("Processing device collection...");
    FormattedOutput::LogAndPrint(message);

    for (size_t i = 0; i < collection_.GetSize(); ++i)
    {
        const std::unique_ptr deviceSmartPtr(collection_.CreateItem(i));

        FormattedOutput::PrintDeviceInfo(deviceSmartPtr.get());
        if (!apiBaseUrl_.empty())
        {
            PostDeviceToApi(SoundDeviceEventType::Confirmed, deviceSmartPtr.get(), "(by iteration on collection) ");
        }
        else
        {
            SPD_L->info("No API base URL configured. Skipping API call.");
        }
    }
    message = "...Processing device collection finished.";
    FormattedOutput::LogAndPrint(message);
    std::cout
        << '\n'
        << FormattedOutput::CurrentLocalTimeWithoutDate << "Press Ctrl-C to stop and finish the application\n"
        << FormattedOutput::CurrentLocalTimeWithoutDate << "-----------------------------------------------\n";
}

void ServiceObserver::OnCollectionChanged(SoundDeviceEventType event, const std::wstring & devicePnpId)
{
    FormattedOutput::PrintEvent(event, devicePnpId);

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
    /*
    else if (event == SoundDeviceEventType::Removed)
    {
        RemoveToApi(devicePnpId);
    }
    else
	{
		SPD_L->warn("Unknown event type: {}", static_cast<int>(event));
	}
	*/

}

void ServiceObserver::OnTrace(const std::wstring & line)
{
    SPD_L->info(ed::WString2StringTruncate(line));
}
