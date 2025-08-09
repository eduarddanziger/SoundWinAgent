﻿#pragma once

#include "public/SoundAgentInterface.h"

class HttpRequestProcessorInterface;
class HttpStandaloneProcessor;

class ServiceObserver final : public SoundDeviceObserverInterface {
public:
    ServiceObserver(SoundDeviceCollectionInterface& collection,
        HttpRequestProcessorInterface& requestProcessor
    );

    void PostDeviceToApi(SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr, const std::string & hintPrefix= "") const;
    void PutVolumeChangeToApi(const std::string & pnpId, bool renderOrCapture, uint16_t volume, const std::string & hintPrefix= "") const;

    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    void PostAndPrintCollection() const;

    void OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId) override;

private:
    static std::string GetHostName();
    static std::string GetOperationSystemName();

private:
    SoundDeviceCollectionInterface& collection_;
    HttpRequestProcessorInterface& requestProcessorInterface_;
};
