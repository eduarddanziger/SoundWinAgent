#include "os-dependencies.h"

#include "SoundDeviceCollection.h"


std::unique_ptr<SoundDeviceCollectionInterface> SoundAgent::CreateDeviceCollection(bool bothHeadsetAndMicro)
{
    return std::make_unique<ed::audio::SoundDeviceCollection>(bothHeadsetAndMicro);
}
