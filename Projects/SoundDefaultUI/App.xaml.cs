using System.Windows;

namespace SoundDefaultUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App
    {
        public App()
        {
            InitializeComponent();
            
            // Initialize theme service and apply initial theme
            var themeService = ThemeService.Instance;
            ApplyTheme(themeService.IsDarkTheme);
            
            // Listen for theme changes
            themeService.PropertyChanged += (_, e) =>
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
            
            // ReSharper disable once InvertIf
            if (Resources[themeKey] is ResourceDictionary themeResources)
            {
                // Clear existing merged dictionaries
                Resources.MergedDictionaries.Clear();
                
                // Apply the selected theme
                Resources.MergedDictionaries.Add(themeResources);
            }
        }
    }


}
