namespace SoundDefaultUI;

using JetBrains.Annotations;
using NLog;

using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Threading;

public class MainViewModel
{
    private SoundDeviceService SoundDeviceService { get; }

    private static Dispatcher Dispatcher { get; } = Dispatcher.CurrentDispatcher;

    public string WindowTitle { get; }

    [UsedImplicitly]
    public static ThemeService ThemeService => ThemeService.Instance;

    public DefaultDeviceViewModel RenderDevice { get; } = new();
    public DefaultDeviceViewModel CaptureDevice { get; } = new();

    public MainViewModel(SoundDeviceService soundDeviceService)
    {
        var logger = LogManager.GetCurrentClassLogger();
        var args = Environment.GetCommandLineArgs();
        logger.Info(args.Length > 1
            ? "Command line parameter(s) detected. They are currently ignored."
            : "No command line parameters detected");

        WindowTitle = "System Default Sound";

        SoundDeviceService = soundDeviceService;
        SoundDeviceService.InitializeAndBind(OnDefaultRenderPresentOrAbsent, OnDefaultCapturePresentOrAbsent);

        var render = SoundDeviceService.GetRenderDevice();
        RenderDevice.Device = render.PnpId.Length != 0 ? render : null;

        var capture = SoundDeviceService.GetCaptureDevice();
        CaptureDevice.Device = capture.PnpId.Length != 0 ? capture : null;
    }

    private static void OnDefaultRenderPresentOrAbsent(bool presentOrAbsent)
    {
        Dispatcher.Invoke(() =>
        {
            var mainWindow = Window.GetWindow(App.Current.MainWindow) as MainWindow;
            if (mainWindow?.DataContext is MainViewModel vm)
            {
                vm.RenderDevice.Device = presentOrAbsent ? vm.SoundDeviceService.GetRenderDevice() : null;
            }
        });
    }

    private static void OnDefaultCapturePresentOrAbsent(bool presentOrAbsent)
    {
        Dispatcher.Invoke(() =>
        {
            var mainWindow = Window.GetWindow(App.Current.MainWindow) as MainWindow;
            if (mainWindow?.DataContext is MainViewModel vm)
            {
                vm.CaptureDevice.Device = presentOrAbsent ? vm.SoundDeviceService.GetCaptureDevice() : null;
            }
        });
    }
}