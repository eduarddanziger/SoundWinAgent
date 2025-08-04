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
        public AudioDeviceService? AudioDeviceService { private get; set; }

        public App()
        {
            InitializeComponent();
        }

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);

            AudioDeviceService?.Dispose();
        }
    }


}
