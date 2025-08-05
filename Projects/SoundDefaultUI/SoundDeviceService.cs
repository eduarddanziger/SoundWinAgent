using static SoundDefaultUI.SoundAgentApi;

namespace SoundDefaultUI;

public class SoundDeviceService
{
    private readonly ulong _serviceHandle;

    public SoundDeviceService(TSaaDefaultRenderChangedDelegate discoverDelegate)
    {
#pragma warning disable CA1806
        SaaInitialize(out _serviceHandle, discoverDelegate);
#pragma warning restore CA1806
    }

    public SoundDeviceInfo GetSoundDevice()
    {
        if (_serviceHandle == 0)
        {
            throw new InvalidOperationException("Service handle is not initialized.");
        }
        // Get the default render device information
#pragma warning disable CA1806
        SaaGetDefaultRender(_serviceHandle, out var device);
#pragma warning restore CA1806
        return new SoundDeviceInfo
        {
            PnpId = device.PnpId, DeviceName = device.Name,
            IsRenderingAvailable = device.IsRender, IsCapturingAvailable = device.IsCapture,
            RenderVolumeLevel = device.RenderVolume, CaptureVolumeLevel = device.CaptureVolume
        };
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