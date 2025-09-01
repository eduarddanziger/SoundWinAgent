// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#include "os-dependencies.h"

#define INITGUID
extern "C" const CLSID CLSID_StdGlobalInterfaceTable;
#include <initguid.h>

#include "SoundDeviceCollection.h"

#include "SoundDevice.h"
#include "Utilities.h"

#include "ApiClient/common/StringUtils.h"

#include <iostream>
#include <cstddef>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <ranges>
#include <string>
#include <valarray>

#include <magic_enum/magic_enum_iostream.hpp>
#include <spdlog/spdlog.h>


using namespace std::literals::string_literals;

namespace {
    SoundDeviceFlowType ConvertFromLowLevelFlow(const EDataFlow flow)
    {
        switch (flow)
        {
        case eRender:
            return SoundDeviceFlowType::Render;
        case eCapture:
            return SoundDeviceFlowType::Capture;
        case eAll:
            return SoundDeviceFlowType::RenderAndCapture;
        case EDataFlow_enum_count:
        default: // NOLINT(clang-diagnostic-covered-switch-default)
            return SoundDeviceFlowType::None;
        }
    }
}


ed::audio::SoundDeviceCollection::~SoundDeviceCollection()
{
    UnregisterAllEndpointsVolumes();
}

void ed::audio::SoundDeviceCollection::ResetContent()
{
    RecreateActiveDeviceList();
}

void ed::audio::SoundDeviceCollection::ActivateAndStartLoop()
{
}

void ed::audio::SoundDeviceCollection::DeactivateAndStopLoop()
{
}

size_t ed::audio::SoundDeviceCollection::GetSize() const
{
    return pnpToDeviceMap_.size();
}

std::unique_ptr<SoundDeviceInterface> ed::audio::SoundDeviceCollection::CreateItem(size_t deviceNumber) const
{
    if (deviceNumber >= pnpToDeviceMap_.size())
    {
        throw std::runtime_error("Device number is too big");
    }
    size_t i = 0;
    for (const auto & recordVal : pnpToDeviceMap_ | std::views::values)
    {
        if (i++ == deviceNumber)
        {
            return std::make_unique<SoundDevice>(recordVal);
        }
    }
    throw std::runtime_error("Device number not found");
}

std::unique_ptr<SoundDeviceInterface> ed::audio::SoundDeviceCollection::CreateItem(
    const std::string & devicePnpId) const
{
    if (!pnpToDeviceMap_.contains(devicePnpId))
    {
        return nullptr;
    }
    return std::make_unique<SoundDevice>(pnpToDeviceMap_.at(devicePnpId));
}

std::optional<std::string> ed::audio::SoundDeviceCollection::GetDefaultRenderDevicePnpId() const
{
    return defaultRenderDevicePnpId_;
}

std::optional<std::string> ed::audio::SoundDeviceCollection::GetDefaultCaptureDevicePnpId() const
{
    return defaultCaptureDevicePnpId_;
}

void ed::audio::SoundDeviceCollection::Subscribe(SoundDeviceObserverInterface & observer)
{
    observers_.insert(&observer);
}

void ed::audio::SoundDeviceCollection::Unsubscribe(SoundDeviceObserverInterface & observer)
{
    observers_.erase(&observer);
}

std::optional<std::wstring> ed::audio::SoundDeviceCollection::GetDeviceId(CComPtr<IMMDevice> deviceEndpointSmartPtr)
{
    if (!deviceEndpointSmartPtr)
        return std::nullopt;

    LPWSTR deviceIdPtr = nullptr;
    if (const HRESULT hr = deviceEndpointSmartPtr->GetId(&deviceIdPtr);
        FAILED(hr) || !deviceIdPtr)
    {
        return std::nullopt;
    }
    std::wstring deviceId(deviceIdPtr);
    CoTaskMemFree(deviceIdPtr);

    return deviceId;
}

std::string ed::audio::SoundDeviceCollection::DeviceIdToPnpIdForm(const std::string& deviceIdAscii)
{
    auto pnpId = deviceIdAscii;
    if (pnpId.length() > 79)
    {
        pnpId = pnpId.substr(0, 79);
    }
    std::erase_if(pnpId,
                  [](wchar_t c) { return c == L'{' || c == L'}'; });

    std::ranges::transform(pnpId, pnpId.begin(),
                           [](wchar_t c) { return std::toupper(c); });

    return pnpId;
}

