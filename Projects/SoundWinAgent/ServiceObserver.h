#pragma once

#include <public/SoundAgentInterface.h>

class HttpRequestProcessor;

class ServiceObserver final : public SoundDeviceObserverInterface {
public:
    ServiceObserver(SoundDeviceCollectionInterface& collection,
        std::wstring apiBaseUrl,
        std::wstring universalToken,
        std::wstring codespaceName); // Added codespaceName parameter

    void PostDeviceToApi(SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr, const std::string & hintPrefix= "") const;
    void PutVolumeChangeToApi(const std::wstring& pnpId, bool renderOrCapture, uint16_t volume, const std::string & hintPrefix= "") const;

    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    void PostAndPrintCollection() const;

    void OnCollectionChanged(SoundDeviceEventType event, const std::wstring& devicePnpId) override;

    void OnTrace(const std::wstring& line) override;

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    static std::string GetHostName();

private:
    SoundDeviceCollectionInterface& collection_;
    std::wstring apiBaseUrl_;
    std::wstring universalToken_;
    std::wstring codespaceName_; // Newly added member for codespaceName
    std::shared_ptr<HttpRequestProcessor> requestProcessorSmartPtr_;
};
