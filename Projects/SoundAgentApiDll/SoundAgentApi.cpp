#include "stdafx.h"

#include "SoundAgentApi.h"

#include "public/SoundAgentInterface.h"
#include "ApiClient/common/ClassDefHelper.h"


class DllObserver final : public SoundDeviceObserverInterface {
public:
    explicit DllObserver(TSaaDefaultRenderChangedCallback defaultRenderChangedCallback)
        : defaultRenderChangedCallback_(defaultRenderChangedCallback)
    {
    }
    DISALLOW_COPY_MOVE(DllObserver);
    ~DllObserver() override;

    void OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId) override;

private:
    TSaaDefaultRenderChangedCallback defaultRenderChangedCallback_;
};

DllObserver::~DllObserver() = default;

void DllObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId)
{
    if(defaultRenderChangedCallback_ != nullptr)
    {
        if (event == SoundDeviceEventType::DefaultRenderChanged
            || event == SoundDeviceEventType::VolumeRenderChanged)
        {
            defaultRenderChangedCallback_(devicePnpId.empty() ? FALSE : TRUE);
        }
    }
}


namespace  {
    std::unique_ptr< SoundDeviceCollectionInterface> device_collection;
    std::unique_ptr<SoundDeviceObserverInterface> device_collection_observer;
}

SaaResult SaaInitialize(SaaHandle* handle, TSaaDefaultRenderChangedCallback defaultRenderChangedCallback)
{
    device_collection = SoundAgent::CreateDeviceCollection();
    device_collection_observer = std::make_unique<DllObserver>(defaultRenderChangedCallback);
    device_collection->Subscribe(*device_collection_observer);
    device_collection->ResetContent();
    
    *handle = reinterpret_cast<SaaHandle>(device_collection.get());

    return 0;
}

SaaResult SaaGetDefaultRender(SaaHandle handle, SaaDescription* description)
{
    if(description == nullptr)
    {
        return 0;
    }

    if (const auto pnpId = device_collection->GetDefaultRenderDevicePnpId()
        ; pnpId.has_value())
    {
        const auto device = device_collection->CreateItem(*pnpId);
        strncpy_s(description->PnpId, _countof(description->PnpId), device->GetPnpId().c_str(), device->GetPnpId().size());
        strncpy_s(description->Name, _countof(description->Name), device->GetName().c_str(), device->GetName().size());
        description->RenderVolume = device->GetCurrentRenderVolume();
        description->CaptureVolume = device->GetCurrentCaptureVolume();
        return 0;
    }
    description->PnpId[0] = '\0';
    description->Name[0] = '\0';
    description->RenderVolume = 0;
    description->CaptureVolume = 0;
    return 0;
}

SaaResult SaaUnInitialize(SaaHandle handle)
{
    if(device_collection != nullptr)
    {
        device_collection->Unsubscribe(*device_collection_observer);
        device_collection_observer.reset();
        device_collection.reset();
    }
    return 0;
}
