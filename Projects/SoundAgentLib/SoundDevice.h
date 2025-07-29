#pragma once

#include <string>

#include "public/SoundAgentInterface.h"

namespace ed::audio {
class SoundDevice final : public SoundDeviceInterface {
public:
    ~SoundDevice() override;

public:
    SoundDevice();
    SoundDevice(std::string pnpId, std::string name, SoundDeviceFlowType flow, uint16_t renderVolume, uint16_t captureVolume);
    SoundDevice(const SoundDevice & toCopy);
    SoundDevice(SoundDevice && toMove) noexcept;
    SoundDevice & operator=(const SoundDevice & toCopy);
    SoundDevice & operator=(SoundDevice && toMove) noexcept;

public:
    [[nodiscard]] std::string GetName() const override;
    [[nodiscard]] std::string GetPnpId() const override;
    [[nodiscard]] SoundDeviceFlowType GetFlow() const override;
    [[nodiscard]] uint16_t GetCurrentRenderVolume() const override; // 0 to 1000
    [[nodiscard]] uint16_t GetCurrentCaptureVolume() const override; // 0 to 1000
    void SetCurrentRenderVolume(uint16_t volume); // 0 to 1000
    void SetCurrentCaptureVolume(uint16_t volume); // 0 to 1000
    [[nodiscard]] bool IsCaptureCurrentlyDefault() const override;
    [[nodiscard]] bool IsRenderCurrentlyDefault() const override;

private:
    std::string pnpId_;
    std::string name_;
    SoundDeviceFlowType flow_;
    uint16_t renderVolume_; // 0 to 1000
    uint16_t captureVolume_; // 0 to 1000
};
}
