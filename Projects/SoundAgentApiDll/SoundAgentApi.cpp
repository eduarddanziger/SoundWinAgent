#include "stdafx.h"

#include "SoundAgentApi.h"

#include "public/SoundAgentInterface.h"
#include "ApiClient/common/ClassDefHelper.h"

#include "VersionInformation.h"

#include <algorithm>
#include <crtdbg.h>
#include <intsafe.h>

#include "ApiClient/common/SpdLogger/Logger.h"

class DllObserver final : public SoundDeviceObserverInterface {
public:
    explicit DllObserver(TSaaDefaultChangedCallback defaultRenderChangedCallback
        , TSaaDefaultChangedCallback defaultCaptureChangedCallback)
        : defaultRenderChangedCallback_(defaultRenderChangedCallback)
        , defaultCaptureChangedCallback_(defaultCaptureChangedCallback)
    {
    }
    DISALLOW_COPY_MOVE(DllObserver);
    ~DllObserver() override;

    void OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId) override;

private:
    TSaaDefaultChangedCallback defaultRenderChangedCallback_;
    TSaaDefaultChangedCallback defaultCaptureChangedCallback_;
};

DllObserver::~DllObserver() = default;

namespace {
    std::unique_ptr< SoundDeviceCollectionInterface> device_collection;
    std::unique_ptr<SoundDeviceObserverInterface> device_collection_observer;
}


void DllObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId)
{
    if (defaultRenderChangedCallback_ != nullptr)
    {
        if (event == SoundDeviceEventType::DefaultRenderChanged)
        {
            defaultRenderChangedCallback_(devicePnpId.empty() ? FALSE : TRUE);
        }
        if (
            (event == SoundDeviceEventType::VolumeRenderChanged || event == SoundDeviceEventType::VolumeCaptureChanged)
            && device_collection != nullptr && device_collection->GetDefaultRenderDevicePnpId() == devicePnpId
        )
        {
            defaultRenderChangedCallback_(TRUE);
        }
    }
    if (defaultCaptureChangedCallback_ != nullptr)
    {
        if (event == SoundDeviceEventType::DefaultCaptureChanged)
        {
            defaultCaptureChangedCallback_(devicePnpId.empty() ? FALSE : TRUE);
        }
        if (
            (event == SoundDeviceEventType::VolumeCaptureChanged || event == SoundDeviceEventType::VolumeRenderChanged)
            && device_collection != nullptr && device_collection->GetDefaultCaptureDevicePnpId() == devicePnpId
        )
        {
            defaultCaptureChangedCallback_(TRUE);
        }
    }
}


namespace  {
    TSaaGotLogMessageCallback got_log_message_callback = nullptr;

    void LoggerMessageBridge(const std::string& level, const std::string& message)
    {
        if (got_log_message_callback == nullptr) return;
        SaaLogMessage out{}; // zero-init
        strncpy_s(out.Level, _countof(out.Level), level.c_str(), _TRUNCATE);
        strncpy_s(out.Content, _countof(out.Content), message.c_str(), _TRUNCATE);
        got_log_message_callback(out);
    }

    void SetUpLog
        (
        TSaaGotLogMessageCallback gotLogMessageCallback,
        const CHAR* appName,
        const CHAR* appVersion
        )
    {
        const auto appNameString = appName != nullptr ? std::string(appName) : std::string(RESOURCE_FILENAME_ATTRIBUTE);
        const auto appVersionString = appVersion != nullptr ? std::string(appVersion) : std::string(PRODUCT_VERSION_ATTRIBUTE);

        got_log_message_callback = gotLogMessageCallback;

        ed::model::Logger::Inst()
            .ConfigureAppNameAndVersion(appNameString, appVersionString)
            .SetOutputToConsole(false);
        if (gotLogMessageCallback != nullptr)
        {
            ed::model::Logger::Inst()
                .SetMessageCallback(
                    LoggerMessageBridge
                );
        }

    }

}

SaaResult SaaInitialize(SaaHandle* handle,
    TSaaGotLogMessageCallback gotLogMessageCallback,
    const CHAR* appName,
    const CHAR* appVersion
)
{
    SetUpLog(gotLogMessageCallback, appName, appVersion);

    device_collection = SoundAgent::CreateDeviceCollection();
    *handle = reinterpret_cast<SaaHandle>(device_collection.get());

    return 0;
}

SaaResult SaaRegisterCallbacks([[maybe_unused]] SaaHandle handle
    , TSaaDefaultChangedCallback defaultRenderChangedCallback
    , TSaaDefaultChangedCallback defaultCaptureChangedCallback
)
{
    if (device_collection_observer != nullptr)
    {
        device_collection->Unsubscribe(*device_collection_observer);
    }

    device_collection_observer = std::make_unique<DllObserver>(defaultRenderChangedCallback,
        defaultCaptureChangedCallback);
    device_collection->Subscribe(*device_collection_observer);
    device_collection->ResetContent();

    return 0;
}

namespace
{
    SaaResult GetDeviceOnPnpId(SaaDescription* description, const std::optional<std::string>& pnpId);
}


SaaResult SaaGetDefaultRender([[maybe_unused]] SaaHandle handle, SaaDescription* description)
{
    if (description == nullptr)
    {
        return 0;
    }
    const auto pnpId = device_collection->GetDefaultRenderDevicePnpId();

    return GetDeviceOnPnpId(description, pnpId);
}

SaaResult SaaGetDefaultCapture([[maybe_unused]] SaaHandle handle, SaaDescription* description)
{
    if (description == nullptr)
    {
        return 0;
    }
    const auto pnpId = device_collection->GetDefaultCaptureDevicePnpId();

    return GetDeviceOnPnpId(description, pnpId);
}

namespace
{
    SaaResult GetDeviceOnPnpId(SaaDescription* description, const std::optional<std::string>& pnpId)
    {
        std::ranges::fill(description->PnpId, '\0');
        std::ranges::fill(description->Name, '\0');
        description->IsRender = FALSE;
        description->IsCapture = FALSE;
        description->RenderVolume = 0;
        description->CaptureVolume = 0;
        if (pnpId.has_value())
        {
            if (const auto device = device_collection->CreateItem(*pnpId)
                ; device != nullptr)
            {
                {

                    const auto devicePnpId = device->GetPnpId();
                    const auto deviceName = device->GetName();
                    strncpy_s(description->PnpId, _countof(description->PnpId), devicePnpId.c_str(),_TRUNCATE);
                    strncpy_s(description->Name, _countof(description->Name), deviceName.c_str(), _TRUNCATE);
                }
                description->IsRender = device->GetFlow() == SoundDeviceFlowType::Render || device->GetFlow() ==
                                        SoundDeviceFlowType::RenderAndCapture
                                            ? TRUE
                                            : FALSE;
                description->IsCapture = device->GetFlow() == SoundDeviceFlowType::Capture || device->GetFlow() ==
                                         SoundDeviceFlowType::RenderAndCapture
                                             ? TRUE
                                             : FALSE;
                description->RenderVolume = device->GetCurrentRenderVolume();
                description->CaptureVolume = device->GetCurrentCaptureVolume();
            }
            return 0;
        }
        return 0;
    }
}


SaaResult SaaUnInitialize(SaaHandle handle)
{
    if(device_collection != nullptr)
    {
        if (device_collection_observer != nullptr)
        {
            device_collection->Unsubscribe(*device_collection_observer);
            device_collection_observer.reset();
        }
        device_collection.reset();
    }
    return 0;
}
