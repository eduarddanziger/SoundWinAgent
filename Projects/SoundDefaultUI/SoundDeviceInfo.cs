namespace SoundDefaultUI;

public class SoundDeviceInfo
{
    public string PnpId { get; init; } = "";
    public string DeviceName { get; init; } = "";
    public float RenderVolumeLevel { get; init; }
    public float CaptureVolumeLevel { get; init; }
    public string RenderVolumePercentText => $"{Math.Round(RenderVolumeLevel / 10.0)}%";
    public string CaptureVolumePercentText => $"{Math.Round(CaptureVolumeLevel / 10.0)}%";
}