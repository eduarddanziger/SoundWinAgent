using JetBrains.Annotations;

namespace SoundDefaultUI;

public class SoundDeviceInfo
{
    public required string PnpId { get; init; }
    [UsedImplicitly]
    public required string DeviceName { get; init; }
    public required bool IsRenderingAvailable { get; init; }
    public required bool IsCapturingAvailable { get; init; }
    public float RenderVolumeLevel { get; init; }
    public float CaptureVolumeLevel { get; init; }
    [UsedImplicitly]
    public string RenderVolumePercentText => IsRenderingAvailable ? $"{Math.Round(RenderVolumeLevel / 10.0)}%" : "";
    [UsedImplicitly]
    public string CaptureVolumePercentText => IsCapturingAvailable ? $"{Math.Round(CaptureVolumeLevel / 10.0)}%" : "";
}