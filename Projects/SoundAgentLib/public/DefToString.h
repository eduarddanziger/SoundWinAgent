#pragma once

#include <string>
#include <mmdeviceapi.h>

#include "SoundAgentInterface.h"


#ifndef COMMAND_CASE
#define COMMAND_CASE(cmd) case cmd: return L#cmd;  // NOLINT(cppcoreguidelines-macro-usage)
#endif

#ifndef COMMAND_CASE2
#define COMMAND_CASE2(enumClass, cmd) case enumClass::cmd: return L#cmd;  // NOLINT(cppcoreguidelines-macro-usage)
#endif


namespace ed {
template <class T>
std::wstring GetEDataFlowAsString(T v)
{
    switch (v)
    {
    COMMAND_CASE(eRender)
    COMMAND_CASE(eCapture)
    COMMAND_CASE(eAll)
    default:
        return L"Unknown data flow";
    }
}

inline std::wstring GetFlowAsString(SoundDeviceFlowType v)
{
    switch (v)
    {
    COMMAND_CASE2(SoundDeviceFlowType, Render)
    COMMAND_CASE2(SoundDeviceFlowType, Capture)
    COMMAND_CASE2(SoundDeviceFlowType, RenderAndCapture)
    case SoundDeviceFlowType::None:
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        return L"Unknown flow";
    }
}

inline std::wstring GetDeviceCollectionEventAsString(SoundDeviceEventType v)
{
    switch (v)
    {
    COMMAND_CASE2(SoundDeviceEventType, Discovered)
    COMMAND_CASE2(SoundDeviceEventType, Detached)
    COMMAND_CASE2(SoundDeviceEventType, VolumeRenderChanged)
    COMMAND_CASE2(SoundDeviceEventType, VolumeCaptureChanged)
    case SoundDeviceEventType::Confirmed:
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        return L"Unknown event";
    }
}
}
