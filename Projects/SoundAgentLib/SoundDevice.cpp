#include "stdafx.h"

#include "SoundDevice.h"

ed::audio::SoundDevice::~SoundDevice() = default;

ed::audio::SoundDevice::SoundDevice()
    : SoundDevice(L"", L"", SoundDeviceFlowType::None, 0, 0)
{
}

// ReSharper disable once CppParameterMayBeConst
ed::audio::SoundDevice::SoundDevice(std::wstring pnpId, std::wstring name, SoundDeviceFlowType flow, uint16_t renderVolume,
                          uint16_t captureVolume)
    : pnpId_(std::move(pnpId))
      , name_(std::move(name))
      , flow_(flow)
      , renderVolume_(renderVolume)
      , captureVolume_(captureVolume)
{
}

ed::audio::SoundDevice::SoundDevice(const SoundDevice & toCopy)
    : pnpId_(toCopy.pnpId_)
      , name_(toCopy.name_)
      , flow_(toCopy.flow_)
      , renderVolume_(toCopy.renderVolume_)
      , captureVolume_(toCopy.captureVolume_)
{
}

ed::audio::SoundDevice::SoundDevice(SoundDevice && toMove) noexcept
    : pnpId_(std::move(toMove.pnpId_))
      , name_(std::move(toMove.name_))
      , flow_(toMove.flow_)
      , renderVolume_(toMove.renderVolume_)
      , captureVolume_(toMove.captureVolume_)
{
}

ed::audio::SoundDevice & ed::audio::SoundDevice::operator=(const SoundDevice & toCopy)
{
    if (this != &toCopy)
    {
        pnpId_ = toCopy.pnpId_;
        name_ = toCopy.name_;
        flow_ = toCopy.flow_;
        renderVolume_ = toCopy.renderVolume_;
        captureVolume_ = toCopy.captureVolume_;
    }
    return *this;
}

ed::audio::SoundDevice & ed::audio::SoundDevice::operator=(SoundDevice && toMove) noexcept
{
    if (this != &toMove)
    {
        pnpId_ = std::move(toMove.pnpId_);
        name_ = std::move(toMove.name_);
        flow_ = toMove.flow_;
        renderVolume_ = toMove.renderVolume_;
        captureVolume_ = toMove.captureVolume_;
    }
    return *this;
}

std::wstring ed::audio::SoundDevice::GetName() const
{
    return name_;
}

std::wstring ed::audio::SoundDevice::GetPnpId() const
{
    return pnpId_;
}

SoundDeviceFlowType ed::audio::SoundDevice::GetFlow() const
{
    return flow_;
}

uint16_t ed::audio::SoundDevice::GetCurrentRenderVolume() const
{
    return renderVolume_;
}

uint16_t ed::audio::SoundDevice::GetCurrentCaptureVolume() const
{
    return captureVolume_;
}

void ed::audio::SoundDevice::SetCurrentRenderVolume(uint16_t volume)
{
    renderVolume_ = volume;
}

void ed::audio::SoundDevice::SetCurrentCaptureVolume(uint16_t volume)
{
    captureVolume_ = volume;
}

