#include "stdafx.h"

#include "SoundAgentApi.h"

#include "public/SoundAgentInterface.h"
#include "ApiClient/common/ClassDefHelper.h"

class DllObserver final : public SoundDeviceObserverInterface {
public:
    explicit DllObserver(TSaaDiscover discoverCallback)
        : discoverCallback_(discoverCallback)
    {
    }
    DISALLOW_COPY_MOVE(DllObserver);
    ~DllObserver() override;

    void OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId) override;

private:
    TSaaDiscover discoverCallback_;
};

DllObserver::~DllObserver() = default;

void DllObserver::OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId)
{
    if(discoverCallback_ != nullptr)
    {
        const bool attached = event == SoundDeviceEventType::Discovered;
        // ReSharper disable once CppTooWideScopeInitStatement
        const bool detached = event == SoundDeviceEventType::Detached;

        if (attached || detached)
        {
            // SaaDescription description; // only for logging
            // strncpy_s(description.Guid, _countof(description.Guid), device->GetPnpId().c_str(), device->GetPnpId().size());
            // strncpy_s(description.Name, _countof(description.Name), device->GetName().c_str(), device->GetName().size());
            // description.Volume = 100;
            //
            discoverCallback_(attached ? TRUE : FALSE);
        }
    }
}


namespace  {
    std::unique_ptr< SoundDeviceCollectionInterface> device_collection;
    std::unique_ptr<SoundDeviceObserverInterface> device_collection_observer;
}

SaaResult AcInitialize(SaaHandle* handle, TSaaDiscover discoverCallback)
{
    device_collection = SoundAgent::CreateDeviceCollection();
    device_collection_observer = std::make_unique<DllObserver>(discoverCallback);
    device_collection->Subscribe(*device_collection_observer);

    return 0;
}

SaaResult AcGetAttached(SaaHandle handle, SaaDescription* description)
{
    if(description == nullptr)
    {
        return 0;
    }

    if (device_collection != nullptr && device_collection->GetSize() > 0)
    {
        const auto device = device_collection->CreateItem(0);
        strncpy_s(description->Guid, _countof(description->Guid), device->GetPnpId().c_str(), device->GetPnpId().size());
        strncpy_s(description->Name, _countof(description->Name), device->GetName().c_str(), device->GetName().size());
        description->Volume = device->GetCurrentRenderVolume();
        return 0;
    }
    description->Guid[0] = '\0';
    description->Name[0] = '\0';
    description->Volume = 0;
    return 0;
}

SaaResult AcUnInitialize(SaaHandle handle)
{
    if(device_collection != nullptr)
    {
        device_collection->Unsubscribe(*device_collection_observer);
        device_collection_observer.reset();
        device_collection.reset();
    }
    return 0;
}
