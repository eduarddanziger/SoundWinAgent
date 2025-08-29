// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
#pragma once

#include <cassert>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <ApiClient/common/ClassDefHelper.h>

#include "Utilities.h"


namespace ed::audio {
class MultipleNotificationClient : public virtual IMMNotificationClient, public virtual IAudioEndpointVolumeCallback {
public:
    DISALLOW_COPY_MOVE(MultipleNotificationClient);

private:
    LONG ref_ = 1;
    IMMDeviceEnumerator * enumerator_ = nullptr;

public:
    MultipleNotificationClient()
    {
        const auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&enumerator_));
        assert(SUCCEEDED(hr));

        RegisterEnumerator();
    }

    virtual ~MultipleNotificationClient()
    {
        UnregisterEnumerator();
    }

    // IUnknown methods
    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&ref_);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        const ULONG ulRef = InterlockedDecrement(&ref_);
        if (0 == ulRef)
        {
            this->UnregisterEnumerator();
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID refIId, VOID ** ppvInterface) override
    {
        if (IID_IUnknown == refIId)
        {
            AddRef();
            *ppvInterface = static_cast<void*>(this);
        }
        else if (__uuidof(IMMNotificationClient) == refIId)
        {
            AddRef();
            *ppvInterface = static_cast<IMMNotificationClient*>(this);
        }
        else if (__uuidof(IAudioEndpointVolumeCallback) == refIId)
        {
            AddRef();
            *ppvInterface = static_cast<IAudioEndpointVolumeCallback*>(this);
        }
        else
        {
            *ppvInterface = nullptr;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // IMMNotificationClient methods
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR deviceId, DWORD dwNewState) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR deviceId) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR deviceId) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR defaultDeviceId) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR deviceId, const PROPERTYKEY key) override
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override
    {
        return S_OK;
    }

protected:
    [[nodiscard]] IMMDeviceEnumerator* GetEnumeratorOrNull() const noexcept
    {
        return enumerator_; // nullptr if unavailable
    }

private:
    void RegisterEnumerator()
    {
        if (enumerator_ != nullptr)
        {
            // ReSharper disable once CppFunctionResultShouldBeUsed
            enumerator_->RegisterEndpointNotificationCallback(this);
        }
    }
    void UnregisterEnumerator()
    {
        if (enumerator_ != nullptr)
        {
            // ReSharper disable once CppFunctionResultShouldBeUsed
            enumerator_->UnregisterEndpointNotificationCallback(this);
            enumerator_->Release();
            enumerator_ = nullptr;
        }
    }

};

}
