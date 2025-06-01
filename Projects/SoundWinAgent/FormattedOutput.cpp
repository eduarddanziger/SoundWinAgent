#include "stdafx.h"

#include "FormattedOutput.h"

#include <SpdLogger.h>

#include <magic_enum/magic_enum.hpp>


void FormattedOutput::LogAndPrint(const std::string & mess)
{
    SPD_L->info(mess);
}

void FormattedOutput::PrintEvent(SoundDeviceEventType event, const std::string & devicePnpId)
{
	spdlog::info("Event caught: {}, device PnP id: {}.", magic_enum::enum_name(event), devicePnpId);
}

void FormattedOutput::PrintDeviceInfo(const SoundDeviceInterface * device)
{
	spdlog::info(R"({}, "{}", {}, Volume {} / {})", device->GetPnpId(), device->GetName(), magic_enum::enum_name(device->GetFlow()), device->GetCurrentRenderVolume(), device->GetCurrentCaptureVolume());
}

