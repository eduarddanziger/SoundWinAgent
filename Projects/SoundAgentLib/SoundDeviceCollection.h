﻿#pragma once

#include <endpointvolume.h>
#include <set>
#include <map>
#include <atlbase.h>
#include <functional>

#include "public/SoundAgentInterface.h"

#include "SoundDevice.h"

#include "MultipleNotificationClient.h"


namespace ed::audio {
using EndPointVolumeSmartPtr = CComPtr<IAudioEndpointVolume>;


class SoundDeviceCollection final : public SoundDeviceCollectionInterface, protected MultipleNotificationClient {
protected:
    using TPnPIdToDeviceMap = std::map<std::string, SoundDevice>;
    using ProcessDeviceFunctionT =
        std::function<void(ed::audio::SoundDeviceCollection*, const std::wstring&, const SoundDevice&, EndPointVolumeSmartPtr)>;

public:
    DISALLOW_COPY_MOVE(SoundDeviceCollection);
    ~SoundDeviceCollection() override;

public:
    explicit SoundDeviceCollection(bool bothHeadsetAndMicro, std::function<void()> wainFunc);

    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(size_t deviceNumber) const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(const std::string& devicePnpId) const override;

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


    void NotifyObservers(SoundDeviceEventType action, const std::string & devicePNpId) const;
    [[nodiscard]] bool IsDeviceApplicable(const SoundDevice & device) const;
    static bool TryCreateDeviceAndGetVolumeEndpoint(ULONG i,
                                                    CComPtr<IMMDevice> deviceEndpointSmartPtr,
                                                    SoundDevice & device,
                                                    std::wstring & deviceId,
                                                    EndPointVolumeSmartPtr & outVolumeEndpoint
    );

    void UnregisterAllEndpointsVolumes();
    void UnregisterAndRemoveEndpointsVolumes(const std::wstring & deviceId);

    [[nodiscard]] SoundDevice MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(const SoundDevice & device) const;
    [[nodiscard]] bool CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(const SoundDevice & device, SoundDevice & unmergedDev) const;

    bool TryCreateDeviceOnId(LPCWSTR deviceId,
                                               SoundDevice & device,
                                               EndPointVolumeSmartPtr & outVolumeEndpoint
    ) const;

    static std::pair<std::vector<std::string>, std::vector<std::string>>
        GetDevicePnPIdsWithChangedVolume(const TPnPIdToDeviceMap & devicesBeforeUpdate, const TPnPIdToDeviceMap & devicesAfterUpdate);

public:
    void ResetContent() override;
    void ActivateAndStartLoop() override;
    void DeactivateAndStopLoop() override;

private:
    std::map<std::string, SoundDevice> pnpToDeviceMap_;
    std::set<SoundDeviceObserverInterface*> observers_;
    IMMDeviceEnumerator * enumerator_ = nullptr;
    bool bothHeadsetAndMicro_;
    std::function<void()> wainFunc_;
    const std::string noPlugAndPlayGuid_ = "00000000-0000-0000-FFFF-FFFFFFFFFFFF";

    std::map<std::wstring, CComPtr<IAudioEndpointVolume>> devIdToEndpointVolumes_;
};
}
