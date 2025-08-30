#pragma once

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
    SoundDeviceCollection() = default;

    [[nodiscard]] size_t GetSize() const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(size_t deviceNumber) const override;
    [[nodiscard]] std::unique_ptr<SoundDeviceInterface> CreateItem(const std::string& devicePnpId) const override;

    [[nodiscard]] std::optional<std::string> GetDefaultRenderDevicePnpId() const override;
    [[nodiscard]] std::optional<std::string> GetDefaultCaptureDevicePnpId() const override;

    void Subscribe(SoundDeviceObserverInterface & observer) override;
    void Unsubscribe(SoundDeviceObserverInterface & observer) override;

public:
    HRESULT OnDeviceAdded(LPCWSTR deviceId) override;
    HRESULT OnDeviceRemoved(LPCWSTR deviceId) override;
    HRESULT OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState) override;
    HRESULT OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;
    HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR defaultDeviceId) override;

private:
    void SetDefaultRenderDeviceAndNotifyObservers(const std::string& pnpId);
    void SetDefaultCaptureDeviceAndNotifyObservers(const std::string& pnpId);

    void ProcessActiveDeviceList(const ProcessDeviceFunctionT& processDeviceFunc);
    [[nodiscard]] std::pair<std::optional<std::wstring>, std::optional<std::wstring>> TryGetRenderAndCaptureDefaultDeviceIds() const;

    void RecreateActiveDeviceList();
    void RefreshVolumes();
    static void RegisterDevice(SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr endpointVolume);
    static void UpdateDeviceVolume(SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr);


    void NotifyObservers(SoundDeviceEventType action, const std::string & devicePNpId) const;
    static bool TryCreateDeviceAndGetVolumeEndpoint(
        CComPtr<IMMDevice> deviceEndpointSmartPtr,
        SoundDevice& device,
        std::wstring& deviceId,
        EndPointVolumeSmartPtr& outVolumeEndpoint
    );

    static std::optional<std::wstring> GetDeviceId(CComPtr<IMMDevice> deviceEndpointSmartPtr);
    static std::string DeviceIdToPnpIdForm(const std::string& deviceIdAscii);

    void UnregisterAllEndpointsVolumes();
    void UnregisterAndRemoveEndpointsVolumes(const std::wstring& deviceId);

    [[nodiscard]] SoundDevice MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(const SoundDevice& device) const;
    [[nodiscard]] bool CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(
        const SoundDevice& device, SoundDevice& unmergedDev) const;

    bool TryCreateDeviceOnId(LPCWSTR deviceId,
                             SoundDevice& device,
                             EndPointVolumeSmartPtr& outVolumeEndpoint
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

    std::map<std::wstring, CComPtr<IAudioEndpointVolume>> devIdToEndpointVolumes_;

    std::optional<std::string> defaultRenderDevicePnpId_;
    std::optional<std::string> defaultCaptureDevicePnpId_;
};
}
