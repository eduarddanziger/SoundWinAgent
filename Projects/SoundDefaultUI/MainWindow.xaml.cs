using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace SoundDefaultUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // DWM API for immersive dark mode on the window title bar (per guide)
        [DllImport("dwmapi.dll", PreserveSig = true)]
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);

        // Windows 10 1809 uses 19, Windows 10 1903+ and Windows 11 use 20
        private const int DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_1903 = 19;
        private const int DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

        public MainWindow(MainViewModel mainWindowViewModel)
        {
            InitializeComponent();
            DataContext = mainWindowViewModel;

            // Apply once the window source is initialized (HWND available)
            SourceInitialized += (_, __) => ApplyDarkTitleBar(ThemeService.Instance.IsDarkTheme);

            // React to theme changes
            ThemeService.Instance.PropertyChanged += OnThemeServicePropertyChanged;
        }

        private void OnThemeServicePropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(ThemeService.IsDarkTheme))
            {
                ApplyDarkTitleBar(ThemeService.Instance.IsDarkTheme);
            }
        }

        private void ApplyDarkTitleBar(bool useDarkMode)
        {
            var hwnd = new WindowInteropHelper(this).Handle;
            if (hwnd == IntPtr.Zero) return;

            int useDark = useDarkMode ? 1 : 0;

            // Try the modern attribute first, fall back for 1809
            int hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, ref useDark, sizeof(int));
            if (hr != 0)
            {
                DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_1903, ref useDark, sizeof(int));
            }
        }
    }
}