bool ed::audio::SoundDeviceCollection::TryCreateDeviceAndGetVolumeEndpoint(
    CComPtr<IMMDevice> deviceEndpointSmartPtr,  // NOLINT(performance-unnecessary-value-param)
    SoundDevice & device,
    std::wstring & deviceId,
    EndPointVolumeSmartPtr & outVolumeEndpoint
)
{
    const auto deviceIdOpt = GetDeviceId(deviceEndpointSmartPtr);
    if (!deviceIdOpt.has_value()) {
        spdlog::warn("Failed to get device ID from IMMDevice.");
        return false;
    }
    deviceId = deviceIdOpt.value();
    auto deviceIdAscii = WString2StringTruncate(deviceId);


    HRESULT hr;
    spdlog::info(R"(Id of the current device is "{}".)", deviceIdAscii);
    // Get flow direction via IMMEndpoint
    auto flow = SoundDeviceFlowType::None;
    {
        EDataFlow lowLevelFlow;
        IMMEndpoint * pEndpoint = nullptr;
        hr = deviceEndpointSmartPtr->QueryInterface(__uuidof(IMMEndpoint), reinterpret_cast<void**>(&pEndpoint));
        if (FAILED(hr)) {
            return false;
        }
        hr = pEndpoint->GetDataFlow(&lowLevelFlow);
        SAFE_RELEASE(pEndpoint)
        if (FAILED(hr)) {
            return false;
        }
        flow = ConvertFromLowLevelFlow(lowLevelFlow);
        spdlog::info(R"(The end point device "{}", has a data flow "{}".)", deviceIdAscii,
                     magic_enum::enum_name(flow));
    }
    // Read device PnP Class id property
    std::string pnpId;
    std::string name;
    EndpointFormFactor formFactorEnum;
    {
        IPropertyStore* pProps = nullptr;
        hr = deviceEndpointSmartPtr->OpenPropertyStore(STGM_READ, &pProps);
        if (FAILED(hr)) {
            return false;
        }
        {
            PROPVARIANT propVarForName;

            PropVariantInit(&propVarForName);

            hr = pProps->GetValue(
                PKEY_Device_FriendlyName, &propVarForName);
            assert(SUCCEEDED(hr));
            if (propVarForName.vt == VT_LPWSTR)
            {
                name = Utf16ToUtf8(propVarForName.pwszVal);
                spdlog::info(R"(The end point device "{}" got a name "{}".)",
                             deviceIdAscii, name);
            }
            else
            {
                name = "UnknownDeviceName";
                spdlog::warn(
                    R"(The end point device "{}" has no friendly name not of expected type VT_LPWSTR. Assigning "{}".)",
                    deviceIdAscii, name);
            }
            // ReSharper disable once CppFunctionResultShouldBeUsed
            PropVariantClear(&propVarForName);
        }

        {
            PROPVARIANT propVarForFormFactor;

            PropVariantInit(&propVarForFormFactor);

            hr = pProps->GetValue(
                PKEY_AudioEndpoint_FormFactor, &propVarForFormFactor);
            assert(SUCCEEDED(hr));
            if (propVarForFormFactor.vt == VT_UI4)
            {
                formFactorEnum = static_cast<EndpointFormFactor>(propVarForFormFactor.ulVal);
                spdlog::info(R"(The end point device "{}" form factor is "{}")",
                    deviceIdAscii, magic_enum::enum_name(formFactorEnum));
            }
            // ReSharper disable once CppFunctionResultShouldBeUsed
            PropVariantClear(&propVarForFormFactor);
        }
        {
            PROPVARIANT propVarForGuid;
            PropVariantInit(&propVarForGuid);

            hr = pProps->GetValue(
                PKEY_Device_ContainerId, &propVarForGuid);

            assert(SUCCEEDED(hr));
            assert(propVarForGuid.vt == VT_CLSID);
            {
                WCHAR buff[80];
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto len = StringFromGUID2(
                    *propVarForGuid.puuid,
                    buff,
                    std::size(buff)
                );
                if (len >= 2)
                {
                    pnpId = WString2StringTruncate(buff);
                    if (pnpId[0] == '{')
                    {
                        pnpId = pnpId.substr(1, pnpId.length() - 2);
                    }
                }
                if (constexpr auto noPlugAndPlayGuid = "00000000-0000-0000-FFFF-FFFFFFFFFFFF"
                    ; pnpId == noPlugAndPlayGuid)
                {
                    pnpId = DeviceIdToPnpIdForm(deviceIdAscii);

                    spdlog::info(R"(The end point device "{}" has got no-plug-and-play-id {}. Assigning a simplified device id "{}" .)",
                                 deviceIdAscii, noPlugAndPlayGuid, pnpId);
                }
            }
            spdlog::info(R"(The end point device "{}", got a PnP id "{}".)",
                deviceIdAscii, pnpId);

            // ReSharper disable once CppFunctionResultShouldBeUsed
            PropVariantClear(&propVarForGuid);
        }
        SAFE_RELEASE(pProps)
    }

    // check special case: exclude render end point devices with form factor Headset
    if (formFactorEnum == EndpointFormFactor::Headset && flow == SoundDeviceFlowType::Render)
    {
        spdlog::info(R"(We exclude the render end point device "{}" with name "{}", while its form factor is Headset.)",
            deviceIdAscii, name);
        return false;
    }


    // Get IAudioEndpointVolume and volume
    outVolumeEndpoint = nullptr;
    uint16_t volume = 0;
    {
        IAudioEndpointVolume* pEndpointVolume;
        hr = deviceEndpointSmartPtr->Activate(
            __uuidof(IAudioEndpointVolume),
            CLSCTX_INPROC_SERVER,
            nullptr,
            reinterpret_cast<void**>(&pEndpointVolume)
        );
        if (SUCCEEDED(hr)) {
            outVolumeEndpoint.Attach(pEndpointVolume);
        }
    }
    // Check mute and possibly correct volume
    if (outVolumeEndpoint == nullptr) {
        spdlog::warn(R"(The end point device "{}" has no volume property.)", deviceIdAscii);
        return false;
    }
    BOOL mute;
    hr = outVolumeEndpoint->GetMute(&mute);
    if (FAILED(hr)) {
        return false;
    }
    if (mute == FALSE) {
        float currVolume = 0.0f;
        hr = outVolumeEndpoint->GetMasterVolumeLevelScalar(&currVolume);
        if (FAILED(hr)) {
            return false;
        }
        volume = static_cast<uint16_t>(lround(currVolume * 1000.0f));
        spdlog::info(R"(The end point device "{}" has a volume "{}".)", deviceIdAscii, volume);
    }
    uint16_t renderVolume = 0;
    uint16_t captureVolume = 0;

    switch (flow)
    {
    case SoundDeviceFlowType::Capture:
        captureVolume = volume;
        break;
    case SoundDeviceFlowType::Render:
        renderVolume = volume;
        break;
    case SoundDeviceFlowType::None:
    case SoundDeviceFlowType::RenderAndCapture:
        break;
    }
    device = SoundDevice(pnpId, name, flow, renderVolume, captureVolume, false, false);
    return true;
}


