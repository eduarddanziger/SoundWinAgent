using System.Runtime.InteropServices;

namespace SoundDefaultUI;

public class SoundDeviceInfo
{
    public string PnpId { get; init; } = "";
    public string DeviceName { get; init; } = "";
    public bool IsRenderingAvailable { get; init; } = false;
    public bool IsCapturingAvailable { get; init; } = false;
    public float RenderVolumeLevel { get; init; }
    public float CaptureVolumeLevel { get; init; }
    public string RenderVolumePercentText => IsRenderingAvailable ? $"{Math.Round(RenderVolumeLevel / 10.0)}%" : "";
    public string CaptureVolumePercentText => IsCapturingAvailable ? $"{Math.Round(CaptureVolumeLevel / 10.0)}%" : "";
}