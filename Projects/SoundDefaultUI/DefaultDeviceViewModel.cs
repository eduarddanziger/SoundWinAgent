using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SoundDefaultUI;

public class DefaultDeviceViewModel(bool isRenderOrCaptureDevice) : INotifyPropertyChanged
{
    private SoundDeviceInfo? _device;

    public SoundDeviceInfo? Device
    {
        get => _device;
        set
        {
            // ReSharper disable once InvertIf
            if (_device != value)
            {
                _device = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsDeviceNotNull));
                OnPropertyChanged(nameof(IsRenderingAvailable));
                OnPropertyChanged(nameof(IsCapturingAvailable));
                OnPropertyChanged(nameof(Availability2GroupOpacity));
                OnPropertyChanged(nameof(RenderingAvailability2IndicatorOpacity));
                OnPropertyChanged(nameof(CapturingAvailability2IndicatorOpacity));
            }
        }
    }

    public bool IsDeviceNotNull => Device != null;
    public bool IsRenderingAvailable => Device is { IsRenderingAvailable: true };
    public bool IsCapturingAvailable => Device is { IsCapturingAvailable: true };

    public double Availability2GroupOpacity => IsDeviceNotNull ? 1.0 : 0.55;
    public double RenderingAvailability2IndicatorOpacity => IsRenderingAvailable ? (isRenderOrCaptureDevice ? 1.0 : 0.55) : 0.2;
    public double CapturingAvailability2IndicatorOpacity => IsCapturingAvailable ? (isRenderOrCaptureDevice ? 0.55 : 1.0) : 0.2;

    public event PropertyChangedEventHandler? PropertyChanged;
    private void OnPropertyChanged([CallerMemberName] string propertyName = "")
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
}