std::pair<std::optional<std::wstring>, std::optional<std::wstring>>
ed::audio::SoundDeviceCollection::TryGetRenderAndCaptureDefaultDeviceIds() const
{
    IMMDevice* devicePtr;

    CComPtr<IMMDevice> renderDeviceSmartPtr;
    CComPtr<IMMDevice> captureDeviceSmartPtr;
    if (GetEnumeratorOrNull() == nullptr)
    {
        return {};
    }
    {
        if (
            const auto hr = GetEnumeratorOrNull()->GetDefaultAudioEndpoint(
                eRender,
                eConsole,
                &devicePtr)
            ; SUCCEEDED(hr)
        )
        {
            renderDeviceSmartPtr.Attach(devicePtr);
        }
        else
        {
            spdlog::warn("Failed to get default render audio endpoint.");
        }
    }

    if (
        const auto hr = GetEnumeratorOrNull()->GetDefaultAudioEndpoint(
            eCapture,
            eConsole,
            &devicePtr)
        ; SUCCEEDED(hr)
    )
    {
        captureDeviceSmartPtr.Attach(devicePtr);
    }
    else
    {
        spdlog::warn("Failed to get default capture audio endpoint.");
    }

    return {GetDeviceId(renderDeviceSmartPtr), GetDeviceId(captureDeviceSmartPtr)};
}

void ed::audio::SoundDeviceCollection::UnregisterAllEndpointsVolumes()
{
    for (const auto& [deviceId, endpointVolume] : devIdToEndpointVolumes_)
    {
        // ReSharper disable once CppFunctionResultShouldBeUsed
        endpointVolume->UnregisterControlChangeNotify(this);
        spdlog::info(R"(The next end point device "{}" unregistered for notifications.)",
            WString2StringTruncate(deviceId));
    }
}

// template<class INTERFACE>
// ULONG CountRef(INTERFACE* pInterface) noexcept
// {
//     if (pInterface)
//     {
//         pInterface->AddRef();
//         return pInterface->Release();
//     }
//
//     return 0;
// }

void ed::audio::SoundDeviceCollection::UnregisterAndRemoveEndpointsVolumes(const std::wstring & deviceId)
{
    if
    (
        const auto foundPair = devIdToEndpointVolumes_.find(deviceId)
        ; foundPair != devIdToEndpointVolumes_.end()
    )
    {
        auto audioEndpointVolume = foundPair->second;
        // ReSharper disable once CppFunctionResultShouldBeUsed
        audioEndpointVolume->UnregisterControlChangeNotify(this);
        spdlog::info(R"(The end point device "{}" unregistered for notifications before removal.)",
            WString2StringTruncate(deviceId));

        //        const auto ii = CountRef(static_cast<IAudioEndpointVolume*>(audioEndpointVolume));
        audioEndpointVolume.Detach();
        devIdToEndpointVolumes_.erase(foundPair);
    }
}

