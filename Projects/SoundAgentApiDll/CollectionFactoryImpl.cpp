#include "stdafx.h"

#include "SoundDeviceCollection.h"


std::unique_ptr<SoundDeviceCollectionInterface> SoundAgent::CreateDeviceCollection()
{
    return std::make_unique<ed::audio::SoundDeviceCollection>();
}

