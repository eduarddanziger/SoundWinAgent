namespace SoundDefaultUI;

public class AudioDeviceService
{
    private readonly ulong _serviceHandle;
    public AudioDeviceService(TSaaDefaultRenderChangedDelegate discoverDelegate)
    {
#pragma warning disable CA1806
        SoundAgentApi.SaaInitialize(out _serviceHandle, discoverDelegate);
#pragma warning restore CA1806
    }

    public AudioDeviceInfo GetAudioDevice()
    {
#pragma warning disable CA1806
        SoundAgentApi.SaaGetDefaultRender(_serviceHandle, out var device);
#pragma warning restore CA1806
        return new AudioDeviceInfo
            { PnpId = device.PnpId, DeviceName = device.Name, VolumeLevel = device.RenderVolume }; ;
    }


}