ed::audio::SoundDevice ed::audio::SoundDeviceCollection::MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(
    const ed::audio::SoundDevice & device) const
{
    if
    (
        const auto foundPair = pnpToDeviceMap_.find(device.GetPnpId())
        ; foundPair != pnpToDeviceMap_.end()
    )
    {
        auto flow = device.GetFlow();
        uint16_t renderVolume = device.GetCurrentRenderVolume();
        uint16_t captureVolume = device.GetCurrentCaptureVolume();
        bool captureIsDefault = device.IsCaptureCurrentlyDefault();
        bool renderIsDefault = device.IsRenderCurrentlyDefault();


        const auto & foundDev = foundPair->second;
        if (foundDev.GetFlow() != device.GetFlow())
        {

            switch (flow)
            {
            case SoundDeviceFlowType::Capture:
                renderVolume = foundDev.GetCurrentRenderVolume();
                renderIsDefault = foundDev.IsRenderCurrentlyDefault();
                break;
            case SoundDeviceFlowType::Render:
                captureVolume = foundDev.GetCurrentCaptureVolume();
                captureIsDefault = foundDev.IsCaptureCurrentlyDefault();
            case SoundDeviceFlowType::None:
            case SoundDeviceFlowType::RenderAndCapture:
            default:  // NOLINT(clang-diagnostic-covered-switch-default)
                break;
            }

            flow = SoundDeviceFlowType::RenderAndCapture;
        }
        auto foundDevNameAsSet = Split(foundDev.GetName(), '/');

        foundDevNameAsSet.insert(device.GetName());
        return {
            device.GetPnpId(), Merge(foundDevNameAsSet, '/'),
            flow, renderVolume, captureVolume,
            renderIsDefault, captureIsDefault
        };
    }
    return device;
}

// ReSharper disable once CppPassValueParameterByConstReference
void ed::audio::SoundDeviceCollection::ProcessActiveDeviceList(const ProcessDeviceFunctionT& processDeviceFunc)
{
    HRESULT hr;
    CComPtr<IMMDeviceCollection> deviceCollectionSmartPtr;
    if (GetEnumeratorOrNull() != nullptr)
    {
        IMMDeviceCollection* deviceCollection = nullptr;
        hr = GetEnumeratorOrNull()->EnumAudioEndpoints(
            eAll, DEVICE_STATE_ACTIVE,
            &deviceCollection);
        if (FAILED(hr))
        {
            spdlog::warn("EnumAudioEndpoints failed");
            return;
        }
        spdlog::info("Audio devices enumerated.");
        deviceCollectionSmartPtr.Attach(deviceCollection);
    }
    UINT count = 0;
    hr = deviceCollectionSmartPtr->GetCount(&count);
    assert(SUCCEEDED(hr));
    for (ULONG i = 0; i < count; i++)
    {
        SoundDevice device;
        EndPointVolumeSmartPtr endPointVolumeSmartPtr;
        bool isDeviceCreated;
        std::wstring deviceId;
        {
            CComPtr<IMMDevice> endpointDeviceSmartPtr;
            {
                IMMDevice* pEndpointDevice = nullptr;
                hr = deviceCollectionSmartPtr->Item(i, &pEndpointDevice);
                if (FAILED(hr))
                {
                    spdlog::warn("Collection::Item failed.");
                    continue;
                }
                endpointDeviceSmartPtr.Attach(pEndpointDevice);
            }
            isDeviceCreated = TryCreateDeviceAndGetVolumeEndpoint(endpointDeviceSmartPtr, device, deviceId, endPointVolumeSmartPtr);
        }
        if (!isDeviceCreated)
        {
            continue;
        }
        processDeviceFunc(this, deviceId, device, endPointVolumeSmartPtr);
        spdlog::info(R"(End point {} with plug-and-play id {} processed.)", i, device.GetPnpId());
    }
}


