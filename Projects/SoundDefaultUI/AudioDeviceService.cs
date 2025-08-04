using static SoundDefaultUI.SoundAgentApi;

namespace SoundDefaultUI;

public class AudioDeviceService
{
    private readonly ulong _serviceHandle;

    public AudioDeviceService(TSaaDefaultRenderChangedDelegate discoverDelegate)
    {
#pragma warning disable CA1806
        SaaInitialize(out _serviceHandle, discoverDelegate);
#pragma warning restore CA1806
    }

    public AudioDeviceInfo GetAudioDevice()
    {
        if (_serviceHandle == 0)
        {
            throw new InvalidOperationException("Service handle is not initialized.");
        }
        // Get the default render device information
#pragma warning disable CA1806
        SaaGetDefaultRender(_serviceHandle, out var device);
#pragma warning restore CA1806
        return new AudioDeviceInfo
            { PnpId = device.PnpId, DeviceName = device.Name, VolumeLevel = device.RenderVolume };
        ;
    }

    public void Dispose()
    {
        if (_serviceHandle != 0)
        {
#pragma warning disable CA1806
            SaaUnInitialize(_serviceHandle);
#pragma warning restore CA1806
        }
    }
}