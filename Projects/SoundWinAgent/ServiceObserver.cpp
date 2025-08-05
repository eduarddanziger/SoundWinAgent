#include "os-dependencies.h"

#include "ServiceObserver.h"

#include "ApiClient/AudioDeviceApiClient.h"
#include "ApiClient/HttpRequestProcessor.h"

#include <magic_enum/magic_enum.hpp>

#include <spdlog/spdlog.h>
#include <winternl.h>
#include <cpprest/asyncrt_utils.h>

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
    const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_, GetHostName, GetOperationSystemName);
    apiClient.PostDeviceToApi(messageType, devicePtr, hintPrefix);
}

void ServiceObserver::PutVolumeChangeToApi(const std::string & pnpId, bool renderOrCapture, uint16_t volume, const std::string & hintPrefix) const
{
	const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_, GetHostName, GetOperationSystemName);
	apiClient.PutVolumeChangeToApi(pnpId, renderOrCapture, volume, hintPrefix);
}

void ServiceObserver::PostAndPrintCollection() const
{
    spdlog::info("Processing device collection...");

    for (size_t i = 0; i < collection_.GetSize(); ++i)
    {
        const auto deviceSmartPtr(collection_.CreateItem(i));

        spdlog::info(R"({}, "{}", {}, Volume {} / {})", deviceSmartPtr->GetPnpId(), deviceSmartPtr->GetName(),
                     magic_enum::enum_name(deviceSmartPtr->GetFlow()), deviceSmartPtr->GetCurrentRenderVolume(),
                     deviceSmartPtr->GetCurrentCaptureVolume());
        if (!apiBaseUrl_.empty())
        {
            PostDeviceToApi(SoundDeviceEventType::Confirmed, deviceSmartPtr.get(), "(by iteration on device collection) ");
        }
        else
        {
            spdlog::info("No API base URL configured. Skipping API call.");
        }
    }
    spdlog::info("...Processing device collection finished.");
}

void ServiceObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string & devicePnpId)
{
    spdlog::info("Event caught: {}, device PnP id: {}.", magic_enum::enum_name(event), devicePnpId);

    const auto soundDeviceInterface = collection_.CreateItem(devicePnpId);
    if (!soundDeviceInterface)
    {
        spdlog::warn("Sound device with PnP id cannot be initialized.", devicePnpId);
        return;
    }

	//There is no SoundDeviceEventType::Confirmed processing. "Confirmed" is sent by collection initialization only
    if (event == SoundDeviceEventType::Discovered)
    {
        PostDeviceToApi(event, soundDeviceInterface.get(), "(by device discovery) ");
    }
    else if (event == SoundDeviceEventType::VolumeRenderChanged || event == SoundDeviceEventType::VolumeCaptureChanged)
    {
		const bool renderOrCapture = event == SoundDeviceEventType::VolumeRenderChanged;
        PutVolumeChangeToApi(devicePnpId, renderOrCapture, renderOrCapture ? soundDeviceInterface->GetCurrentRenderVolume() : soundDeviceInterface->GetCurrentCaptureVolume());
    }
    else if (event == SoundDeviceEventType::Detached)
    {
        // not yet implemented RemoveToApi(devicePnpId);
    }
    else
	{
        spdlog::warn("Unexpected event type: {}", static_cast<int>(event));
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

std::string ServiceObserver::GetOperationSystemName()
{
	static const std::string OS_NAME = []() -> std::string
	{
		OSVERSIONINFOEX osVersionInfo{};
		osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		typedef NTSTATUS (WINAPI *RtlGetVersionFuncT)(OSVERSIONINFOEX*);
		const auto ntDllHandle = GetModuleHandleA("ntdll.dll");
		if (ntDllHandle == nullptr)
		{
			return "Windows, no version info";
		}

		if (const auto rtlGetVersionFuncPtr =
				reinterpret_cast<RtlGetVersionFuncT>(GetProcAddress(ntDllHandle, "RtlGetVersion"));  // NOLINT(clang-diagnostic-cast-function-type-strict)
			rtlGetVersionFuncPtr == nullptr || rtlGetVersionFuncPtr(&osVersionInfo) != 0)
		{
			return "Windows, no version info";
		}

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
            return "Windows, no version info";
        }
        char productName[256] = { 0 };
        DWORD osRevision = 0;
        char displayVersion[256] = { 0 };
        char editionId[256] = { 0 };

        // Get Product Name
        DWORD size = sizeof(productName);
        if (RegQueryValueExA(hKey, "ProductName", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(productName), &size) == ERROR_SUCCESS) {
            // Successfully got product name
        }

        // Get UBR (Update Build Revision)
        size = sizeof(osRevision);
        RegQueryValueExA(hKey, "UBR", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(&osRevision), &size);

        // Get DisplayVersion (for Win11 22H2+)
        size = sizeof(displayVersion);
        RegQueryValueExA(hKey, "DisplayVersion", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(displayVersion), &size);

        // Get EditionID (Pro, Home, etc.)
        size = sizeof(editionId);
        RegQueryValueExA(hKey, "EditionID", nullptr, nullptr,
            reinterpret_cast<LPBYTE>(editionId), &size);

		RegCloseKey(hKey);

        // Windows 11 detection (build 22000+ or product name contains "Windows 11")
        bool isWindows11 = false;
        if (osVersionInfo.dwBuildNumber >= 22000 ||
            strstr(productName, "Windows 11") != nullptr)
        {
            isWindows11 = true;
        }

        // Format the version string
        std::string versionString;
        if (isWindows11)
        {
            versionString = std::string("Windows 11 ") + editionId;
        }
        else
        {
            versionString = std::string(productName);
        }

        // Add display version if available (e.g., "22H2")
        if (strlen(displayVersion) > 0) {
            versionString += " " + std::string(displayVersion);
        }

        versionString += " Build " + std::to_string(osVersionInfo.dwBuildNumber) + "." + std::to_string(osRevision);

		return versionString;
	}();
	return OS_NAME;
}