void ed::audio::SoundDeviceCollection::RecreateActiveDeviceList()
{
    spdlog::info("Recreating audio device info list..");
    pnpToDeviceMap_.clear();

    UnregisterAllEndpointsVolumes();
    devIdToEndpointVolumes_.clear();

    auto [renderDefaultDeviceId, captureDefaultDeviceId] = TryGetRenderAndCaptureDefaultDeviceIds();

    // ReSharper disable once CppPassValueParameterByConstReference
    auto setActiveAndRegisterDeviceClosure = [renderDefaultDeviceId, captureDefaultDeviceId, this](ed::audio::SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr endpointVolume)
        {
            RegisterDevice(self, deviceId, device, endpointVolume);  // NOLINT(performance-unnecessary-value-param)

            const auto pnpId = device.GetPnpId();
            const auto foundPair = self->pnpToDeviceMap_.find(pnpId);

            if (SoundDevice* foundDevicePtr = foundPair != self->pnpToDeviceMap_.end() ? &(foundPair->second) : nullptr
                ; foundDevicePtr != nullptr)
            {
                if
                (
                    (device.GetFlow() == SoundDeviceFlowType::Render || device.GetFlow() ==
                        SoundDeviceFlowType::RenderAndCapture)
                    && renderDefaultDeviceId.has_value() && *renderDefaultDeviceId == deviceId
                )
                {
                    foundDevicePtr->SetRenderCurrentlyDefault(true);
                    defaultRenderDevicePnpId_ = pnpId;
                    spdlog::info(
                        R"(Device "{}", PnPId "{}", name "{}" detected as Render-Default and set respectively.)"
                        , WString2StringTruncate(deviceId)
                        , pnpId
                        , foundDevicePtr->GetName()
                    );
                }
                if
                (
                    (device.GetFlow() == SoundDeviceFlowType::Capture || device.GetFlow() ==
                        SoundDeviceFlowType::RenderAndCapture)
                    && captureDefaultDeviceId.has_value() && *captureDefaultDeviceId == deviceId
                )
                {
                    foundDevicePtr->SetCaptureCurrentlyDefault(true);
                    defaultCaptureDevicePnpId_ = pnpId;
                    spdlog::info(
                        R"(Device "{}", PnPId "{}", name "{}" detected as Capture-Default and set respectively.)"
                        , WString2StringTruncate(deviceId)
                        , pnpId
                        , foundDevicePtr->GetName()
                    );
                }
            }
        };
    ProcessActiveDeviceList(setActiveAndRegisterDeviceClosure);
}

void ed::audio::SoundDeviceCollection::RefreshVolumes()
{
    spdlog::info("Refreshing volumes of audio devices..");
    ProcessActiveDeviceList(&SoundDeviceCollection::UpdateDeviceVolume);
}


// ReSharper disable CppPassValueParameterByConstReference
/*static*/
void ed::audio::SoundDeviceCollection::RegisterDevice(ed::audio::SoundDeviceCollection* self, const std::wstring& deviceId, const SoundDevice& device, EndPointVolumeSmartPtr endpointVolume)  // NOLINT(performance-unnecessary-value-param)
{
    if (endpointVolume != nullptr)
    {
        // ReSharper disable once CppFunctionResultShouldBeUsed
        endpointVolume->RegisterControlChangeNotify(self);
        self->devIdToEndpointVolumes_[deviceId] = endpointVolume;
        spdlog::info(R"(The end point device "{}" registered for notifications.)",
            WString2StringTruncate(deviceId));
    }

    const auto possiblyMergedDevice = self->MergeDeviceWithExistingOneBasedOnPnpIdAndFlow(device);

    self->pnpToDeviceMap_[device.GetPnpId()] = possiblyMergedDevice;

    spdlog::info(R"(Device "{}", PnPId "{}", name "{}", flow {} merged and added to the list.)"
        , WString2StringTruncate(deviceId)
        , possiblyMergedDevice.GetPnpId()
        , possiblyMergedDevice.GetName()
        , magic_enum::enum_name(possiblyMergedDevice.GetFlow())
    );
}

void ed::audio::SoundDeviceCollection::UpdateDeviceVolume(SoundDeviceCollection* self,
                                                          [[maybe_unused]] const std::wstring& deviceId,
                                                          const SoundDevice& device, EndPointVolumeSmartPtr)
{
    // ReSharper restore CppPassValueParameterByConstReference
    const auto pnpGuid = device.GetPnpId();
    if
    (
        const auto foundPair = self->pnpToDeviceMap_.find(pnpGuid)
        ; foundPair != self->pnpToDeviceMap_.end()
    )
    {
        auto& foundDev = foundPair->second;
        if (device.GetFlow() == SoundDeviceFlowType::Render)
        {
            foundDev.SetCurrentRenderVolume(device.GetCurrentRenderVolume());
        }
        else
        {
            foundDev.SetCurrentCaptureVolume(device.GetCurrentCaptureVolume());
        }
    }
}


void ed::audio::SoundDeviceCollection::NotifyObservers(SoundDeviceEventType action, const std::string & devicePNpId) const
{
    for (auto * observer : observers_)
    {
        observer->OnCollectionChanged(action, devicePNpId);
    }
}

