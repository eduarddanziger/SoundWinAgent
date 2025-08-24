using Microsoft.Win32;
using NLog;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Threading;
using Windows.UI;
using Windows.UI.ViewManagement;

namespace SoundDefaultUI;

public sealed class ThemeService : INotifyPropertyChanged
{
    private static ThemeService? _instance;
    private bool _isDarkTheme;
    private static Logger Logger { get; } = LogManager.GetCurrentClassLogger();
    private static Dispatcher Dispatcher { get; } = Dispatcher.CurrentDispatcher;

    public static ThemeService Instance => _instance ??= new ThemeService();

    public bool IsDarkTheme
    {
        get => _isDarkTheme;
        private set
        {
            // ReSharper disable once InvertIf
            if (_isDarkTheme != value)
            {
                _isDarkTheme = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsLightTheme));
            }
        }
    }

    public bool IsLightTheme => !_isDarkTheme;

    private ThemeService()
    {
        SystemEvents.UserPreferenceChanged += OnUserPreferenceChanged;
        UpdateTheme();
    }

    private void OnUserPreferenceChanged(object? sender, UserPreferenceChangedEventArgs e)
    {
        // ReSharper disable once InvertIf
        if (e.Category == UserPreferenceCategory.General)
        {
            // ensure update on UI thread
            if (Dispatcher.CheckAccess())
            {
                UpdateTheme();
            }
            else
            {
                Dispatcher.Invoke(UpdateTheme);
            }
        }
    }

    private static bool IsColorLight(Color clr)
        => ((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128);
    private void UpdateTheme()
    {
        try
        {

#pragma warning disable CA1416 // Windows 10.0.17763.0+ only
            var uiSettings = new UISettings();
            IsDarkTheme = !IsColorLight(uiSettings.GetColorValue(UIColorType.Background));
#pragma warning restore CA1416
        }
        catch (Exception e)
        {
            Logger.Warn($"Failed to use UISettings available in Windows > 10.0.17763.0 wheile detecting Dark mode. Set app to the Light mode: {e.Message}.");
            IsDarkTheme = false; // default to light theme
        }
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}