#pragma once

#include <endpointvolume.h>
#include <set>
#include <atlbase.h>
#include <functional>

#include "public/SoundAgentInterface.h"

#include "SoundDevice.h"

#include "MultipleNotificationClient.h"


namespace ed::audio {
using EndPointVolumeSmartPtr = CComPtr<IAudioEndpointVolume>;


class SoundDeviceCollection final : public SoundDeviceCollectionInterface, protected MultipleNotificationClient {
protected:
    using TPnPIdToDeviceMap = std::map<std::wstring, SoundDevice>;
    using ProcessDeviceFunctionT =
        std::function<void(ed::audio::SoundDeviceCollection*, const std::wstring&, const SoundDevice&, EndPointVolumeSmartPtr)>;

public:
    DISALLOW_COPY_MOVE(SoundDeviceCollection);
    ~SoundDeviceCollection() override;

public:
    explicit SoundDeviceCollection(std::wstring nameFilter, bool bothHeadsetAndMicro);

    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(size_t deviceNumber) const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(const std::wstring& devicePnpId) const override;
    void Subscribe(SoundDeviceObserverInterface & observer) override;
    void Unsubscribe(SoundDeviceObserverInterface & observer) override;

public:
    HRESULT OnDeviceAdded(LPCWSTR deviceId) override;
    HRESULT OnDeviceRemoved(LPCWSTR deviceId) override;
    HRESULT OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState) override;
    HRESULT OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;

private:
    void ProcessActiveDeviceList(ProcessDeviceFunctionT processDeviceFunc);
    void RecreateActiveDeviceList();
    void RefreshVolumes();
    static void RegisterDevice(SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr endpointVolume);
    static void UpdateDeviceVolume(SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr);


    void NotifyObservers(SoundDeviceEventType action, const std::wstring & devicePNpId) const;
    [[nodiscard]] bool IsDeviceApplicable(const SoundDevice & device) const;
    bool TryCreateDeviceAndGetVolumeEndpoint(ULONG i,
                                             CComPtr<IMMDevice> deviceEndpointSmartPtr,
                                             SoundDevice & device,
                                             std::wstring & deviceId,
                                             EndPointVolumeSmartPtr & outVolumeEndpoint
    ) const;

    void TraceIt(const std::wstring & line) const;
    void TraceItDebug(const std::wstring & line) const;
    void UnregisterAllEndpointsVolumes();
    void UnregisterAndRemoveEndpointsVolumes(const std::wstring & deviceId);

    [[nodiscard]] SoundDevice MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(const SoundDevice & device) const;
    [[nodiscard]] bool CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(const SoundDevice & device, SoundDevice & unmergedDev) const;

    bool TryCreateDeviceOnId(LPCWSTR deviceId,
                                               SoundDevice & device,
                                               EndPointVolumeSmartPtr & outVolumeEndpoint
    ) const;

    static std::pair<std::vector<std::wstring>, std::vector<std::wstring>>
        GetDevicePnPIdsWithChangedVolume(const TPnPIdToDeviceMap & devicesBeforeUpdate, const TPnPIdToDeviceMap & devicesAfterUpdate);

public:
    void ResetContent() override;


private:
    std::map<std::wstring, SoundDevice> pnpToDeviceMap_;
    std::set<SoundDeviceObserverInterface*> observers_;
    IMMDeviceEnumerator * enumerator_ = nullptr;
    std::wstring nameFilter_;
    bool bothHeadsetAndMicro_;
    const std::wstring noPlugAndPlayGuid_ = L"00000000-0000-0000-FFFF-FFFFFFFFFFFF";

    std::map<std::wstring, CComPtr<IAudioEndpointVolume>> devIdToEndpointVolumes_;
};
}
