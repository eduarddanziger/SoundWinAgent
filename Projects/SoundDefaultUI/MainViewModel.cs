using NLog;

namespace SoundDefaultUI;

using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

public class MainViewModel : INotifyPropertyChanged
{
    private AudioDeviceService AudioDeviceService { get; }

    private static Dispatcher? MyDispatcher { get; set; }
    private AudioDeviceInfo? _device;

    public AudioDeviceInfo? Device
    {
        get => _device;
        set
        {
            // ReSharper disable once InvertIf
            if (_device != value)
            {
                _device = value;
                OnPropertyChanged(nameof(Device));
            }
        }
    }

    public string WindowTitle { get; }

    private readonly TSaaDefaultRenderChangedDelegate _onDefaultRenderPresentOrAbsent = OnDefaultRenderPresentOrAbsent;

    public MainViewModel()
    {
        MyDispatcher = Dispatcher.CurrentDispatcher;

        var logger = LogManager.GetCurrentClassLogger();
        var args = Environment.GetCommandLineArgs();
        logger.Info(args.Length > 1
            ? $"Command line parameter(s) detected. They are currently ignored."
            : "No command line parameters detected");

        WindowTitle = "Default Render Sound Device";

        AudioDeviceService = new AudioDeviceService(_onDefaultRenderPresentOrAbsent);

        var app = (App)Application.Current;
        app.AudioDeviceService = AudioDeviceService;

        var audioDeviceInfo = AudioDeviceService.GetAudioDevice();
        Device = audioDeviceInfo.PnpId.Length != 0 ? audioDeviceInfo : null;

        RefreshCommand = new RelayCommand(Refresh, () => Device != null);
        RefreshCommand.CanExecuteChanged += (sender, eventArgs) => OnPropertyChanged();
    }

    private static void OnDefaultRenderPresentOrAbsent(bool attach)
    {
        MyDispatcher?.Invoke(() =>
        {
            var mainWindow = Window.GetWindow(App.Current.MainWindow) as MainWindow;

            // ReSharper disable once InvertIf
            if (mainWindow?.DataContext is MainViewModel mainViewModel)
            {
                mainViewModel.Device = attach ? mainViewModel.AudioDeviceService.GetAudioDevice() : null;
                CommandManager.InvalidateRequerySuggested();
            }
        });
    }
    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string propertyName = "")
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    private ICommand RefreshCommand { get; }


    private void Refresh()
    {
        MyDispatcher?.Invoke(() =>
        {
            if (Device != null)
            {
                Device = AudioDeviceService.GetAudioDevice();
            }
        });
    }
}

public class RelayCommand : ICommand
{
    private readonly Action _execute;
    private readonly Func<bool>? _canExecute;

    // ReSharper disable once ConvertToPrimaryConstructor
    public RelayCommand(Action execute, Func<bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }
    public bool CanExecute(object? parameter) => _canExecute == null || _canExecute();

    public void Execute(object? parameter)
    {
        _execute();
    }

    public event EventHandler? CanExecuteChanged
    {
        add => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }
}
