using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace SoundDefaultUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow
    {
        // DWM API for immersive dark mode on the window title bar (per guide)
        [DllImport("dwmapi.dll", PreserveSig = true)]
#pragma warning disable SYSLIB1054
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);
#pragma warning restore SYSLIB1054

        // Windows 10 1809 uses 19, Windows 10 1903+ and Windows 11 use 20
        private const int DwmUseImmersiveDarkModeBefore1903 = 19;
        private const int DwmUseImmersiveDarkMode = 20;

        public MainWindow(MainViewModel mainWindowViewModel)
        {
            InitializeComponent();
            DataContext = mainWindowViewModel;

            // Apply once the window source is initialized (HWND available)
            SourceInitialized += (_, _) => ApplyDarkTitleBar(ThemeService.Instance.IsDarkTheme);

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
            int hr = DwmSetWindowAttribute(hwnd, DwmUseImmersiveDarkMode, ref useDark, sizeof(int));
            if (hr != 0)
            {
                DwmSetWindowAttribute(hwnd, DwmUseImmersiveDarkModeBefore1903, ref useDark, sizeof(int));
            }
        }
    }
}