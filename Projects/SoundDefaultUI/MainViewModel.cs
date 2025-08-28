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

    public DefaultDeviceViewModel RenderDeviceViewModel { get; } = new(true);
    public DefaultDeviceViewModel CaptureDeviceViewModel { get; } = new(false);

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
        RenderDeviceViewModel.Device = render.PnpId.Length != 0 ? render : null;

        var capture = SoundDeviceService.GetCaptureDevice();
        CaptureDeviceViewModel.Device = capture.PnpId.Length != 0 ? capture : null;
    }

    private static void OnDefaultRenderPresentOrAbsent(bool presentOrAbsent)
    {
        Dispatcher.Invoke(() =>
        {
            // ReSharper disable once AccessToStaticMemberViaDerivedType
            var currentMainWindow = App.Current.MainWindow;
            // ReSharper disable once InvertIf
            if (currentMainWindow != null)
            {
                var mainWindow = Window.GetWindow(currentMainWindow) as MainWindow;
                if (mainWindow?.DataContext is MainViewModel vm)
                {
                    vm.RenderDeviceViewModel.Device = presentOrAbsent ? vm.SoundDeviceService.GetRenderDevice() : null;
                }
            }
        });
    }

    private static void OnDefaultCapturePresentOrAbsent(bool presentOrAbsent)
    {
        Dispatcher.Invoke(() =>
        {
            // ReSharper disable once AccessToStaticMemberViaDerivedType
            var currentMainWindow = App.Current.MainWindow;
            // ReSharper disable once InvertIf
            if (currentMainWindow != null)
            {
                var mainWindow = Window.GetWindow(currentMainWindow) as MainWindow;
                if (mainWindow?.DataContext is MainViewModel vm)
                {
                    vm.CaptureDeviceViewModel.Device = presentOrAbsent ? vm.SoundDeviceService.GetCaptureDevice() : null;
                }
            }
        });
    }
}