HRESULT ed::audio::SoundDeviceCollection::OnDeviceAdded(LPCWSTR deviceId)
{
    const HRESULT onDeviceAdded = MultipleNotificationClient::OnDeviceAdded(deviceId);
    if (onDeviceAdded == S_OK)
    {
        spdlog::info(R"(Device added: id "{}".)", WString2StringTruncate(deviceId));

        SoundDevice device;
        if
        (
            EndPointVolumeSmartPtr endPointVolumeSmartPtr;
            TryCreateDeviceOnId(deviceId, device, endPointVolumeSmartPtr)
        )
        {
            RegisterDevice(this, deviceId, device, endPointVolumeSmartPtr);

            const auto pnpId = device.GetPnpId();
            NotifyObservers(SoundDeviceEventType::Discovered, pnpId);
            if (device.IsRenderCurrentlyDefault())
            {
                NotifyObservers(SoundDeviceEventType::DefaultRenderChanged, pnpId);
                spdlog::info(R"(Device "{}", PnPId "{}", name "{}" was already Render-Default. Observers notified.)"
                    , WString2StringTruncate(deviceId)
                    , pnpId
                    , device.GetName()
                );

            }
            if (device.IsCaptureCurrentlyDefault())
            {
                NotifyObservers(SoundDeviceEventType::DefaultCaptureChanged, pnpId);
                spdlog::info(R"(Device "{}", PnPId "{}", name "{}" was already Capture-Default. Observers notified.)"
                    , WString2StringTruncate(deviceId)
                    , pnpId
                    , device.GetName()
                );
            }

        }
        spdlog::info(R"(Device adding finished: id "{}".)", WString2StringTruncate(deviceId));
    }
    return onDeviceAdded;
}

bool ed::audio::SoundDeviceCollection::CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(
    const SoundDevice & device, SoundDevice & unmergedDev) const
{
    unmergedDev = {
        device.GetPnpId(), device.GetName(),
        SoundDeviceFlowType::None, device.GetCurrentRenderVolume(), device.GetCurrentCaptureVolume(),
        device.IsRenderCurrentlyDefault(), device.IsCaptureCurrentlyDefault()
    };

    if
    (
        const auto foundPair = pnpToDeviceMap_.find(device.GetPnpId())
        ; foundPair != pnpToDeviceMap_.end()
    )
    {
        auto flow = device.GetFlow();
        auto name = device.GetName();

        const auto & foundDev = foundPair->second;
        if
        (
            foundDev.GetFlow() == flow
        )
        {
            return true;
        }

        if
        (
            foundDev.GetFlow() == SoundDeviceFlowType::RenderAndCapture
        )
        {
            uint16_t renderVolume = foundDev.GetCurrentRenderVolume();
            uint16_t captureVolume = foundDev.GetCurrentCaptureVolume();
            bool renderIsDefault = foundDev.IsRenderCurrentlyDefault();
            bool captureIsDefault = foundDev.IsCaptureCurrentlyDefault();

            switch (flow)
            {
            case SoundDeviceFlowType::Capture:
                flow = SoundDeviceFlowType::Render;
                captureVolume = 0;
                captureIsDefault = false;
                break;
            case SoundDeviceFlowType::Render:
                flow = SoundDeviceFlowType::Capture;
                renderVolume = 0;
                renderIsDefault = false;
                break;
            case SoundDeviceFlowType::None:
            case SoundDeviceFlowType::RenderAndCapture:
                break;
            }

            // ReSharper disable once CppTooWideScopeInitStatement
            const auto foundDevNameAsSet = Split(foundDev.GetName(), '/');
            for (const auto & elem : foundDevNameAsSet)
            {
                if (elem != name)
                {
                    name = elem;
                    break;
                }
            }
            unmergedDev = { device.GetPnpId(), name, flow,
                renderVolume, captureVolume,
                renderIsDefault, captureIsDefault };
            return true;
        }
    }
    return false;
}


