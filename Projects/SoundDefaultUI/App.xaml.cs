using System.Configuration;
using System.Data;
using System.Windows;

namespace SoundDefaultUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public SoundDeviceService? SoundDeviceService { private get; set; }

        public App()
        {
            InitializeComponent();
            
            // Initialize theme service and apply initial theme
            var themeService = ThemeService.Instance;
            ApplyTheme(themeService.IsDarkTheme);
            
            // Listen for theme changes
            themeService.PropertyChanged += (sender, e) =>
            {
                if (e.PropertyName == nameof(ThemeService.IsDarkTheme))
                {
                    Dispatcher.Invoke(() => ApplyTheme(themeService.IsDarkTheme));
                }
            };
        }

        private void ApplyTheme(bool isDarkTheme)
        {
            var themeKey = isDarkTheme ? "DarkTheme" : "LightTheme";
            
            if (Resources[themeKey] is ResourceDictionary themeResources)
            {
                // Clear existing merged dictionaries
                Resources.MergedDictionaries.Clear();
                
                // Apply the selected theme
                Resources.MergedDictionaries.Add(themeResources);
            }
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);

            SoundDeviceService?.Dispose();
        }
    }


}
