namespace SoundDefaultUI;

using JetBrains.Annotations;
using NLog;

using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Threading;

public class MainViewModel : INotifyPropertyChanged
{
    private SoundDeviceService SoundDeviceService { get; }

    private static Dispatcher? MyDispatcher { get; set; }
    private SoundDeviceInfo? _device;

    // ReSharper disable once MemberCanBePrivate.Global
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

    [UsedImplicitly]
    public string WindowTitle { get; }

    [UsedImplicitly]
    public static ThemeService ThemeService => ThemeService.Instance;

    public MainViewModel(SoundDeviceService soundDeviceService)
    {
        MyDispatcher = Dispatcher.CurrentDispatcher;

        var logger = LogManager.GetCurrentClassLogger();
        var args = Environment.GetCommandLineArgs();
        logger.Info(args.Length > 1
            ? "Command line parameter(s) detected. They are currently ignored."
            : "No command line parameters detected");

        WindowTitle = "System Default Sound";

        SoundDeviceService = soundDeviceService;
        SoundDeviceService.InitializeAndBind(OnDefaultRenderPresentOrAbsent);

        var app = (App)Application.Current;
        app.SoundDeviceService = SoundDeviceService;

        var audioDeviceInfo = SoundDeviceService.GetSoundDevice();
        Device = audioDeviceInfo.PnpId.Length != 0 ? audioDeviceInfo : null;
    }

    private static void OnDefaultRenderPresentOrAbsent(bool presentOrAbsent)
    {
        MyDispatcher?.Invoke(() =>
        {
            // ReSharper disable once AccessToStaticMemberViaDerivedType
            // ReSharper disable once AssignNullToNotNullAttribute
            var mainWindow = Window.GetWindow(App.Current.MainWindow) as MainWindow;

            if (mainWindow?.DataContext is MainViewModel mainViewModel)
            {
                mainViewModel.Device = presentOrAbsent ? mainViewModel.SoundDeviceService.GetSoundDevice() : null;
            }
        });
    }
    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string propertyName = "")
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public bool IsDeviceNotNull => Device != null;
    public bool IsRenderingAvailable => Device is { IsRenderingAvailable: true };
    public bool IsCapturingAvailable => Device is { IsCapturingAvailable: true };

    public double Availability2GroupOpacity => IsDeviceNotNull ? 1.0 : 0.55;
    public double RenderingAvailability2IndicatorOpacity => IsRenderingAvailable ? 1.0 : 0.2;
    public double CapturingAvailability2IndicatorOpacity => IsCapturingAvailable ? 0.55 : 0.2;
}