HRESULT ed::audio::SoundDeviceCollection::OnDeviceRemoved(LPCWSTR deviceId)
{
    using magic_enum::iostream_operators::operator<<; // out-of-the-box stream operators for enums

    const HRESULT hr = MultipleNotificationClient::OnDeviceRemoved(deviceId);
    if (hr == S_OK)
    {
        spdlog::info(R"(Device to remove: id "{}".)", WString2StringTruncate(deviceId));

        SoundDevice removedDeviceToUnmerge;
        if
        (   EndPointVolumeSmartPtr volumeEndpointSmartPtr;
            TryCreateDeviceOnId(deviceId, removedDeviceToUnmerge, volumeEndpointSmartPtr)
        )
        {
            spdlog::info(R"(Device to remove, more info: name "{}", flow: {}, plug-and-play id: {}.)",
                         removedDeviceToUnmerge.GetName(), magic_enum::enum_name(removedDeviceToUnmerge.GetFlow()),
                         removedDeviceToUnmerge.GetPnpId());

            if (SoundDevice possiblyUnmergedDevice; 
                CheckRemovalAndUnmergeDeviceFromExistingOneBasedOnPnpIdAndFlow(removedDeviceToUnmerge, possiblyUnmergedDevice))
            {
                if (possiblyUnmergedDevice.GetFlow() == SoundDeviceFlowType::None)
                {
                    pnpToDeviceMap_.erase(possiblyUnmergedDevice.GetPnpId());
                }
                else
                {
                    spdlog::info(R"(Removed device unmerged: name "{}", flow: {}.)", possiblyUnmergedDevice.GetName(), magic_enum::enum_name(possiblyUnmergedDevice.GetFlow()));

                    pnpToDeviceMap_[possiblyUnmergedDevice.GetPnpId()] = possiblyUnmergedDevice;
                }
                UnregisterAndRemoveEndpointsVolumes(deviceId);
                NotifyObservers(SoundDeviceEventType::Detached, removedDeviceToUnmerge.GetPnpId());
            }
        }
        spdlog::info(R"(Device removal finished: id "{}".)", WString2StringTruncate(deviceId));
    }
    return hr;
}

bool ed::audio::SoundDeviceCollection::TryCreateDeviceOnId(
    LPCWSTR deviceId,
    SoundDevice& device,
    EndPointVolumeSmartPtr& outVolumeEndpoint
) const {
    CComPtr<IMMDevice> deviceSmartPtr;
    // Retrieve the device using the device ID
    if (GetEnumeratorOrNull() != nullptr)
    {
        IMMDevice* devicePtr = nullptr;
        if (
            const auto hr = GetEnumeratorOrNull()->GetDevice(deviceId, &devicePtr)
            ; FAILED(hr)
        )
        {
            return false;
        }
        deviceSmartPtr.Attach(devicePtr);
    }
    std::wstring devId;
    return TryCreateDeviceAndGetVolumeEndpoint(deviceSmartPtr, device, devId, outVolumeEndpoint);
}

std::pair<std::vector<std::string>, std::vector<std::string>>
ed::audio::SoundDeviceCollection::GetDevicePnPIdsWithChangedVolume(
    const TPnPIdToDeviceMap & devicesBeforeUpdate, const TPnPIdToDeviceMap & devicesAfterUpdate)
{
    std::vector<std::string> diffRender;
    std::vector<std::string> diffCapture;
    for (const auto & [pnpIdInBeforeList, deviceInBeforeList] : devicesBeforeUpdate)
    {
        if (auto foundInAfterList = devicesAfterUpdate.find(pnpIdInBeforeList);
            foundInAfterList != devicesAfterUpdate.end())
        {
            const auto & deviceInAfterList = foundInAfterList->second;
            if (deviceInBeforeList.GetCurrentRenderVolume() != deviceInAfterList.GetCurrentRenderVolume())
            {
                diffRender.push_back(pnpIdInBeforeList);
            }
            if (deviceInBeforeList.GetCurrentCaptureVolume() != deviceInAfterList.GetCurrentCaptureVolume())
            {
                diffCapture.push_back(pnpIdInBeforeList);
            }
        }
    }
    return { diffRender, diffCapture };
}

HRESULT ed::audio::SoundDeviceCollection::OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState)
{
    HRESULT hr = MultipleNotificationClient::OnDeviceStateChanged(deviceId, dwNewState);
    assert(SUCCEEDED(hr));

    switch (dwNewState)
    {
    case DEVICE_STATE_ACTIVE:
        hr = OnDeviceAdded(deviceId);
        break;
    case DEVICE_STATE_DISABLED:
    case DEVICE_STATE_NOTPRESENT:
    case DEVICE_STATE_UNPLUGGED:
        hr = OnDeviceRemoved(deviceId);
        break;
    default: ;
    }

    return hr;
}

HRESULT ed::audio::SoundDeviceCollection::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    const HRESULT hResult = MultipleNotificationClient::OnNotify(pNotify);
    const auto copy = pnpToDeviceMap_;

    RefreshVolumes();

    const auto [diffRender, diffCapture] = GetDevicePnPIdsWithChangedVolume(copy, pnpToDeviceMap_);

    for (
        const auto& currPnPId : diffRender)
    {
        NotifyObservers(SoundDeviceEventType::VolumeRenderChanged, currPnPId);
    }

    for (
        const auto& currPnPId : diffCapture)
    {
        NotifyObservers(SoundDeviceEventType::VolumeCaptureChanged, currPnPId);
    }

    return hResult;
}

