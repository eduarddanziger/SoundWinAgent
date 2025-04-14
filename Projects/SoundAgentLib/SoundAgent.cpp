#include "stdafx.h"

#include "SoundDeviceCollection.h"


std::unique_ptr<SoundDeviceCollectionInterface> SoundAgent::CreateDeviceCollection(const std::wstring& nameFilter, bool bothHeadsetAndMicro)
{
    return std::make_unique<ed::audio::SoundDeviceCollection>(nameFilter, bothHeadsetAndMicro);
}
