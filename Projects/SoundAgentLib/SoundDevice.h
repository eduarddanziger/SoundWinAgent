#pragma once

#include <string>

#include "public/SoundAgentInterface.h"

namespace ed::audio {
class SoundDevice final : public SoundDeviceInterface {
public:
    ~SoundDevice() override;

public:
    SoundDevice();
    SoundDevice(std::wstring pnpId, std::wstring name, SoundDeviceFlowType flow, uint16_t renderVolume, uint16_t captureVolume);
    SoundDevice(const SoundDevice & toCopy);
    SoundDevice(SoundDevice && toMove) noexcept;
    SoundDevice & operator=(const SoundDevice & toCopy);
    SoundDevice & operator=(SoundDevice && toMove) noexcept;

public:
    [[nodiscard]] std::wstring GetName() const override;
    [[nodiscard]] std::wstring GetPnpId() const override;
    [[nodiscard]] SoundDeviceFlowType GetFlow() const override;
    [[nodiscard]] uint16_t GetCurrentRenderVolume() const override; // 0 to 1000
    [[nodiscard]] uint16_t GetCurrentCaptureVolume() const override; // 0 to 1000
    void SetCurrentRenderVolume(uint16_t volume); // 0 to 1000
    void SetCurrentCaptureVolume(uint16_t volume); // 0 to 1000

private:
    std::wstring pnpId_;
    std::wstring name_;
    SoundDeviceFlowType flow_;
    uint16_t renderVolume_; // 0 to 1000
    uint16_t captureVolume_; // 0 to 1000
};
}
