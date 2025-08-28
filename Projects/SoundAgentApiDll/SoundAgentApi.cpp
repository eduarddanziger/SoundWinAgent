#include "stdafx.h"

#include "SoundAgentApi.h"

#include "public/SoundAgentInterface.h"
#include "ApiClient/common/ClassDefHelper.h"

#include <algorithm>
#include <crtdbg.h>
#include <intsafe.h>

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

void DllObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId)
{
    if(defaultRenderChangedCallback_ != nullptr)
    {
        if (event == SoundDeviceEventType::DefaultRenderChanged
            || event == SoundDeviceEventType::VolumeRenderChanged
            || event == SoundDeviceEventType::VolumeCaptureChanged)
        {
            defaultRenderChangedCallback_(devicePnpId.empty() ? FALSE : TRUE);
        }
    }
    if (defaultCaptureChangedCallback_ != nullptr)
    {
        if (event == SoundDeviceEventType::DefaultCaptureChanged
            || event == SoundDeviceEventType::VolumeCaptureChanged
            || event == SoundDeviceEventType::VolumeRenderChanged)
        {
            defaultCaptureChangedCallback_(devicePnpId.empty() ? FALSE : TRUE);
        }
    }
}


namespace  {
    std::unique_ptr< SoundDeviceCollectionInterface> device_collection;
    std::unique_ptr<SoundDeviceObserverInterface> device_collection_observer;
}

SaaResult SaaInitialize(SaaHandle* handle)
{
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
#ifdef _DEBUG
                    class RemoveDebugThresholdGuardRaii {
                    public:
                        explicit RemoveDebugThresholdGuardRaii() {
                            _CrtSetDebugFillThreshold(0);
                        }
                        ~RemoveDebugThresholdGuardRaii() {
                            _CrtSetDebugFillThreshold(SIZE_T_MAX);
                        }
                    };
                    RemoveDebugThresholdGuardRaii fillGuard;
#endif
                    strncpy_s(description->PnpId, _countof(description->PnpId), devicePnpId.c_str(),
                        devicePnpId.size());
                    strncpy_s(description->Name, _countof(description->Name), deviceName.c_str(),
                        deviceName.size());
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