HRESULT ed::audio::SoundDeviceCollection::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR defaultDeviceId)
{
    const HRESULT hr = MultipleNotificationClient::OnDefaultDeviceChanged(flow, role, defaultDeviceId);
    assert(SUCCEEDED(hr));

    if (role != eConsole)
    {
        return hr;
    }

    // clear previous default device
    if (flow == eRender && defaultRenderDevicePnpId_.has_value())
    {
        const auto pnpGuid = *defaultRenderDevicePnpId_;
        const auto foundPair = pnpToDeviceMap_.find(pnpGuid);
        if (SoundDevice* foundDevicePtr = foundPair != pnpToDeviceMap_.end() ? &(foundPair->second) : nullptr
            ; foundDevicePtr != nullptr)
        {
            foundDevicePtr->SetRenderCurrentlyDefault(false);
        }
    }
    else if (flow == eCapture && defaultCaptureDevicePnpId_.has_value())
    {
        const auto pnpGuid = *defaultCaptureDevicePnpId_;
        const auto foundPair = pnpToDeviceMap_.find(pnpGuid);
        if (SoundDevice* foundDevicePtr = foundPair != pnpToDeviceMap_.end() ? &(foundPair->second) : nullptr
            ; foundDevicePtr != nullptr)
        {
            foundDevicePtr->SetCaptureCurrentlyDefault(false);
        }
    }

    // default device disabled 
    if (defaultDeviceId == nullptr)
    {
        if (flow == eRender)
        {
            defaultRenderDevicePnpId_ = std::nullopt;
            NotifyObservers(SoundDeviceEventType::DefaultRenderChanged, "");
            spdlog::info("Render-Default device removed.");
        }
        else if (flow == eCapture)
        {
            defaultCaptureDevicePnpId_ = std::nullopt;
            NotifyObservers(SoundDeviceEventType::DefaultCaptureChanged, "");
            spdlog::info("Capture-Default device removed.");
        }
        return hr;
    }

    // got new default device 
    SoundDevice device;
    if (
        EndPointVolumeSmartPtr endPointVolumeSmartPtr;
        TryCreateDeviceOnId(defaultDeviceId, device, endPointVolumeSmartPtr)
    )
    {
        const auto pnpId = device.GetPnpId();
        const auto foundPair = pnpToDeviceMap_.find(pnpId);

        if (SoundDevice* foundDevicePtr = foundPair != pnpToDeviceMap_.end() ? &(foundPair->second) : nullptr
            ; foundDevicePtr != nullptr)
        {
            if (flow == eRender)
            {
                foundDevicePtr->SetRenderCurrentlyDefault(true);
                SetDefaultRenderDeviceAndNotifyObservers(pnpId);
                spdlog::info(R"(Device "{}", PnPId "{}", name "{}" set as Render-Default according to Default-Change-Event. Observers notified.)"
                    , WString2StringTruncate(defaultDeviceId)
                    , pnpId
                    , foundDevicePtr->GetName()
                );
            }
            else if (flow == eCapture)
            {
                foundDevicePtr->SetCaptureCurrentlyDefault(true);
                SetDefaultCaptureDeviceAndNotifyObservers(pnpId);
                spdlog::info(R"(Device "{}", PnPId "{}", name "{}" set as Capture-Default according to Default-Change-Event. Observers notified.)"
                    , WString2StringTruncate(defaultDeviceId)
                    , pnpId
                    , foundDevicePtr->GetName()
                );
            }
        }
    }
    else
    {
        if (flow == eRender)
        {
            defaultRenderDevicePnpId_ = std::nullopt;
            NotifyObservers(SoundDeviceEventType::DefaultRenderChanged, "");

        }
        else if (flow == eCapture)
        {
            defaultCaptureDevicePnpId_ = std::nullopt;
            NotifyObservers(SoundDeviceEventType::DefaultCaptureChanged, "");
        }
    }

    return hr;
}

void ed::audio::SoundDeviceCollection::SetDefaultRenderDeviceAndNotifyObservers(const std::string& pnpId)
{
    defaultRenderDevicePnpId_ = pnpId;
    NotifyObservers(SoundDeviceEventType::DefaultRenderChanged, pnpId);
    if (defaultCaptureDevicePnpId_ == defaultRenderDevicePnpId_)
    {
        NotifyObservers(SoundDeviceEventType::DefaultCaptureChanged, pnpId);
    }
}

void ed::audio::SoundDeviceCollection::SetDefaultCaptureDeviceAndNotifyObservers(const std::string& pnpId)
{
    defaultCaptureDevicePnpId_ = pnpId;
    NotifyObservers(SoundDeviceEventType::DefaultCaptureChanged, pnpId);
    if (defaultRenderDevicePnpId_ == defaultCaptureDevicePnpId_)
    {
        NotifyObservers(SoundDeviceEventType::DefaultRenderChanged, pnpId);
    }
}
