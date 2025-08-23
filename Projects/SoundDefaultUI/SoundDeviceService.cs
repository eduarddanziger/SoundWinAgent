using static SoundDefaultUI.SoundAgentApi;

namespace SoundDefaultUI;

public class SoundDeviceService
{
    private ulong _serviceHandle;

    public void InitializeAndBind(SaaDefaultChangedDelegate renderNotification, SaaDefaultChangedDelegate captureNotification)
    {
#pragma warning disable CA1806
        SaaInitialize(out _serviceHandle, renderNotification, captureNotification);
#pragma warning restore CA1806
    }

    // Common helpers
    private static SoundDeviceInfo EmptyDeviceInfo() => new SoundDeviceInfo
    {
        PnpId = "",
        DeviceName = "",
        IsRenderingAvailable = false,
        IsCapturingAvailable = false,
        RenderVolumeLevel = 0,
        CaptureVolumeLevel = 0
    };

    private static SoundDeviceInfo SaaDescription2SoundDeviceInfo(in SaaDescription device) => new SoundDeviceInfo
    {
        PnpId = device.PnpId,
        DeviceName = device.Name,
        IsRenderingAvailable = device.IsRender,
        IsCapturingAvailable = device.IsCapture,
        RenderVolumeLevel = device.RenderVolume,
        CaptureVolumeLevel = device.CaptureVolume
    };

    private SoundDeviceInfo GetDevice(Func<ulong, SaaDescription> fetch)
    {
        if (_serviceHandle == 0)
        {
            return EmptyDeviceInfo();
        }
        var dev = fetch(_serviceHandle);
        return SaaDescription2SoundDeviceInfo(dev);
    }

    public SoundDeviceInfo GetRenderDevice()
    {
        return GetDevice(handle =>
        {
#pragma warning disable CA1806
            SaaGetDefaultRender(handle, out var device);
#pragma warning restore CA1806
            return device;
        });
    }

    public SoundDeviceInfo GetCaptureDevice()
    {
        return GetDevice(handle =>
        {
#pragma warning disable CA1806
            SaaGetDefaultCapture(handle, out var device);
#pragma warning restore CA1806
            return device;
        });
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