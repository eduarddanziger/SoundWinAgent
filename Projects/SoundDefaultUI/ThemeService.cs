using Microsoft.Win32;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;

namespace SoundDefaultUI;

public sealed class ThemeService : INotifyPropertyChanged
{
    private static ThemeService? _instance;
    private bool _isDarkTheme;

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
        // Listen to system preference changes (theme toggles)
        SystemEvents.UserPreferenceChanged += OnUserPreferenceChanged;
        UpdateTheme();
    }

    private void OnUserPreferenceChanged(object? sender, UserPreferenceChangedEventArgs e)
    {
        // ReSharper disable once InvertIf
        if (e.Category == UserPreferenceCategory.General)
        {
            // ensure update on UI thread
            if (Application.Current?.Dispatcher.CheckAccess() == true)
            {
                UpdateTheme();
            }
            else
            {
                Application.Current?.Dispatcher.Invoke(UpdateTheme);
            }
        }
    }

    private void UpdateTheme()
    {
        try
        {
            using var key = Registry.CurrentUser.OpenSubKey(@"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");
            if (key?.GetValue("AppsUseLightTheme") is int value)
            {
                IsDarkTheme = value == 0;
            }
            else
            {
                IsDarkTheme = false;
            }
        }
        catch
        {
            IsDarkTheme = false;
        }
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}